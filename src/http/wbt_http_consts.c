/* 
 * File:   wbt_http_consts.c
 * Author: Fcten
 *
 * Created on 2014年10月24日, 下午3:40
 */

#include "../webit.h"
#include "../common/wbt_string.h"

wbt_str_t http_ver_1_0 = wbt_string("HTTP/1.0");
wbt_str_t http_ver_1_1 = wbt_string("HTTP/1.1");

wbt_str_t header_server = wbt_string("BitMQ");
wbt_str_t header_connection_keep_alive = wbt_string("keep-alive");
wbt_str_t header_connection_close = wbt_string("close");
wbt_str_t header_cache_control = wbt_string("max-age=3600");
wbt_str_t header_content_type_text_html = wbt_string("text/html");
wbt_str_t header_cache_control_no_cache = wbt_string("no-store, no-cache, must-revalidate");
wbt_str_t header_pragma_no_cache = wbt_string("no-cache");
wbt_str_t header_expires_no_cache = wbt_string("Sat, 26 Jul 1997 05:00:00 GMT");
wbt_str_t header_encoding_gzip = wbt_string("gzip");

wbt_str_t REQUEST_METHOD[] = {
    wbt_null_string,
    wbt_string("GET"),
    wbt_string("POST"),
    wbt_string("HEAD"),
    wbt_string("PUT"), 
    wbt_string("DELETE"), 
    wbt_string("TRACE"),
    wbt_string("CONNECT"),
    wbt_string("OPTIONS"),
    wbt_null_string
};

wbt_str_t HTTP_HEADERS[] = {
    wbt_null_string,
    wbt_string("Cache-Control"),
    wbt_string("Connection"),
    wbt_string("Date"),
    wbt_string("Pragma"),
    wbt_string("Trailer"),
    wbt_string("Transfer-Encoding"),
    wbt_string("Upgrade"),
    wbt_string("Via"),
    wbt_string("Warning"),
    wbt_string("Accept"),
    wbt_string("Accept-Charset"),
    wbt_string("Accept-Encoding"),
    wbt_string("Accept-Language"),
    wbt_string("Authorization"),
    wbt_string("Expect"),
    wbt_string("From"),
    wbt_string("Host"),
    wbt_string("If-Match"),
    wbt_string("If-Modified-Since"),
    wbt_string("If-None-Match"),
    wbt_string("If-Range"),
    wbt_string("If-Unmodified-Since"),
    wbt_string("Max-Forwards"),
    wbt_string("Proxy-Authorization"),
    wbt_string("Range"),
    wbt_string("Referer"),
    wbt_string("TE"),
    wbt_string("User-Agent"),
    wbt_string("Accept-Ranges"),
    wbt_string("Age"),
    wbt_string("Etag"),
    wbt_string("Location"),
    wbt_string("Proxy-Autenticate"),
    wbt_string("Retry-After"),
    wbt_string("Server"),
    wbt_string("Vary"),
    wbt_string("WWW-Authenticate"),
    wbt_string("Allow"),
    wbt_string("Content-Encoding"),
    wbt_string("Content-Language"),
    wbt_string("Content-Length"),
    wbt_string("Content-Location"),
    wbt_string("Content-MD5"),
    wbt_string("Content-Range"),
    wbt_string("Content-Type"),
    wbt_string("Expires"),
    wbt_string("Last-Modified"),
    wbt_string("Cookie"),
    wbt_string("Set-Cookie"),
    wbt_string("Access-Control-Allow-Origin"),
    wbt_null_string
};

wbt_str_t STATUS_CODE[] = {
    wbt_null_string,
    wbt_string("100 Continue"),
    wbt_string("101 Switching Protocols"), 
    wbt_string("102 Processing"),
    wbt_string("200 OK"),
    wbt_string("201 Created"),
    wbt_string("202 Accepted"),
    wbt_string("203 Non-Authoritative Information"),
    wbt_string("204 No Content"),
    wbt_string("205 Reset Content"),
    wbt_string("206 Partial Content"),
    wbt_string("207 Multi-Status"),
    wbt_string("300 Multiple Choices"),
    wbt_string("301 Moved Permanently"),
    wbt_string("302 Found"),
    wbt_string("302 Moved Temporarily"), // 注意：这并不是一个标准的 HTTP/1.1 状态码，只是为了兼容而添加
    wbt_string("303 See Other"),
    wbt_string("304 Not Modified"),
    wbt_string("305 Use Proxy"),
    wbt_string("307 Temporary Redirect"),
    wbt_string("400 Bad Request"),
    wbt_string("401 Unauthorized"),
    wbt_string("402 Payment Required"),
    wbt_string("403 Forbidden"),
    wbt_string("404 Not Found"),
    wbt_string("405 Method Not Allowed"),
    wbt_string("406 Not Acceptable"),
    wbt_string("407 Proxy Authentication Required"),
    wbt_string("408 Request Time-out"),
    wbt_string("409 Conflict"),
    wbt_string("410 Gone"),
    wbt_string("411 Length Required"),
    wbt_string("412 Precondition Failed"),
    wbt_string("413 Request Entity Too Large"),
    wbt_string("414 Request-URI Too Large"),
    wbt_string("415 Unsupported Media Type"),
    wbt_string("416 Requested range not satisfiable"),
    wbt_string("417 Expectation Failed"),
    wbt_string("422 Unprocessable Entity"),
    wbt_string("423 Locked"),
    wbt_string("424 Failed Dependency"),
    wbt_string("426 Upgrade Required"),
    wbt_string("500 Internal Server Error"),
    wbt_string("501 Not Implemented"),
    wbt_string("502 Bad Gateway"),
    wbt_string("503 Service Unavailable"),
    wbt_string("504 Gateway Time-out"),
    wbt_string("505 HTTP Version not supported"),
    wbt_string("506 Variant Also Negotiates"),
    wbt_string("507 Insufficient Storage"),
    wbt_string("510 Not Extended"),
    wbt_null_string
};

wbt_str_t wbt_http_error_page[] = {
    wbt_null_string,  /* STATUS UNKNOWN */
    wbt_string("100 Continue"),
    wbt_string("101 Switching Protocols"), 
    wbt_string("102 Processing"),
    wbt_string("200 OK"),
    wbt_string("201 Created"),
    wbt_string("202 Accepted"),
    wbt_string("203 Non-Authoritative Information"),
    wbt_null_string,
    wbt_string("205 Reset Content"),
    wbt_string("206 Partial Content"),
    wbt_string("207 Multi-Status"),
    wbt_string("300 Multiple Choices"),
    wbt_string("301 Moved Permanently"),
    wbt_string("302 Found"),
    wbt_string("302 Moved Temporarily"), // 注意：这并不是一个标准的 HTTP/1.1 状态码，只是为了兼容而添加
    wbt_string("303 See Other"),
    wbt_null_string,
    wbt_string("305 Use Proxy"),
    wbt_string("307 Temporary Redirect"),
    wbt_string(  /* STATUS 400 */
    "<html>" CRLF
    "<head><title>400 Bad Request</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>400 Bad Request</h1></center>" CRLF
    "<!-- BitMQ -->" CRLF
    "</body>" CRLF
    "</html>"
    ),wbt_string("401 Unauthorized"),
    wbt_string("402 Payment Required"),
    wbt_string(  /* STATUS 403 */
    "<html>" CRLF
    "<head><title>403 Forbidden</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>403 Forbidden</h1></center>" CRLF
    "<!-- BitMQ -->" CRLF
    "</body>" CRLF
    "</html>"
    ),wbt_string(  /* STATUS 404 */
    "<html>" CRLF
    "<head><title>404 Not Found</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>404 Not Found</h1></center>" CRLF
    "<!-- BitMQ -->" CRLF
    "</body>" CRLF
    "</html>"
    ),
    wbt_string("405 Method Not Allowed"),
    wbt_string("406 Not Acceptable"),
    wbt_string("407 Proxy Authentication Required"),
    wbt_string("408 Request Time-out"),
    wbt_string("409 Conflict"),
    wbt_string("410 Gone"),
    wbt_string("411 Length Required"),
    wbt_string("412 Precondition Failed"),
    wbt_string(  /* STATUS 413 */
    "<html>" CRLF
    "<head><title>413 Request Entity Too Large</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>413 Request Entity Too Large</h1></center>" CRLF
    "<!-- BitMQ -->" CRLF
    "</body>" CRLF
    "</html>"),
    wbt_string(  /* STATUS 414 */
    "<html>" CRLF
    "<head><title>414 Request-URI Too Large</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>414 Request-URI Too Large</h1></center>" CRLF
    "<!-- BitMQ -->" CRLF
    "</body>" CRLF
    "</html>"),
    wbt_string("415 Unsupported Media Type"),
    wbt_string("416 Requested range not satisfiable"),
    wbt_string("417 Expectation Failed"),
    wbt_string("422 Unprocessable Entity"),
    wbt_string("423 Locked"),
    wbt_string("424 Failed Dependency"),
    wbt_string("426 Upgrade Required"),
    wbt_string(
    "<html>" CRLF
    "<head><title>500 Internal Server Error</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>500 Internal Server Error</h1></center>" CRLF
    "<!-- BitMQ -->" CRLF
    "</body>" CRLF
    "</html>"
    ),
    wbt_string("501 Not Implemented"),
    wbt_string("502 Bad Gateway"),
    wbt_string("503 Service Unavailable"),
    wbt_string("504 Gateway Time-out"),
    wbt_string("505 HTTP Version not supported"),
    wbt_string("506 Variant Also Negotiates"),
    wbt_string("507 Insufficient Storage"),
    wbt_string("510 Not Extended"),
    wbt_null_string
};
