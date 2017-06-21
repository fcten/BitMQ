/* 
 * File:   wbt_mq_subscriber.h
 * Author: fcten
 *
 * Created on 2016年1月16日, 上午12:21
 */

#ifndef WBT_MQ_SUBSCRIBER_H
#define	WBT_MQ_SUBSCRIBER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "wbt_mq.h"
#include "wbt_mq_channel.h"

wbt_status wbt_mq_subscriber_init();
wbt_subscriber_t * wbt_mq_subscriber_create();
wbt_subscriber_t * wbt_mq_subscriber_get(wbt_mq_id subscriber_id);
void wbt_mq_subscriber_destory(wbt_subscriber_t *subscriber);

int wbt_mq_subscriber_has_channel(wbt_subscriber_t *subscriber, wbt_channel_t *channel);
wbt_status wbt_mq_subscriber_add_channel(wbt_subscriber_t *subscriber, wbt_channel_t *channel);
wbt_status wbt_mq_subscriber_msg_ack(wbt_subscriber_t *subscriber, wbt_mq_id msg_id);

long long int wbt_mq_subscriber_status_active();

json_object_t * wbt_mq_subscriber_print(wbt_subscriber_t *subscriber);
void wbt_mq_subscriber_print_all(json_object_t * obj);
void wbt_mq_subscriber_msg_print(wbt_subscriber_t *subscriber, json_object_t * obj);
void wbt_mq_subscriber_channel_print(wbt_subscriber_t *subscriber, json_object_t * obj);

#ifdef	__cplusplus
}
#endif

#endif	/* WBT_MQ_SUBSCRIBER_H */

