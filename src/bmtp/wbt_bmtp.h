/* 
 * File:   wbt_bmtp.h
 * Author: fcten
 *
 * Created on 2016年2月26日, 下午4:42
 */

#ifndef WBT_BMTP_H
#define	WBT_BMTP_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../common/wbt_list.h"
#include "../common/wbt_event.h"

#define BMTP_VERSION 0x1

/*
 +---------+-----------------------------------------------+
 |   Bit   |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 +---------+-----------------------------------------------+
 |         |      Message Type     |   Protocol Version    |
 +  Byte1  +-----------------------------------------------+
 |         |  0  |  0  |  0  |  1  |  0  |  0  |  0  |  1  |
 +---------+-----------------------------------------------+
 |  Byte2  |                      'B'                      |
 +---------+-----------------------------------------------+
 |  Byte3  |                      'M'                      |
 +---------+-----------------------------------------------+
 |  Byte4  |                      'T'                      |
 +---------+-----------------------------------------------+
 |  Byte5  |                      'P'                      |
 +---------+-----------------------------------------------+

 协议版本（Protocol Version）的值必须为 0x1，如果不是，服务端应当在返回 CONNACK 后关闭连接

 BMTP_CONN 必须是连接建立后客户端发送的第一个请求，如果不是，服务端必须关闭连接
 如果一个连接重复收到 BMTP_CONN 请求，服务端必须关闭连接
*/

#define BMTP_CONN 0x10

#define wbt_bmtp_cmd(b)     (b&240)
#define wbt_bmtp_version(b) (b&15)

/* 
 +---------+-----------------------------------------------+
 |   Bit   |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 +---------+-----------------------------------------------+
 |         |      Message Type     |      Status Code      |
 +  Byte1  +-----------------------------------------------+
 |         |  0  |  0  |  1  |  0  |  X  |  X  |  X  |  X  |
 +---------+-----------------------------------------------+

 Status Code:
      0: 接受
      1: 拒绝，不支持的协议版本
      2: 拒绝，服务暂时不可用
   3-15: 保留

 如果状态码不等于 0x0，服务端必须在发送数据包后关闭连接，客户端应当在接收数据包后关闭连接
*/

#define BMTP_CONNACK 0x20

#define wbt_bmtp_status(b) (b&15)
    
/* 
 +---------+-----------------------------------------------+
 |   Bit   |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 +---------+-----------------------------------------------+
 |         |      Message Type     | DUP |  R  |    QoS    |
 |  Byte1  +-----------------------------------------------+
 |         |  0  |  0  |  1  |  1  |  X  |  0  |  X  |  X  |
 +---------+-----------------------------------------------+
 |  Byte2  |                   Stream ID                   |
 +---------+-----------------------------------------------+
 |  Byte3  |                Payload Length                 |
 |  Byte4  |                                               |
 +---------+-----------------------------------------------+
 |  Byte5+ |                    Payload                    |

 DUP: 重发标志
 QoS: 服务质量等级 0-至多一次 1-至少一次 2-恰好一次
*/

#define BMTP_PUB 0x30

#define wbt_bmtp_dup(b)     (b&8)
#define wbt_bmtp_qos(b)     (b&3)

#define BMTP_QOS_AT_MOST_ONCE  0
#define BMTP_QOS_AT_LEAST_ONCE 1
#define BMTP_QOS_EXACTLY_ONCE  2

#define BMTP_DUP  8

/* 
 +---------+-----------------------------------------------+
 |   Bit   |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 +---------+-----------------------------------------------+
 |         |      Message Type     |       Reserved        |
 |  Byte1  +-----------------------------------------------+
 |         |  0  |  1  |  0  |  0  |  0  |  0  |  0  |  0  |
 +---------+-----------------------------------------------+
 |  Byte2  |                   Stream ID                   |
 +---------+-----------------------------------------------+
*/

#define BMTP_PUBACK 0x40

/* 
 +---------+-----------------------------------------------+
 |   Bit   |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 +---------+-----------------------------------------------+
 |         |      Message Type     |       Reserved        |
 |  Byte1  +-----------------------------------------------+
 |         |  1  |  1  |  0  |  0  |  0  |  0  |  0  |  0  |
 +---------+-----------------------------------------------+
 */

#define BMTP_PING 0xC0

/* 
 +---------+-----------------------------------------------+
 |   Bit   |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 +---------+-----------------------------------------------+
 |         |      Message Type     |       Reserved        |
 |  Byte1  +-----------------------------------------------+
 |         |  1  |  1  |  0  |  1  |  0  |  0  |  0  |  0  |
 +---------+-----------------------------------------------+
 */

#define BMTP_PINGACK 0xD0

/* 
 +---------+-----------------------------------------------+
 |   Bit   |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 +---------+-----------------------------------------------+
 |         |      Message Type     |       Reserved        |
 |  Byte1  +-----------------------------------------------+
 |         |  1  |  1  |  1  |  0  |  0  |  0  |  0  |  0  |
 +---------+-----------------------------------------------+

 客户端或服务端发送 BMTP_DISCONN 后必须关闭连接，另一方在接收到 BMTP_DISCONN 后应当关闭连接
 */

#define BMTP_DISCONN 0xE0

typedef struct wbt_bmtp_msg_s {
    wbt_list_t head;

    unsigned int  len;
    unsigned int  offset;
    unsigned char *msg;
} wbt_bmtp_msg_t;

typedef struct {
    // 会话状态
    unsigned int  state;
    unsigned char header;
    unsigned char sid;
    unsigned int  payload_length;
    unsigned char *payload;
    unsigned int  recv_offset;
    
    unsigned int  is_conn:1;
    unsigned int  is_exit:1;
    
    unsigned int  last_sid:8;
    unsigned int  usable_sids:8;
    unsigned int  page[8];
    
    wbt_bmtp_msg_t wait_queue;
    wbt_bmtp_msg_t send_queue;
    wbt_bmtp_msg_t ack_queue;
} wbt_bmtp_t;

#ifdef	__cplusplus
}
#endif

#endif	/* WBT_BMTP_H */

