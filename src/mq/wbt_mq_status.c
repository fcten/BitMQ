/* 
 * File:   wbt_mq_status.c
 * Author: fcten
 *
 * Created on 2016年2月14日, 下午4:00
 */

#include "wbt_mq_status.h"
#include "wbt_mq_channel.h"
#include "wbt_mq_msg.h"
#include "wbt_mq_subscriber.h"
#include "../json/wbt_json.h"

wbt_status wbt_mq_status(wbt_event_t *ev) {
    // 分发请求
    wbt_http_t * http = ev->data;

    // 只过滤 404 响应
    if( http->status != STATUS_404 ) {
        return WBT_OK;
    }
    
    // TODO 实际的访问控制需要自己通过 IP 实现
    wbt_str_t access_control_allow_origin = wbt_string("*");
    wbt_http_set_header( http, HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, &access_control_allow_origin );

    wbt_str_t status = wbt_string("/mq/status/");
    wbt_str_t status_system = wbt_string("/mq/status/system/");
    wbt_str_t status_msg = wbt_string("/mq/status/message/");
    wbt_str_t status_channel = wbt_string("/mq/status/channel/");
    
    wbt_str_t http_uri;
    wbt_offset_to_str(http->uri, http_uri, ev->buff);
    
    if( wbt_strcmp( &http_uri, &status ) == 0 ) {
        return wbt_mq_status_general(ev);
    } else if( wbt_strcmp( &http_uri, &status_msg ) == 0 ) {
        return wbt_mq_status_message_general(ev);
    } else if( wbt_strcmp( &http_uri, &status_channel ) == 0 ) {
        return wbt_mq_status_channel_general(ev);
    } else if( wbt_strcmp( &http_uri, &status_system ) == 0 ) {
        return wbt_mq_status_system_general(ev);
    } else if( wbt_strncmp( &http_uri, &status_msg, status_msg.len ) == 0 ) {
        return wbt_mq_status_message(ev);
    } else if( wbt_strncmp( &http_uri, &status_channel, status_channel.len ) == 0 ) {
        return wbt_mq_status_channel(ev);
    } else if( wbt_strncmp( &http_uri, &status_channel, status_channel.len ) == 0 ) {
        return wbt_mq_status_system(ev);
    }

    return WBT_OK;
}

wbt_status wbt_mq_status1(wbt_event_t *ev) {
    wbt_http_t * http = ev->data;
    
    // 必须是 GET 请求
    if( http->method != METHOD_GET ) {
        http->status = STATUS_405;
        return WBT_OK;
    }

    http->resp_body_memory.len = 10240;
    http->resp_body_memory.str = wbt_malloc(http->resp_body_memory.len);
    
    wbt_str_t resp;
    resp.len = 0;
    resp.str = http->resp_body_memory.str;
    
    wbt_str_t http_uri;
    wbt_offset_to_str(http->uri, http_uri, ev->buff);
    
    wbt_str_t channel_ids;
    channel_ids.str = http_uri.str + 11;
    channel_ids.len = http_uri.len - 11;
    if( channel_ids.len != 16 ) {
        wbt_mq_print_channels(&resp, http->resp_body_memory.len);
    } else {
        wbt_mq_id channel_id = wbt_str_to_ull(&channel_ids, 16);
        wbt_mq_print_channel(channel_id, &resp, http->resp_body_memory.len);
    }

    http->resp_body_memory.str = wbt_realloc(http->resp_body_memory.str, resp.len);
    http->resp_body_memory.len = resp.len;
    http->status = STATUS_200;

    return WBT_OK;
}

static wbt_str_t wbt_str_message = wbt_string("message");
static wbt_str_t wbt_str_channel = wbt_string("channel");
static wbt_str_t wbt_str_subscriber = wbt_string("subscriber");
static wbt_str_t wbt_str_total = wbt_string("total");
static wbt_str_t wbt_str_active = wbt_string("active");
static wbt_str_t wbt_str_delayed = wbt_string("delayed");
static wbt_str_t wbt_str_waiting_ack = wbt_string("waiting_ack");
static wbt_str_t wbt_str_system = wbt_string("system");
static wbt_str_t wbt_str_uptime = wbt_string("uptime");

wbt_status wbt_mq_status_general(wbt_event_t *ev) {
    wbt_http_t * http = ev->data;
    
    long long int message_total = wbt_mq_msg_status_total();
    long long int message_active = wbt_mq_msg_status_active();
    long long int message_delayed = wbt_mq_msg_status_delayed();
    long long int message_waiting_ack = wbt_mq_msg_status_waiting_ack();
    long long int channel_active = wbt_mq_channel_status_active();
    long long int subscriber_active = wbt_mq_subscriber_status_active();
    long long int system_uptime = wbt_mq_uptime();
    
    json_object_t * obj        = json_create_object();
    json_object_t * message    = json_create_object();
    json_object_t * channel    = json_create_object();
    json_object_t * subscriber = json_create_object();
    json_object_t * system     = json_create_object();
    
    json_append(obj, wbt_str_message.str,    wbt_str_message.len,    JSON_OBJECT, message,    0);
    json_append(obj, wbt_str_channel.str,    wbt_str_channel.len,    JSON_OBJECT, channel,    0);
    json_append(obj, wbt_str_subscriber.str, wbt_str_subscriber.len, JSON_OBJECT, subscriber, 0);
    json_append(obj, wbt_str_system.str,     wbt_str_system.len,     JSON_OBJECT, system,     0);

    json_append(message, wbt_str_total.str,       wbt_str_total.len,       JSON_LONGLONG, &message_total,       0);
    json_append(message, wbt_str_active.str,      wbt_str_active.len,      JSON_LONGLONG, &message_active,      0);
    json_append(message, wbt_str_delayed.str,     wbt_str_delayed.len,     JSON_LONGLONG, &message_delayed,     0);
    json_append(message, wbt_str_waiting_ack.str, wbt_str_waiting_ack.len, JSON_LONGLONG, &message_waiting_ack, 0);

    json_append(channel, wbt_str_active.str, wbt_str_active.len, JSON_LONGLONG, &channel_active, 0);

    json_append(subscriber, wbt_str_active.str, wbt_str_active.len, JSON_LONGLONG, &subscriber_active, 0);
    
    json_append(system, wbt_str_uptime.str, wbt_str_uptime.len, JSON_LONGLONG, &system_uptime, 0);

    http->resp_body_memory.len = 1024;
    http->resp_body_memory.str = wbt_malloc( http->resp_body_memory.len );
    if( http->resp_body_memory.str == NULL ) {
        http->status = STATUS_503;
        return WBT_OK;
    }
    char *ptr = http->resp_body_memory.str;
    size_t len = http->resp_body_memory.len;
    json_print(obj, &ptr, &len);
    http->resp_body_memory.len = http->resp_body_memory.len - len;
    http->resp_body_memory.str = wbt_realloc( http->resp_body_memory.str, http->resp_body_memory.len );

    json_delete_object(obj);
    
    http->status = STATUS_200;

    return WBT_OK;
}

wbt_status wbt_mq_status_message_general(wbt_event_t *ev) {
    return wbt_mq_status1(ev);
}

wbt_status wbt_mq_status_channel_general(wbt_event_t *ev) {
    return wbt_mq_status1(ev);
}

wbt_status wbt_mq_status_system_general(wbt_event_t *ev) {
    return wbt_mq_status1(ev);
}

wbt_status wbt_mq_status_message(wbt_event_t *ev) {
    return wbt_mq_status1(ev);
}

wbt_status wbt_mq_status_channel(wbt_event_t *ev) {
    return wbt_mq_status1(ev);
}

wbt_status wbt_mq_status_system(wbt_event_t *ev) {
    return wbt_mq_status1(ev);
}