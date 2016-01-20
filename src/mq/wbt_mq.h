/* 
 * File:   wbt_mq.h
 * Author: fcten
 *
 * Created on 2016年1月14日, 上午10:59
 */

#ifndef WBT_MQ_H
#define	WBT_MQ_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../webit.h"
#include "../common/wbt_log.h"
#include "../common/wbt_time.h"
#include "../common/wbt_rbtree.h"
#include "../common/wbt_heap.h"
#include "../common/wbt_list.h"
#include "../common/wbt_event.h"

enum {
    MSG_UNKNOWN,
    MSG_CREATED,   // 已创建
    MSG_EFFECTIVE, // 已生效
    MSG_DELIVERED, // 已投递
    MSG_CONSUMED,  // 已消费
    MSG_EXPIRED    // 已过期
};

enum {
    MSG_BROADCAST = 1, // 广播模式
    MSG_LOAD_BALANCE   // 负载均衡模式
};

typedef long long unsigned int wbt_mq_id;

typedef struct wbt_msg_s {
    // 消息 ID
    wbt_mq_id msg_id;
    // 消息生产者所订阅的频道 ID
    wbt_mq_id producer_id;
    // 消息消费者所订阅的频道 ID
    wbt_mq_id consumer_id;
    // 创建时间
    time_t create;
    // 生效时间
    time_t effect;
    // 过期时间
    time_t expire;
    // 优先级
    // 数值较小的消息会被优先处理
    //unsigned int priority:4;
    // 投递模式
    // 1、负载均衡模式：有多个消费者。消息以负载均衡的方式投递给某一个消费者。
    // 2、广播模式：有多个消费者。消息以广播的方式投递给所有消费者。
    unsigned int delivery_mode:4;
    // 投递次数
    unsigned int delivery_count;
    // 消费次数
    unsigned int consumption_count;
    // 引用次数
    unsigned int reference_count;
    // 消息内容
    wbt_mem_t data;
} wbt_msg_t;

typedef struct wbt_msg_list_s {
    // 链表结构体
    wbt_list_t head;
    // 消息指针
    wbt_msg_t * msg;
} wbt_msg_list_t;

typedef struct wbt_subscriber_s {
    // 订阅者 ID
    wbt_mq_id subscriber_id;
    // 创建时间
    time_t create;
    // 消息队列
    struct wbt_msg_list_s * msg_list;
    // TCP 连接上下文
    wbt_event_t * ev;
    // 订阅频道列表
    struct wbt_channel_list_s * channel_list;
    // 已投递消息队列
    // 保存已投递但订阅者尚未处理完毕的负载均衡消息
    wbt_heap_t delivered_heap;
} wbt_subscriber_t;

typedef struct wbt_subscriber_list_s {
    // 链表结构体
    wbt_list_t head;
    // 消息指针
    wbt_subscriber_t * subscriber;
} wbt_subscriber_list_t;

typedef struct wbt_channel_s {
    // 频道 ID
    wbt_mq_id channel_id;
    // 创建时间
    time_t create;
    // 订阅者
    struct wbt_subscriber_list_s * subscriber_list;
    // 广播消息队列
    // 广播消息会一直保存到消息过期，以供新上线的订阅者获得该消息
    // 负载均衡消息将保存到第一次投递成功，或者消息过期
    wbt_heap_t effective_heap;
    // 负载均衡消息队列
    struct wbt_msg_list_s * delivered_list;
} wbt_channel_t;

typedef struct wbt_channel_list_s {
    // 链表结构体
    wbt_list_t head;
    // 消息指针
    wbt_channel_t * channel;
} wbt_channel_list_t;

wbt_status wbt_mq_init();
wbt_status wbt_mq_on_recv(wbt_event_t *ev);
wbt_status wbt_mq_on_close(wbt_event_t *ev);

wbt_status wbt_mq_login(wbt_event_t *ev);
wbt_status wbt_mq_push(wbt_event_t *ev);
wbt_status wbt_mq_pull(wbt_event_t *ev);
wbt_status wbt_mq_pull_timeout(wbt_event_t *ev);
wbt_status wbt_mq_ack(wbt_event_t *ev);
wbt_status wbt_mq_status(wbt_event_t *ev);

#ifdef	__cplusplus
}
#endif

#endif	/* WBT_MQ_H */

