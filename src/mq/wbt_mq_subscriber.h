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

wbt_subscriber_t * wbt_mq_subscriber_create(wbt_mq_id subscriber_id);
wbt_subscriber_t * wbt_mq_subscriber_get(wbt_mq_id subscriber_id);
void wbt_mq_subscriber_destory(wbt_subscriber_t *subscriber);

#ifdef	__cplusplus
}
#endif

#endif	/* WBT_MQ_SUBSCRIBER_H */
