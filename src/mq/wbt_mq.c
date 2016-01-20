/* 
 * File:   wbt_mq.c
 * Author: fcten
 *
 * Created on 2016年1月14日, 上午10:59
 */

#include "wbt_mq.h"
#include "../common/wbt_module.h"
#include "wbt_mq_channel.h"
#include "wbt_mq_msg.h"
#include "wbt_mq_subscriber.h"

wbt_module_t wbt_module_mq = {
    wbt_string("mq"),
    wbt_mq_init, // init
    NULL, // exit
    NULL, // on_conn
    wbt_mq_on_recv, // on_recv
    NULL, // on_send
    wbt_mq_on_close  // on_close
};

wbt_status wbt_mq_init() {
    wbt_mq_channel_init();
    wbt_mq_subscriber_init();
    wbt_mq_msg_init();

    return WBT_OK;
}

wbt_status wbt_mq_on_recv(wbt_event_t *ev) {
    // 分发请求
    wbt_http_t * http = ev->data.ptr;

    // 只过滤 404 响应
    if( http->status != STATUS_404 ) {
        return WBT_OK;
    }

    wbt_str_t login  = wbt_string("/mq/login/");
    wbt_str_t pull   = wbt_string("/mq/pull/");
    wbt_str_t push   = wbt_string("/mq/push/");
    wbt_str_t status = wbt_string("/mq/status/");
    
    wbt_str_t http_uri;
    wbt_offset_to_str(http->uri, http_uri, ev->buff.ptr);
    
    if( wbt_strncmp( &http_uri, &login, login.len ) == 0 ) {
        return wbt_mq_login(ev);
    } else if( wbt_strcmp( &http_uri, &pull ) == 0 ) {
        return wbt_mq_pull(ev);
    } else if( wbt_strcmp( &http_uri, &push ) == 0 ) {
        return wbt_mq_push(ev);
    } else if( wbt_strncmp( &http_uri, &status, status.len ) == 0 ) {
        return wbt_mq_status(ev);
    }

    return WBT_OK;
}

wbt_status wbt_mq_on_close(wbt_event_t *ev) {
    wbt_subscriber_t *subscriber = ev->ctx;
    if( subscriber == NULL ) {
        return WBT_OK;
    }

    // 遍历所有订阅的频道
    wbt_channel_list_t *channel_node;
    wbt_list_for_each_entry(channel_node, &subscriber->channel_list->head, head) {
        wbt_channel_t *channel = channel_node->channel;
        // 从该频道的 subscriber_list 中移除该订阅者
        // TODO 这需要遍历链表，对于大量订阅者的频道来说不可接受。
        // 但是负载均衡模式又需要用到链表，这个地方还需要斟酌
        wbt_subscriber_list_t *subscriber_node;
        wbt_list_for_each_entry(subscriber_node, &channel->subscriber_list->head, head) {
            if( subscriber_node->subscriber == subscriber ) {
                wbt_list_del(&subscriber_node->head);
                break;
            }
        }
    }

    // 遍历所有尚未处理完毕的负载均衡消息，将消息重新进行投递
    // 广播消息可以直接忽略，无需处理
    wbt_heap_node_t *msg_node;
    int i;
    wbt_heap_for_each(i, msg_node, &subscriber->delivered_heap) {
        wbt_msg_t * msg = msg_node->ptr;
        wbt_mq_msg_delivery(msg);
    }

    // 删除该订阅者
    wbt_mq_subscriber_destory(subscriber);
    
    return WBT_OK;
}

wbt_status wbt_mq_login(wbt_event_t *ev) {
    // 解析请求
    wbt_http_t * http = ev->data.ptr;

    // 必须是 POST 请求
    if( http->method != METHOD_POST ) {
        http->status = STATUS_405;
        return WBT_OK;
    }
    
    // 处理 body
    if( http->body.len != 16 ) {
        http->status = STATUS_413;
        return WBT_OK;
    }
    
    // 检查是否已经登录过
    if( ev->ctx != NULL ) {
        wbt_mq_on_close(ev);
    }

    // 创建一个新的订阅者并初始化
    wbt_subscriber_t * subscriber = wbt_mq_subscriber_create();
    if( subscriber == NULL ) {
        // 返回登录失败
        return WBT_OK;
    }
    
    ev->ctx = subscriber;
    
    // 在所有想要订阅的频道的 subscriber_list 中添加该订阅者
    wbt_str_t channel_ids;
    wbt_offset_to_str(http->body, channel_ids, ev->buff.ptr);
    wbt_mq_id channel_id = wbt_str_to_ull(&channel_ids, 16);

    // 遍历想要订阅的所有频道
        wbt_channel_t * channel = wbt_mq_channel_get(channel_id);
        if( channel == NULL ) {
            http->status = STATUS_503;
            return WBT_OK;
        }
        // 在该频道的 subscriber_list 中添加该订阅者
        wbt_subscriber_list_t *subscriber_node = wbt_new(wbt_subscriber_list_t);
        if( subscriber_node == NULL ) {
            http->status = STATUS_503;
            return WBT_OK;
        }
        subscriber_node->subscriber = subscriber;
        wbt_list_add(&subscriber_node->head, &channel->subscriber_list->head);
        // 遍历该频道的 effective_heap
        wbt_heap_node_t *msg_node;
        int i;
        wbt_heap_for_each(i, msg_node, &channel->effective_heap) {
            // 复制该消息到订阅者的 msg_list 中
            wbt_msg_list_t *tmp_node = wbt_new(wbt_msg_list_t);
            if( tmp_node == NULL ) {
                // 内存不足，操作失败
                continue;
            }
            tmp_node->msg = msg_node->ptr;
            wbt_list_add(&tmp_node->head, &subscriber->msg_list->head);
            // TODO 如果是负载均衡消息，则从 effective_heap 中移除该消息
        }

    http->status = STATUS_200;
        
    return WBT_OK;
}

wbt_status wbt_mq_push(wbt_event_t *ev) {
    // 解析请求
    wbt_http_t * http = ev->data.ptr;

    // 必须是 POST 请求
    if( http->method != METHOD_POST ) {
        http->status = STATUS_405;
        return WBT_OK;
    }
    
    // 处理 body
    // 前 16 位作为 ID，后面的作为 push 的数据
    if( http->body.len <= 16 ) {
        http->status = STATUS_413;
        return WBT_OK;
    }
    
    wbt_str_t channel_ids, data;
    wbt_offset_to_str(http->body, channel_ids, ev->buff.ptr);
    channel_ids.len = 16;
    data.str = channel_ids.str + 16;
    data.len = http->body.len - 16;
    
    wbt_mq_id channel_id = wbt_str_to_ull(&channel_ids, 16);

    // 创建消息并初始化
    wbt_msg_t * msg = wbt_mq_msg_create();
    if( msg == NULL ) {
        // 返回投递失败
        return WBT_OK;
    }
    msg->consumer_id = channel_id;
    msg->effect = msg->create;
    msg->expire = msg->create + 3600 * 1000;
    msg->delivery_mode = MSG_BROADCAST;
    
    // 投递消息
    if( wbt_mq_msg_delivery( msg ) != WBT_OK ) {
        http->status = STATUS_403;
        return WBT_OK;
    }
    
    // TODO 返回消息编号
    http->status = STATUS_200;

    return WBT_OK;
}

wbt_status wbt_mq_pull(wbt_event_t *ev) {
    wbt_subscriber_t *subscriber = ev->ctx;
    
    // 从订阅者的 msg_list 中取出第一条消息
    // 将该消息移动到 delivered_heap 中，并返回该消息
    
    // 如果没有可发送的消息，则挂起请求，设定新的超时时间和超时处理函数
    
    return WBT_OK;
}

wbt_status wbt_mq_pull_timeout(wbt_event_t *ev) {
    // 固定返回一个空的响应，通知客户端重新发起 pull 请求
    
    return WBT_OK;
}

wbt_status wbt_mq_ack(wbt_event_t *ev) {
    // 解析请求
    wbt_mq_id msg_id;
    wbt_mq_id subscriber_id;
    
    // 该条消息消费成功
    
    // 从该订阅者的 delivered_heap 中移除消息
    
    return WBT_OK;
}

wbt_status wbt_mq_status(wbt_event_t *ev) {
    wbt_http_t * http = ev->data.ptr;
    
    // 必须是 GET 请求
    if( http->method != METHOD_GET ) {
        http->status = STATUS_405;
        return WBT_OK;
    }

    wbt_str_t http_uri;
    wbt_offset_to_str(http->uri, http_uri, ev->buff.ptr);
    
    wbt_str_t channel_ids;
    channel_ids.str = http_uri.str + 11;
    channel_ids.len = http_uri.len - 11;
    if( channel_ids.len != 16 ) {
        http->status = STATUS_413;
        return WBT_OK;
    }
    wbt_mq_id channel_id = wbt_str_to_ull(&channel_ids, 16);
    
    wbt_channel_t * channel = wbt_mq_channel_get(channel_id);
    if( channel == NULL ) {
        http->status = STATUS_404;
        return WBT_OK;
    }
    
    wbt_mem_t tmp;
    wbt_malloc(&tmp, 10240);

    wbt_str_t resp;
    resp.len = 0;
    resp.str = tmp.ptr;

    wbt_str_t online = wbt_string("test");
    wbt_strcat(&resp, &online, tmp.len);
    wbt_realloc(&tmp, resp.len);
    
    http->status = STATUS_200;
    http->file.ptr = tmp.ptr;
    http->file.size = tmp.len;

    return WBT_OK;
}