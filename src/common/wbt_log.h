/* 
 * File:   wbt_log.h
 * Author: Fcten
 *
 * Created on 2014年8月25日, 下午4:05
 */

#ifndef __WBT_LOG_H__
#define	__WBT_LOG_H__


#include <stdio.h>
#include <time.h>

#include "wbt_string.h"

typedef enum {
    WBT_LOG_OK,
    WBT_LOG_WARNNING,
    WBT_LOG_ERROR,
    WBT_LOG_DEBUG
} wbt_log_level_t;

int wbt_log_write(wbt_str_t p, FILE *fp);

#ifdef WBT_DEBUG
#define wbt_log_debug(fmt, arg...) \
	printf ("\033[31;49;1mDEBUG\033[0m ~ \033[32;49m%s@%d\033[0m " fmt "\n", strrchr (__FILE__, '/') == 0 ?  \
		__FILE__ : strrchr (__FILE__, '/') + 1, \
		__LINE__, ##arg);
#else
#define wbt_log_debug(fmt, arg...) ((void)0);
#endif


#endif	/* __WBT_LOG_H__ */

