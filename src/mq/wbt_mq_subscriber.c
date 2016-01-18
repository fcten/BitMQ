/* 
 * File:   wbt_mq_subscriber.c
 * Author: fcten
 *
 * Created on 2016年1月16日, 上午12:21
 */

#include "wbt_mq_subscriber.h"

// 存储所有的订阅者
wbt_rbtree_t wbt_mq_subscribers;

wbt_subscriber_t * wbt_mq_subscriber_create(wbt_mq_id subscriber_id) {
    wbt_subscriber_t * subscriber = wbt_new(wbt_subscriber_t);
    
    if( subscriber ) {
        subscriber->subscriber_id = subscriber_id;
        subscriber->create = wbt_cur_mtime;
        subscriber->msg_list = wbt_new(wbt_msg_list_t);
        subscriber->channel_list = wbt_new(wbt_channel_list_t);
        
        if( subscriber->msg_list && subscriber->channel_list ) {
            wbt_list_init(&subscriber->msg_list->list);
            wbt_list_init(&subscriber->channel_list->list);
        } else {
            wbt_mq_subscriber_destory(subscriber);
            return NULL;
        }

        wbt_str_t subscriber_key;
        wbt_variable_to_str(subscriber->subscriber_id, subscriber_key);
        wbt_rbtree_node_t * subscriber_node = wbt_rbtree_insert(&wbt_mq_subscribers, &subscriber_key);
        if( subscriber_node == NULL ) {
            wbt_mq_subscriber_destory(subscriber);
            return NULL;
        }
    }
    
    return subscriber;
}

wbt_subscriber_t * wbt_mq_subscriber_get(wbt_mq_id subscriber_id) {
    wbt_subscriber_t * subscriber;
    wbt_str_t subscriber_key;
    wbt_variable_to_str(subscriber_id, subscriber_key);
    wbt_rbtree_node_t * subscriber_node = wbt_rbtree_get(&wbt_mq_subscribers, &subscriber_key);

    if( subscriber_node == NULL ) {
        subscriber = wbt_mq_subscriber_create(subscriber_id);
    } else {
        subscriber = subscriber_node->value.ptr;
    }
    
    return subscriber;
}

void wbt_mq_subscriber_destory(wbt_subscriber_t *subscriber) {
    if( subscriber == NULL ) {
        return;
    }

    if( subscriber->channel_list ) {
        wbt_channel_list_t * pos;
        while(!wbt_list_empty(&subscriber->channel_list->list)) {
            pos = wbt_list_first_entry(&subscriber->channel_list->list, wbt_channel_list_t, list);
            wbt_list_del(&pos->list);
            wbt_delete(pos);
        }
        wbt_delete(subscriber->channel_list);
    }

    if( subscriber->msg_list ) {
        wbt_msg_list_t * pos;
        while(!wbt_list_empty(&subscriber->msg_list->list)) {
            pos = wbt_list_first_entry(&subscriber->msg_list->list, wbt_msg_list_t, list);
            wbt_list_del(&pos->list);
            wbt_delete(pos);
        }
        wbt_delete(subscriber->msg_list);
    }

    wbt_delete(subscriber);
}