include_directories(${PROJECT_SOURCE_DIR}/src ${OPENSSL_INCLUDE_DIR})

set (WBT_SRCS
	webit.c
	webit.h
	common/wbt_config.c
	common/wbt_config.h
	common/wbt_connection.c
	common/wbt_connection.h
	common/wbt_crc.c
	common/wbt_crc.h
	common/wbt_file.c
	common/wbt_file.h
	common/wbt_gzip.c
	common/wbt_gzip.h
	common/wbt_heap.c
	common/wbt_heap.h
	common/wbt_list.c
	common/wbt_list.h
	common/wbt_log.c
	common/wbt_log.h
	common/wbt_memory.c
	common/wbt_memory.h
	common/wbt_module.c
	common/wbt_module.h
	common/wbt_rbtree.c
	common/wbt_rbtree.h
	common/wbt_ssl.c
	common/wbt_ssl.h
	common/wbt_string.c
	common/wbt_string.h
	common/wbt_time.c
	common/wbt_time.h
	common/wbt_timer.c
	common/wbt_timer.h
	common/wbt_base64.c
	common/wbt_base64.h
	common/wbt_auth.c
	common/wbt_auth.h
	event/wbt_event.h
	http/wbt_http.c
	http/wbt_http.h
	http/wbt_http_consts.c
	http/wbt_http_mq.c
	http/wbt_http_mq.h
	json/wbt_json.c
	json/wbt_json.h
	mq/wbt_mq.c
	mq/wbt_mq.h
	mq/wbt_mq_auth.c
	mq/wbt_mq_auth.h
	mq/wbt_mq_channel.c
	mq/wbt_mq_channel.h
	mq/wbt_mq_dlq.c
	mq/wbt_mq_dlq.h
	mq/wbt_mq_msg.c
	mq/wbt_mq_msg.h
	mq/wbt_mq_persistence.c
	mq/wbt_mq_persistence.h
	mq/wbt_mq_subscriber.c
	mq/wbt_mq_subscriber.h
	mq/wbt_mq_replication.c
	mq/wbt_mq_replication.h
	mq/wbt_mq_cluster.c
	mq/wbt_mq_cluster.h
	mq/wbt_mq_snowflake.c
	mq/wbt_mq_snowflake.h
	websocket/wbt_websocket.c
	websocket/wbt_websocket.h
	bmtp2/wbt_bmtp2.c
	bmtp2/wbt_bmtp2.h
	bmtp2/wbt_bmtp2_conn.c
	bmtp2/wbt_bmtp2_connack.c
	bmtp2/wbt_bmtp2_pub.c
	bmtp2/wbt_bmtp2_puback.c
	bmtp2/wbt_bmtp2_sub.c
	bmtp2/wbt_bmtp2_suback.c
	bmtp2/wbt_bmtp2_ping.c
	bmtp2/wbt_bmtp2_pingack.c
	bmtp2/wbt_bmtp2_disconn.c
	bmtp2/wbt_bmtp2_window.c
	bmtp2/wbt_bmtp2_sync.c)

if (UNIX)
	set (WBT_SRCS ${WBT_SRCS}
	event/wbt_event_epoll.c
	event/wbt_event_epoll.h
	os/linux/wbt_os_util.c
	os/linux/wbt_os_util.h
	os/linux/wbt_process.c
	os/linux/wbt_process.h
	os/linux/wbt_setproctitle.c
	os/linux/wbt_setproctitle.h
	os/linux/wbt_terminal.c
	os/linux/wbt_terminal.h)
endif (UNIX)

if (WIN32 OR CYGWIN)
	set (WBT_SRCS ${WBT_SRCS}
	event/wbt_event_select.c
	event/wbt_event_select.h
	os/win32/wbt_os_util.c
	os/win32/wbt_os_util.h
	os/win32/wbt_service.c
	os/win32/wbt_service.h)
endif (WIN32 OR CYGWIN)

add_executable(bmq ${WBT_SRCS})

set (WBT_SIGN_SRCS
	webit_sign.c
	common/wbt_base64.c
	common/wbt_base64.h)

add_executable(bmq_sign ${WBT_SIGN_SRCS})

set (WBT_LIBS ${WBT_LIBS} ${OPENSSL_LIBRARIES})

if (UNIX)
	set (WBT_LIBS ${WBT_LIBS} dl z)
endif (UNIX)

if (WIN32)
	set (WBT_LIBS ${WBT_LIBS} ws2_32)
endif (WIN32)

target_link_libraries(bmq ${WBT_LIBS})
target_link_libraries(bmq_sign ${WBT_LIBS})

install(TARGETS bmq RUNTIME DESTINATION ${BINDIR} LIBRARY DESTINATION ${LIBDIR})
install(FILES DESTINATION ${INCLUDEDIR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

if (UNIX)
	install(CODE "EXEC_PROGRAM(/sbin/ldconfig)")
endif (UNIX)

