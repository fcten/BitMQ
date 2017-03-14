/* 
 * File:   wbt_mq_msg.c
 * Author: fcten
 *
 * Created on 2016年1月18日, 下午4:08
 */

#include "wbt_mq_msg.h"
#include "wbt_mq_channel.h"
#include "wbt_mq_subscriber.h"

extern wbt_atomic_t wbt_wating_to_exit;

// 存储所有已接收到的消息
static wbt_rb_t wbt_mq_messages;

wbt_status wbt_mq_msg_init() {
    wbt_rb_init(&wbt_mq_messages, WBT_RB_KEY_LONGLONG);

    return WBT_OK;
}

static wbt_mq_id wbt_msg_create_count = 0;
static wbt_mq_id wbt_msg_effect_count = 0;
static wbt_mq_id wbt_msg_delete_count = 0;

void wbt_mq_msg_update_create_count(wbt_mq_id msg_id) {
    if( wbt_msg_create_count < msg_id ) {
        wbt_msg_create_count = msg_id;
    }
}

wbt_msg_t * wbt_mq_msg_create(wbt_mq_id msg_id) {
    wbt_msg_t * msg = wbt_calloc(sizeof(wbt_msg_t));
    
    if( msg ) {
        if( msg_id ) {
            msg->msg_id = msg_id;
            if( wbt_msg_create_count < msg_id ) {
                wbt_msg_create_count = msg_id;
            }
        } else {
            msg->msg_id = ++wbt_msg_create_count;
        }
        msg->seq_id = 0;
        msg->create = wbt_cur_mtime;
        
        wbt_str_t msg_key;
        wbt_variable_to_str(msg->msg_id, msg_key);
        wbt_rb_node_t * msg_node = wbt_rb_insert(&wbt_mq_messages, &msg_key);
        if( msg_node == NULL ) {
            wbt_log_debug("msg_id conflict: %llu\n", msg_id);
            wbt_free(msg);
            return NULL;
        }
        msg_node->value.str = (char *)msg;
        
        wbt_log_debug("msg %lld create\n", msg->msg_id);
    }
    
    return msg;
}

wbt_msg_t * wbt_mq_msg_get(wbt_mq_id msg_id) {
    wbt_str_t msg_key;
    wbt_variable_to_str(msg_id, msg_key);
    wbt_rb_node_t * msg_node = wbt_rb_get(&wbt_mq_messages, &msg_key);
    if( msg_node == NULL ) {
        // TODO 调用持久化接口，从磁盘中读取并缓存到内存中
        return NULL;
    }
    return (wbt_msg_t *)msg_node->value.str;
}

void wbt_mq_msg_destory(wbt_msg_t *msg) {
    if( msg == NULL ) {
        return;
    }
    
    wbt_log_debug("msg %lld deleted\n", msg->msg_id);
    
    // 从消息所对应的频道中删除该消息
    // TODO 这里没有必要再做一次 wbt_mq_channel_get 查询
    if( msg->type != MSG_ACK ) {
        wbt_channel_t * channel = wbt_mq_channel_get(msg->consumer_id);
        if( channel ) {
            wbt_mq_channel_del_msg(channel, msg);
        }
    }
    
    wbt_timer_del(&msg->timer);
    
    wbt_free(msg->data);

    wbt_str_t msg_key;
    wbt_variable_to_str(msg->msg_id, msg_key);
    wbt_rb_node_t * msg_node = wbt_rb_get( &wbt_mq_messages, &msg_key );
    if( msg_node ) {
        wbt_rb_delete( &wbt_mq_messages, msg_node );
    }
    
    wbt_msg_delete_count ++;
}

wbt_status wbt_mq_msg_destory_expired(wbt_timer_t *timer) {
    // Bugfix: BitMQ 在退出时会执行并删除所有尚未超时的定时事件，本定时事件不能在程
    // 序退出时执行。
    if( wbt_wating_to_exit ) {
        return WBT_OK;
    }

    wbt_msg_t *msg = wbt_timer_entry(timer, wbt_msg_t, timer);

    wbt_mq_msg_destory(msg);
    
    return WBT_OK;
}

wbt_status wbt_mq_msg_delivery_delayed(wbt_timer_t *timer) {
    // Bugfix: BitMQ 在退出时会执行并删除所有尚未超时的定时事件，本定时事件不能在程
    // 序退出时执行。
    if( wbt_wating_to_exit ) {
        return WBT_OK;
    }

    wbt_msg_t *msg = wbt_timer_entry(timer, wbt_msg_t, timer);
    
    wbt_timer_del(&msg->timer);
    
    if( wbt_mq_msg_delivery(msg) == WBT_OK ) {
        return WBT_OK;
    } else {
        // 投递失败，该消息将会丢失。内存不足时可能出现该情况
        wbt_mq_msg_destory(msg);
        return WBT_ERROR;
    }
}

/**
 * 投递一条消息
 * @param msg 消息体指针
 * @return wbt_status
 */
wbt_status wbt_mq_msg_delivery(wbt_msg_t *msg) {
    if( msg->expire <= wbt_cur_mtime ) {
        // 投递的消息已经过期
        
        return WBT_ERROR;
    } else if( msg->effect > wbt_cur_mtime ) {
        // 未生效消息
        msg->timer.on_timeout = wbt_mq_msg_delivery_delayed;
        msg->timer.timeout    = msg->effect;
        if( wbt_timer_add(&msg->timer) != WBT_OK ) {
            return WBT_ERROR;
        }
        return WBT_OK;
    } else {
        // 已生效消息
        if( !msg->timer.heap_idx ) {
            msg->timer.on_timeout = wbt_mq_msg_destory_expired;
            msg->timer.timeout    = msg->expire;
            if( wbt_timer_add(&msg->timer) != WBT_OK ) {
                return WBT_ERROR;
            }
        }
        
        msg->seq_id = ++wbt_msg_effect_count;
    }

    wbt_channel_t * channel = wbt_mq_channel_get(msg->consumer_id);
    if( channel == NULL ) {
        return WBT_ERROR;
    }

    // 已生效消息，保存该消息
    if( wbt_mq_channel_add_msg(channel, msg) != WBT_OK ) {
        return WBT_ERROR;
    }

    if( msg->type == MSG_BROADCAST ) {
        // 广播模式，通知该频道下所有的订阅者获取该消息
        // TODO 限制循环次数，订阅数量过多的频道不进行实时投递（数值可以由配置文件设定）
        wbt_subscriber_list_t * subscriber_node;
        wbt_subscriber_t * subscriber;
        wbt_list_for_each_entry( subscriber_node, wbt_subscriber_list_t, &channel->subscriber_list.head, head ) {
            subscriber = subscriber_node->subscriber;

            if( subscriber->is_ready(subscriber->ev) == WBT_OK ) {
                if( wbt_mq_subscriber_send_msg(subscriber) != WBT_OK ) {
                    // TODO 记录投递失败
                    wbt_log_debug("msg %lld send to %d failed\n", msg->msg_id, subscriber->ev->fd);
                }
            }
        }
    } else if( msg->type == MSG_LOAD_BALANCE ) {
        if( wbt_list_empty( &channel->subscriber_list.head ) ) {
            // 频道没有任何订阅者
            return WBT_OK;
        }

        // 负载均衡模式，通知订阅队列中的第一个订阅者获取该消息
        wbt_subscriber_list_t * subscriber_node;
        wbt_subscriber_t * subscriber;
        subscriber_node = wbt_list_first_entry(&channel->subscriber_list.head, wbt_subscriber_list_t, head);
        subscriber = subscriber_node->subscriber;

        // TODO 这里忽略了投递失败的情况，导致消息不能严格地平均分发
        // 后续应当改进负载均衡算法
        if( subscriber->is_ready(subscriber->ev) == WBT_OK ) {
            if( wbt_mq_subscriber_send_msg(subscriber) != WBT_OK ) {
                // 直接投递失败，忽略该错误
            }
        }

        // 无论是否成功，都应当将该订阅者移动到链表末尾
        wbt_list_move_tail(&subscriber_node->head, &channel->subscriber_list.head);
    } else {
        return WBT_ERROR;
    }
    
    return WBT_OK;
}

wbt_msg_list_t * wbt_mq_msg_create_node(wbt_mq_id msg_id) {
    wbt_msg_list_t * msg_node = wbt_calloc(sizeof(wbt_msg_list_t));
    if( msg_node == NULL ) {
        return NULL;
    }
    msg_node->msg_id = msg_id;

    return msg_node;
}

void wbt_mq_msg_destory_node(wbt_msg_list_t *node) {
    wbt_free(node);
}

long long int wbt_mq_msg_status_total() {
    return (long long int)wbt_msg_create_count;
}

long long int wbt_mq_msg_status_active() {
    return (long long int)wbt_mq_messages.size;
}

long long int wbt_mq_msg_status_delayed() {
    return 0;
}

long long int wbt_mq_msg_status_waiting_ack() {
    return 0;
}

json_object_t* wbt_mq_msg_print(wbt_msg_t *msg) {
    json_object_t * obj = json_create_object();
    
    json_append(obj, wbt_mq_str_msg_id.str,       wbt_mq_str_msg_id.len,       JSON_LONGLONG, &msg->msg_id,      0);
    if( msg->producer_id ) {
        json_append(obj, wbt_mq_str_producer_id.str, wbt_mq_str_producer_id.len, JSON_LONGLONG, &msg->producer_id, 0);
    }
    json_append(obj, wbt_mq_str_consumer_id.str,  wbt_mq_str_consumer_id.len, JSON_LONGLONG, &msg->consumer_id, 0 );
    json_append(obj, wbt_mq_str_create.str,       wbt_mq_str_create.len,      JSON_LONGLONG, &msg->create,      0);

    json_task_t t;
    t.str = msg->data;
    t.len = msg->data_len;
    t.callback = NULL;

    if( json_parser(&t) != 0 ) {
        json_delete_object(t.root);

        json_append(obj, wbt_mq_str_data.str, wbt_mq_str_data.len, JSON_STRING,         msg->data, msg->data_len);
    } else {
        json_append(obj, wbt_mq_str_data.str, wbt_mq_str_data.len, t.root->object_type, t.root   , 0);
    }
    
    return obj;
}

wbt_rb_t* wbt_mq_msg_get_all() {
    return &wbt_mq_messages;
}
