#include "wbt_module_helloworld.h"
#include "../common/wbt_rbtree.h"
#include "../common/wbt_connection.h"
#include "../common/wbt_config.h"
#include "../http/wbt_http.h"

wbt_module_t wbt_module_helloworld = {
    wbt_string("helloworld"),
    wbt_module_helloworld_init, // init
    wbt_module_helloworld_exit, // exit
    NULL, // on_conn
    wbt_module_helloworld_recv, // on_recv
    NULL, // on_send
    wbt_module_helloworld_close // on_close
};

wbt_rbtree_t wbt_online_fd_rbtree;
wbt_rbtree_t wbt_online_id_rbtree;

extern wbt_rbtree_node_t *wbt_rbtree_node_nil;

wbt_status wbt_module_helloworld_init() {
    /* 初始化一个红黑树用以根据 fd 查找客户端 */
    wbt_rbtree_init(&wbt_online_fd_rbtree);
    /* 初始化一个红黑树用以根据 id 查找客户端 */
    wbt_rbtree_init(&wbt_online_id_rbtree);
    
    return WBT_OK;
}

wbt_status wbt_module_helloworld_exit() {
    return WBT_OK;
}

wbt_status wbt_module_helloworld_conn(wbt_event_t *ev) {
    return WBT_OK;
}

wbt_status wbt_module_helloworld_close(wbt_event_t *ev) {
    /* 根据 fd 将客户端设置为离线 */
    wbt_str_t fd;
    fd.str = (u_char *)&ev->fd;
    fd.len = sizeof(ev->fd);

    wbt_rbtree_node_t *node_fd =  wbt_rbtree_get(&wbt_online_fd_rbtree, &fd);
    if( node_fd != NULL ) {
        wbt_online_t *p = (wbt_online_t *)node_fd->value;
        p->status = 0;
        p->offline = wbt_cur_mtime;
        
        node_fd->value = NULL;
        wbt_rbtree_delete(&wbt_online_fd_rbtree, node_fd);
    }

    return WBT_OK;
}

wbt_status wbt_module_helloworld_callback(wbt_event_t *ev) {    
    /* 生成响应数据 */
    wbt_str_t resp = wbt_string("{\"status\":0}");
    
    wbt_http_t * http = ev->data;
    
    http->state = STATE_SENDING;
    http->status = STATUS_200;
    http->file.ptr = wbt_strdup(resp.str, resp.len);
    http->file.size = resp.len;

    if( wbt_http_process(ev) != WBT_OK ) {
        wbt_conn_close(ev);
    } else {
        /* 等待socket可写 */
        ev->on_timeout = wbt_conn_close;
        ev->on_send = wbt_on_send;
        ev->events = EPOLLOUT | EPOLLET;
        ev->timeout = wbt_cur_mtime + wbt_conf.event_timeout;

        if(wbt_event_mod(ev) != WBT_OK) {
            return WBT_ERROR;
        }
    }

    return WBT_OK;
}

wbt_status wbt_module_helloworld_pull(wbt_event_t *ev) {
    wbt_http_t * http = ev->data;

    // 必须是 GET 请求
    if( http->method != METHOD_GET ) {
        http->status = STATUS_405;
        return WBT_OK;
    }

    // 必须已经登录
    wbt_str_t fd;
    fd.str = (u_char *)&ev->fd;
    fd.len = sizeof(ev->fd);

    wbt_rbtree_node_t *node_fd =  wbt_rbtree_get(&wbt_online_fd_rbtree, &fd);
    if( node_fd == NULL ) {
        http->status = STATUS_403;
        return WBT_OK;
    }

    /* 将 state 置为 STATE_BLOCKING 会使 BitMQ 继续等待数据 */
    http->state = STATE_BLOCKING;

    /* 修改超时时间为 30 秒
     * 这个时候如果继续有数据到来，BitMQ 会立刻关闭这个连接
     */
    ev->timeout = wbt_cur_mtime + 30000;
    ev->on_timeout = wbt_module_helloworld_callback;

    if(wbt_event_mod(ev) != WBT_OK) {
        return WBT_ERROR;
    }

    return WBT_OK;
}

wbt_status wbt_module_helloworld_push(wbt_event_t *ev) {
    wbt_http_t * http = ev->data;

    // 必须是 POST 请求
    if( http->method != METHOD_POST ) {
        http->status = STATUS_405;
        return WBT_OK;
    }
    
    // 处理 body
    // 前 32 位作为 ID，后面的作为 push 的数据
    if( http->body.len <= 32 ) {
        http->status = STATUS_413;
        return WBT_OK;
    }
    
    wbt_str_t id, data;
    wbt_offset_to_str(http->body, id, ev->buff);
    id.len = 32;
    data.str = id.str + 32;
    data.len = http->body.len - 32;

    // 通过ID 查找 ev
    wbt_rbtree_node_t *node_id =  wbt_rbtree_get(&wbt_online_id_rbtree, &id);
    if( node_id == NULL ) {
        http->status = STATUS_404;
        return WBT_OK;
    }
    
    wbt_online_t *p = (wbt_online_t *)node_id->value;
    if( p->status == 0 ) {
        http->status = STATUS_404;
        return WBT_OK;
    }
    
    // 如果该 ev 的超时回调函数为 wbt_module_helloworld_callback，则修改该事件并发送数据
    if( p->ev->on_timeout == wbt_module_helloworld_callback ) {
        wbt_http_t * tmp_http = p->ev->data;
        
        tmp_http->status = STATUS_200;
        tmp_http->file.ptr = wbt_strdup(data.str, data.len);
        tmp_http->file.size = data.len;

        if( wbt_http_process(p->ev) != WBT_OK ) {
            wbt_conn_close(p->ev);
        } else {
            tmp_http->state = STATE_SENDING;

            /* 等待socket可写 */
            p->ev->on_timeout = wbt_conn_close; // TODO 这个事件超时或发送失败意味着消息会丢失
            p->ev->on_send = wbt_on_send;
            p->ev->events = EPOLLOUT | EPOLLET;
            p->ev->timeout = wbt_cur_mtime + wbt_conf.event_timeout;

            if(wbt_event_mod(p->ev) != WBT_OK) {
                return WBT_ERROR;
            }

            http->status = STATUS_200;
        }

        return WBT_OK;
    } else {
        // 否则返回失败
        http->status = STATUS_403;
        return WBT_OK;
    }

    return WBT_OK;
}

wbt_status wbt_module_helloworld_recv(wbt_event_t *ev) {
    wbt_http_t * http = ev->data;

    // 只过滤 404 响应
    if( http->status != STATUS_404 ) {
        return WBT_OK;
    }
    
    wbt_str_t api = wbt_string("/api/");
    wbt_str_t login = wbt_string("/login/");
    wbt_str_t pull = wbt_string("/pull/");
    wbt_str_t push = wbt_string("/push/");
    wbt_str_t show = wbt_string("/show/");
    
    wbt_str_t http_uri;
    wbt_offset_to_str(http->uri, http_uri, ev->buff);
    
    if( wbt_strncmp( &http_uri, &api, api.len ) == 0 ) {
        return WBT_OK;
    } else if( wbt_strcmp( &http_uri, &login ) == 0 ) {
        return wbt_module_helloworld_login(ev);
    } else if( wbt_strcmp( &http_uri, &pull ) == 0 ) {
        return wbt_module_helloworld_pull(ev);
    } else if( wbt_strcmp( &http_uri, &push ) == 0 ) {
        return wbt_module_helloworld_push(ev);
    } else if( wbt_strcmp( &http_uri, &show ) == 0 ) {
        return wbt_module_helloworld_show(ev);
    }
    
    return WBT_OK;
}

wbt_status wbt_module_helloworld_login(wbt_event_t *ev) {
    wbt_http_t * http = ev->data;

    // 必须是 POST 请求
    if( http->method != METHOD_POST ) {
        http->status = STATUS_405;
        return WBT_OK;
    }
    
    // 处理 body
    if( http->body.len != 32 ) {
        http->status = STATUS_413;
        return WBT_OK;
    }
    
    // 如果在同一个连接内重复登录，先将旧的登录客户端设为离线
    wbt_module_helloworld_close(ev);
    
    wbt_online_t * client;
    
    // 记录登录 ID
    wbt_str_t http_body;
    wbt_offset_to_str(http->body, http_body, ev->buff);
    wbt_rbtree_node_t *node_id =  wbt_rbtree_get(&wbt_online_id_rbtree, &http_body);
    if( node_id == NULL ) {
        client = wbt_calloc(sizeof(wbt_online_t));
        if()

        node_id = wbt_rbtree_insert(&wbt_online_id_rbtree, &http_body);
        node_id->value = client;
    } else {
        client = node_id->value;
    }
    
    // 记录该连接
    wbt_str_t fd;
    fd.str = (u_char *)&ev->fd;
    fd.len = sizeof(ev->fd);
    wbt_rbtree_node_t *node_fd = wbt_rbtree_insert(&wbt_online_fd_rbtree, &fd);
    node_fd->value = client;
    
    wbt_online_t *p = (wbt_online_t *)client.ptr;
 
    if ( p->status == 1 && p->ev != ev ) {
        // 踢掉另一个登录的同 ID 客户端
        wbt_conn_close(p->ev);
    }

    p->ev = ev;
    wbt_mem_t tmp;
    tmp.ptr = p->id;
    tmp.len = 32;
    wbt_memcpy( &tmp, (wbt_mem_t *)&http_body, http_body.len );
    p->online = wbt_cur_mtime;
    p->status = 1;
    
    http->status = STATUS_200;
    
    return WBT_OK;
}

void wbt_module_helloworld_show_recursion(wbt_rbtree_node_t *node, wbt_str_t *resp, int maxlen) {
    wbt_online_t *p = (wbt_online_t *)node->value.ptr;

    wbt_str_t online  = wbt_string(" online   \n");
    wbt_str_t offline = wbt_string(" offline  \n");
    wbt_str_t id;
    id.str = p->id;
    id.len = 32;

    if(node != wbt_rbtree_node_nil) {
        wbt_module_helloworld_show_recursion(node->left, resp, maxlen);
        wbt_strcat(resp, &id, maxlen);
        if( p->status == 1 ) {
            wbt_strcat(resp, &online, maxlen);
        } else {
            wbt_strcat(resp, &offline, maxlen);
        }
        resp->str[resp->len-2] = p->ev->fd + '0';
        wbt_module_helloworld_show_recursion(node->right, resp, maxlen);
    }
}

wbt_status wbt_module_helloworld_show(wbt_event_t *ev) {
    wbt_http_t * http = ev->data.ptr;
    
    // 必须是 GET 请求
    if( http->method != METHOD_GET ) {
        http->status = STATUS_405;
        return WBT_OK;
    }

    wbt_mem_t tmp;
    wbt_malloc(&tmp, 10240);

    wbt_str_t resp;
    resp.len = 0;
    resp.str = tmp.ptr;

    wbt_module_helloworld_show_recursion(wbt_online_id_rbtree.root, &resp, tmp.len);
    wbt_realloc(&tmp, resp.len);
    
    http->status = STATUS_200;
    http->file.ptr = tmp.ptr;
    http->file.size = tmp.len;

    return WBT_OK;
}