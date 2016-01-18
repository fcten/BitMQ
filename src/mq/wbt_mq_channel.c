/* 
 * File:   wbt_mq_channel.c
 * Author: fcten
 *
 * Created on 2016年1月16日, 上午12:32
 */

#include "wbt_mq_channel.h"

// 存储所有可用频道
// 一个频道下有一个或多个消费者，如果所有消费者离开频道，该频道将会撤销
wbt_rbtree_t wbt_mq_channels;

wbt_channel_t * wbt_mq_channel_create(wbt_mq_id channel_id) {
    wbt_channel_t * channel = wbt_new(wbt_channel_t);
    
    if( channel ) {
        channel->channel_id = channel_id;
        channel->create = wbt_cur_mtime;
        channel->subscriber_list = wbt_new(wbt_subscriber_list_t);
        
        if( channel->subscriber_list ) {
            wbt_list_init(&channel->subscriber_list->list);
        } else {
            wbt_mq_channel_destory(channel);
            return NULL;
        }

        wbt_str_t channel_key;
        wbt_variable_to_str(channel->channel_id, channel_key);
        wbt_rbtree_node_t * channel_node = wbt_rbtree_insert(&wbt_mq_channels, &channel_key);
        if( channel_node == NULL ) {
            wbt_mq_channel_destory(channel);
            return NULL;
        }
    }
    
    return channel;
}

wbt_channel_t * wbt_mq_channel_get(wbt_mq_id channel_id) {
    wbt_channel_t * channel;
    wbt_str_t channel_key;
    wbt_variable_to_str(channel_id, channel_key);
    wbt_rbtree_node_t * channel_node = wbt_rbtree_get(&wbt_mq_channels, &channel_key);

    if( channel_node == NULL ) {
        channel = wbt_mq_channel_create(channel_id);
    } else {
        channel = channel_node->value.ptr;
    }
    
    return channel;
}

void wbt_mq_channel_destory(wbt_channel_t *channel) {
    if( channel == NULL ) {
        return;
    }

    if( channel->subscriber_list ) {
        wbt_subscriber_list_t * pos;
        while(!wbt_list_empty(&channel->subscriber_list->list)) {
            pos = wbt_list_first_entry(&channel->subscriber_list->list, wbt_subscriber_list_t, list);
            wbt_list_del(&pos->list);
            wbt_delete(pos);
        }
        wbt_delete(channel->subscriber_list);
    }

    wbt_delete(channel);
}