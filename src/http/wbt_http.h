/* 
 * File:   wbt_http.h
 * Author: Fcten
 *
 * Created on 2014年10月24日, 下午3:11
 */

#ifndef __WBT_HTTP_H__
#define	__WBT_HTTP_H__

#ifdef	__cplusplus
extern "C" {
#endif


#include "../webit.h"
#include "../common/wbt_memory.h"
#include "../common/wbt_string.h"
#include "../common/wbt_file.h"
#include "../common/wbt_auth.h"
#include "../common/wbt_base64.h"
#include "../json/wbt_json.h"
#include "../mq/wbt_mq.h"
#include "../mq/wbt_mq_msg.h"
#include "../mq/wbt_mq_auth.h"
#include "../event/wbt_event.h"

typedef enum { 
    METHOD_UNKNOWN,
    METHOD_GET,
    METHOD_POST,
    METHOD_HEAD,
    METHOD_PUT, 
    METHOD_DELETE, 
    METHOD_TRACE,
    METHOD_CONNECT,
    METHOD_OPTIONS,
    METHOD_LENGTH
} wbt_http_method_t;

typedef enum {
    HEADER_UNKNOWN,
    HEADER_CACHE_CONTROL,
    HEADER_CONNECTION,
    HEADER_DATE,
    HEADER_PRAGMA,
    HEADER_TRAILER,
    HEADER_TRANSFER_ENCODING,
    HEADER_UPGRADE,
    HEADER_VIA,
    HEADER_WARNING,
    HEADER_ACCEPT,
    HEADER_ACCEPT_CHARSET,
    HEADER_ACCEPT_ENCODING,
    HEADER_ACCEPT_LANGUAGE,
    HEADER_AUTHORIZATION,
    HEADER_EXPECT,
    HEADER_FROM,
    HEADER_HOST,
    HEADER_IF_MATCH,
    HEADER_IF_MODIFIED_SINCE,
    HEADER_IF_NONE_MATCH,
    HEADER_IF_RANGE,
    HEADER_IF_UNMODIFIED_SINCE,
    HEADER_MAX_FORWARDS,
    HEADER_PROXY_AUTHORIZATION,
    HEADER_RANGE,
    HEADER_REFERER,
    HEADER_TE,
    HEADER_USER_AGENT,
    HEADER_ACCEPT_RANGES,
    HEADER_AGE,
    HEADER_ETAG,
    HEADER_LOCATION,
    HEADER_PROXY_AUTENTICATE,
    HEADER_RETRY_AFTER,
    HEADER_SERVER,
    HEADER_VARY,
    HEADER_WWW_AUTHENTICATE,
    HEADER_ALLOW,
    HEADER_CONTENT_ENCODING,
    HEADER_CONTENT_LANGUAGE,
    HEADER_CONTENT_LENGTH,
    HEADER_CONTENT_LOCATION,
    HEADER_CONTENT_MD5,
    HEADER_CONTENT_RANGE,
    HEADER_CONTENT_TYPE,
    HEADER_EXPIRES,
    HEADER_LAST_MODIFIED,
    HEADER_COOKIE,
    HEADER_SET_COOKIE,
    HEADER_ACCESS_CONTROL_ALLOW_ORIGIN,
#ifdef WITH_WEBSOCKET
    HEADER_SEC_WEBSOCKET_KEY,
    HEADER_SEC_WEBSOCKET_VERSION,
    HEADER_SEC_WEBSOCKET_PROTOCOL,
    HEADER_SEC_WEBSOCKET_EXTENSIONS,
    HEADER_SEC_WEBSOCKET_ACCEPT,
#endif
    HEADER_LENGTH
} wbt_http_line_t;

typedef enum {
    STATUS_UNKNOWN,  // Unknown
    STATUS_100,  // Continue
    STATUS_101,  // Switching Protocols
    STATUS_102,  // Processing
    STATUS_200,  // OK
    STATUS_201,  // Created
    STATUS_202,  // Accepted
    STATUS_203,  // Non_Authoritative Information
    STATUS_204,  // No Content
    STATUS_205,  // Reset Content
    STATUS_206,  // Partial Content
    STATUS_207,  // Multi-Status
    STATUS_300,  // Multiple Choices
    STATUS_301,  // Moved Permanently
    STATUS_302,  // Found
    STATUS_302_1,// Moved Temporarily
    STATUS_303,  // See Other
    STATUS_304,  // Not Modified
    STATUS_305,  // Use Proxy
    STATUS_307,  // Temporary Redirect
    STATUS_400,  // Bad Request
    STATUS_401,  // Unauthorized
    STATUS_402,  // Payment Required
    STATUS_403,  // Forbidden
    STATUS_404,  // Not Found
    STATUS_405,  // Method Not Allowed
    STATUS_406,  // Not Acceptable
    STATUS_407,  // Proxy Authentication Required
    STATUS_408,  // Request Time_out
    STATUS_409,  // Conflict
    STATUS_410,  // Gone
    STATUS_411,  // Length Required
    STATUS_412,  // Precondition Failed
    STATUS_413,  // Request Entity Too Large
    STATUS_414,  // Request_URI Too Large
    STATUS_415,  // Unsupported Media Type
    STATUS_416,  // Requested range not satisfiable
    STATUS_417,  // Expectation Failed
    STATUS_422,  // Unprocessable Entity
    STATUS_423,  // Locked
    STATUS_424,  // Failed Dependency
    STATUS_426,  // Upgrade Required
    STATUS_500,  // Internal Server Error
    STATUS_501,  // Not Implemented
    STATUS_502,  // Bad Gateway
    STATUS_503,  // Service Unavailable
    STATUS_504,  // Gateway Time_out
    STATUS_505,  // HTTP Version not supported
    STATUS_506,  // Variant Also Negotiates
    STATUS_507,  // Insufficient Storage
    STATUS_510,  // Not Extended
    STATUS_LENGTH
} wbt_http_status_t;

typedef enum {
    STATE_UNKNOWN,  // Unknown
    STATE_CONNECTION_ESTABLISHED,
    STATE_RECEIVING_HEADER,
    STATE_PARSING_HEADER,
    STATE_RECEIVING_BODY,
    STATE_PARSING_BODY,
    STATE_PARSING_REQUEST,
    STATE_READ_FILE,
    STATE_BLOCKING,
    STATE_READY_TO_SEND,
    STATE_SENDING,
    STATE_SEND_COMPLETED,
    STATE_CONNECTION_CLOSED,
    STATE_LENGTH
} wbt_http_state_t;

typedef struct wbt_http_header_s {
    wbt_http_line_t key;
    wbt_str_union_t name;
    wbt_str_union_t value;
    struct wbt_http_header_s * next;
} wbt_http_header_t;
    
typedef struct wbt_http_s {
    /* 以下变量用于保存接收到的请求数据的解析结果 */
    wbt_http_method_t method;
    wbt_str_offset_t uri;
    wbt_str_offset_t version;
    wbt_http_header_t * headers;
    wbt_str_offset_t body;
    /* 以下变量用于保存响应数据 */
    wbt_http_status_t status;
    wbt_http_header_t * resp_headers;
    wbt_str_t resp_header;
    off_t header_offset;
    wbt_str_t resp_body_memory;
    wbt_str_t resp_body_gzip;
    wbt_file_t * resp_body_file;
    off_t body_offset;
    /* 以下变量用于保存请求头的解析结果 */
    unsigned int bit_flag;
    /* 以下变量用于保存请求处理状态 */
    wbt_http_state_t state;
    
    int is_auth;
    wbt_auth_t *auth;
} wbt_http_t;

#define WBT_HTTP_KEEP_ALIVE         1
#define WBT_HTTP_IF_MODIFIED_SINCE  2
#define WBT_HTTP_GZIP               4

extern wbt_str_t REQUEST_METHOD[];
extern wbt_str_t HTTP_HEADERS[];
extern wbt_str_t STATUS_CODE[];
extern wbt_str_t wbt_http_error_page[];

extern wbt_str_t http_ver_1_0;
extern wbt_str_t http_ver_1_1;

extern wbt_str_t header_server;
extern wbt_str_t header_connection_keep_alive;
extern wbt_str_t header_connection_close;
extern wbt_str_t header_cache_control;
extern wbt_str_t header_content_type_text_html;
extern wbt_str_t header_cache_control_no_cache;
extern wbt_str_t header_pragma_no_cache;
extern wbt_str_t header_expires_no_cache;
extern wbt_str_t header_encoding_gzip;

wbt_status wbt_http_init();
wbt_status wbt_http_exit();
wbt_status wbt_http_check_header_end( wbt_event_t * );
wbt_status wbt_http_parse_request_header( wbt_event_t * );
wbt_status wbt_http_check_body_end( wbt_event_t * );
wbt_status wbt_http_set_header( wbt_http_t*, wbt_http_line_t, wbt_str_t* );
wbt_status wbt_http_generate_response_header( wbt_http_t * );

wbt_status wbt_http_process( wbt_event_t * );

wbt_status wbt_http_on_conn( wbt_event_t * );
wbt_status wbt_http_on_recv( wbt_event_t * );
wbt_status wbt_http_on_send( wbt_event_t * );
wbt_status wbt_http_on_close( wbt_event_t * );

wbt_status wbt_http_generater( wbt_event_t * );

#ifdef	__cplusplus
}
#endif

#endif	/* __WBT_HTTP_H__ */

