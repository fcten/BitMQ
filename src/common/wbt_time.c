/* 
 * File:   wbt_time.c
 * Author: Fcten
 *
 * Created on 2015年1月9日, 上午10:51
 */

#include "../webit.h"
#include "wbt_time.h"
#include "wbt_memory.h"
#include "wbt_log.h"
#include "wbt_module.h"

wbt_module_t wbt_module_time = {
    wbt_string("time"),
    wbt_time_init
};

/* 当前时间戳，单位毫秒 */
time_t cur_mtime;

wbt_str_t wbt_time_str_log;
wbt_str_t wbt_time_str_http;

wbt_mem_t wbt_time_struct_log;
wbt_mem_t wbt_time_struct_http;

wbt_status wbt_time_init() {
    wbt_malloc( &wbt_time_struct_log, sizeof("[05/06/91 00:00:00] ") );
    wbt_malloc( &wbt_time_struct_http, sizeof("Mon, 28 Sep 1970 06:00:00 GMT") );
    
    wbt_time_str_log.str = wbt_time_struct_log.ptr;
    wbt_time_str_log.len = wbt_time_struct_log.len - 1;

    wbt_time_str_http.str = wbt_time_struct_http.ptr;
    wbt_time_str_http.len = wbt_time_struct_http.len - 1;
    
    wbt_time_update();

    return WBT_OK;
}

wbt_status wbt_time_update() {
    struct timeval cur_utime;
    gettimeofday(&cur_utime, NULL);
    cur_mtime = 1000 * cur_utime.tv_sec + cur_utime.tv_usec / 1000;

    time_t now;
    struct tm *timenow;

    now = time(NULL);
    timenow = localtime(&now);
    strftime(wbt_time_struct_log.ptr, wbt_time_struct_log.len, "[%x %X] ", timenow);
    timenow = gmtime(&now);
    strftime(wbt_time_struct_http.ptr, wbt_time_struct_http.len, "%a, %d %b %Y %T GMT", timenow);
    
    wbt_log_debug("%.*s",  wbt_time_struct_log.len-1, wbt_time_struct_log.ptr);
    wbt_log_debug("%.*s",  wbt_time_struct_http.len-1, wbt_time_struct_http.ptr);
    
    return WBT_OK;
}