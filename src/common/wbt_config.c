/* 
 * File:   wbt_config.c
 * Author: Fcten
 *
 * Created on 2014年12月16日, 下午3:03
 */

#include "wbt_config.h"
#include "wbt_module.h"
#include "wbt_file.h"
#include "wbt_log.h"
#include "wbt_rbtree.h"
#include "../os/linux/wbt_os_util.h"
#include "../os/linux/wbt_setproctitle.h"

wbt_module_t wbt_module_conf = {
    wbt_string("config"),
    wbt_conf_init,
    wbt_conf_exit
};

/* 默认的配置文件位置 */
wbt_str_t wbt_config_file_name = wbt_string("./webit.conf");

wbt_str_t wbt_conf_option_on = wbt_string("on");
wbt_str_t wbt_conf_option_off = wbt_string("off");

/* 供 setproctitle 显示使用，如果路径过长则会被截断 */
wbt_mem_t wbt_config_file_path;

wbt_mem_t wbt_config_file_content;

wbt_rbtree_t wbt_config_rbtree;

int wbt_conf_line = 1;
int wbt_conf_charactor = 0;

wbt_conf_t wbt_conf;

wbt_status wbt_conf_init() {
    wbt_rbtree_init(&wbt_config_rbtree);
    
    if( wbt_malloc(&wbt_config_file_path, 64) != WBT_OK ) {
        return WBT_ERROR;
    }
    
    if( wbt_conf_reload() != WBT_OK ) {
        return WBT_ERROR;
    }
    
    /* 初始化 wbt_conf */
    const char * value;
    wbt_mem_t * m_value;
    
    wbt_conf.listen_port = 80;
    if( ( value = wbt_conf_get("listen") ) != NULL ) {
        wbt_conf.listen_port = atoi(value);
        if( wbt_conf.listen_port < 0 || wbt_conf.listen_port > 65535  ) {
            wbt_log_add("listen port out of range ( expect 0 - 65535 )\n");
            return WBT_ERROR;
        }
    }
    
    wbt_conf.process = 1;
    if( ( value = wbt_conf_get("process") ) != NULL ) {
        wbt_conf.process = atoi(value);
        if( wbt_conf.process < 1 || wbt_conf.process > 128 ) {
            wbt_log_add("worker process number out of range ( expect 1 - 128 )\n");
            return WBT_ERROR;
        }
    }
    
    wbt_conf.run_mode = 0;
    if( ( value = wbt_conf_get("run_mode") ) != NULL ) {
        wbt_conf.run_mode = atoi(value);
        if( wbt_conf.run_mode < 0 || wbt_conf.run_mode > 1 ) {
            wbt_log_add("run_mode out of range ( expect 0 - 1 )\n");
            return WBT_ERROR;
        }
    }
    
    wbt_conf.secure = 0;
    if( ( m_value = wbt_conf_get_v("secure") ) != NULL ) {
        if( wbt_strcmp( (wbt_str_t *)m_value, &wbt_conf_option_on ) == 0 ) {
            wbt_conf.secure = 1;
        }
    }
    
    wbt_str_set_null(wbt_conf.secure_key);
    wbt_str_set_null(wbt_conf.secure_crt);
    if( wbt_conf.secure ) {
        if( ( m_value = wbt_conf_get_v("secure_key") ) != NULL ) {
            wbt_conf.secure_key.len = m_value->len;
            wbt_conf.secure_key.str = m_value->ptr;
        } else {
            wbt_log_add("secure_key option must be defined in config file\n");
            return WBT_ERROR;
        }
        
        if( ( m_value = wbt_conf_get_v("secure_crt") ) != NULL ) {
            wbt_conf.secure_crt.len = m_value->len;
            wbt_conf.secure_crt.str = m_value->ptr;
        } else {
            wbt_log_add("secure_crt option must be defined in config file\n");
            return WBT_ERROR;
        }
    }

    wbt_conf.sendfile = 0;
    if( !wbt_conf.secure ) {
        if( ( m_value = wbt_conf_get_v("sendfile") ) != NULL ) {
            if( wbt_strcmp( (wbt_str_t *)m_value, &wbt_conf_option_on ) == 0 ) {
                wbt_conf.sendfile = 1;
            }
        }
    }
    
    wbt_conf.keep_alive_timeout = 600000;
    if( ( value = wbt_conf_get("keep_alive_timeout") ) != NULL ) {
        wbt_conf.keep_alive_timeout = atoi(value);
    }
    
    wbt_conf.event_timeout = 150000;
    if( ( value = wbt_conf_get("event_timeout") ) != NULL ) {
        wbt_conf.event_timeout = atoi(value);
    }

    wbt_conf.max_open_files = 65535;
    if( ( value = wbt_conf_get("max_open_files") ) != NULL ) {
        wbt_conf.max_open_files = atoi(value);
    }
    
    wbt_conf.max_core_file_size = 0;
    if( ( value = wbt_conf_get("max_core_file_size") ) != NULL ) {
        wbt_conf.max_core_file_size = atoi(value);
    }
    
    wbt_conf.max_memory_usage = 0;
    if( ( value = wbt_conf_get("max_memory_usage") ) != NULL ) {
        wbt_conf.max_memory_usage = atoll(value);
    }

    wbt_str_set_null(wbt_conf.root); 
    if( ( m_value = wbt_conf_get_v("root") ) != NULL ) {
        wbt_conf.root.len = m_value->len;
        wbt_conf.root.str = m_value->ptr;
        // TODO 检查 root 是否存在
    } else {
        wbt_log_add("root option must be defined in config file\n");
        return WBT_ERROR;
    }
    
    wbt_str_set_null(wbt_conf.index);
    if( ( m_value = wbt_conf_get_v("default") ) != NULL ) {
        wbt_conf.index.len = m_value->len;
        wbt_conf.index.str = m_value->ptr;
    }

    wbt_str_set_null(wbt_conf.admin);
    if( ( m_value = wbt_conf_get_v("server_admin") ) != NULL ) {
        wbt_conf.admin.len = m_value->len;
        wbt_conf.admin.str = m_value->ptr;
    }

    wbt_str_set_null(wbt_conf.user);
    if( ( m_value = wbt_conf_get_v("user") ) != NULL ) {
        wbt_conf.user.len = m_value->len;
        wbt_conf.user.str = m_value->ptr;
    }

    return WBT_OK;
}

wbt_status wbt_conf_exit() {
    return WBT_OK;
}

wbt_status wbt_conf_set_file( const char * file ) {
    wbt_config_file_name.str = (char *)file; /* 这个类型转换只是为了避免编译器报
                                              * 错，不允许通过该指针进行写操作 */
    wbt_config_file_name.len = wbt_strlen(file);
    
    return WBT_OK;
}

static wbt_status wbt_conf_parse(wbt_mem_t * conf) {
    int status = 0;
    u_char *p, ch;
    
    wbt_conf_line = 1;
    wbt_conf_charactor = 0;
    
    wbt_str_t key, value;
    
    //wbt_log_debug("Parse config file: size %d", conf->len);
    
    int i = 0;
    for( i = 0 ; i < conf->len ; i ++ ) {
        p = (u_char *)conf->ptr;
        ch = *(p+i);

        wbt_conf_charactor ++;

        switch( status ) {
            case 0:
                if( ch == ' ' || ch == '\t' ) {
                    /* continue; */
                } else if( ch == '\n' ) {
                    wbt_conf_line ++;
                    wbt_conf_charactor = 0;
                } else if( ch == '#' ) {
                    status = 1;
                } else if( ( ch >= 'a' && ch <= 'z' ) || ch == '_' ) {
                    key.str = p + i;
                    status = 2;
                } else {
                    return WBT_ERROR;
                }
                break;
            case 1:
                if( ch == '\n' ) {
                    wbt_conf_line ++;
                    wbt_conf_charactor = 0;
                    status = 0;
                }
                break;
            case 2:
                if( ( ch >= 'a' && ch <= 'z' ) || ch == '_' ) {
                    /* continue; */
                } else if( ch == ' ' || ch == '\t' ) {
                    key.len = p + i - key.str;
                    status = 3;
                } else {
                    return WBT_ERROR;
                }
                break;
            case 3:
                if( ch == ' ' || ch == '\t' ) {
                    /* continue; */
                } else {
                    value.str = p + i;
                    status = 4;
                }
                break;
            case 4:
                if( ch == '\n' ) {
                    wbt_conf_line ++;
                    wbt_conf_charactor = 0;
                    value.len = p + i - value.str;
                    status = 0;
                    
                    /* 储存配置信息 */
                    wbt_rbtree_node_t *option =  wbt_rbtree_get(&wbt_config_rbtree, &key);
                    if( option == NULL ) {
                        /* 新的值 */
                        option = wbt_rbtree_insert(&wbt_config_rbtree, &key);
                    } else {
                        /* 已有的值 */
                        wbt_free(&option->value);
                    }
                    if( wbt_malloc(&option->value, value.len) != WBT_OK ) {
                        /* 这里返回 WBT_ERROR 会导致提示配置文件语法错误，
                         * 但事实上是由于内存不足导致的。在启动阶段出现这种情况的概率不大，所以就这样吧。 */
                        return WBT_ERROR;
                    }
                    /* 直接把 (wbt_str_t *) 转换为 (wbt_mem_t *) 会导致越界访问，
                     * 不过目前我认为这不是问题 */
                    wbt_memcpy(&option->value, (wbt_mem_t *)&value, value.len);
                }
                break;
            default:
                /* 不应当出现未知的状态 */
                return WBT_ERROR;
        }
    }
    
    return WBT_OK;
}

wbt_status wbt_conf_reload() {
    /* TODO 清理已保存的配置信息 */

    /* 尝试读取配置文件 */
    wbt_file_t wbt_config_file;
    wbt_config_file.fd = open(wbt_config_file_name.str, O_RDONLY);
    
    if( wbt_config_file.fd <= 0 ) {
        /* 找不到配置文件 */
        wbt_log_add("Can't find config file: %.*s\n", wbt_config_file_name.len, wbt_config_file_name.str);
        return WBT_ERROR;
    }

    int len = wbt_get_file_path_by_fd(wbt_config_file.fd, &wbt_config_file_path);
    if( len <= 0 ) {
        return WBT_ERROR;
    }
    
    struct stat statbuff;  
    if(stat(wbt_config_file_name.str, &statbuff) < 0){
        return WBT_ERROR;
    }
    wbt_config_file.size = statbuff.st_size;

    if( wbt_malloc(&wbt_config_file_content, wbt_config_file.size) != WBT_OK ) {
        return WBT_ERROR;
    }
    if( read( wbt_config_file.fd,
            wbt_config_file_content.ptr,
            wbt_config_file_content.len) != wbt_config_file_content.len ) {
        wbt_log_add("Read config file failed\n");
        return WBT_ERROR;
    }
    
    /* 关闭配置文件 */
    close(wbt_config_file.fd);

    /* 解析配置文件 */
    if( wbt_conf_parse(&wbt_config_file_content) == WBT_OK ) {
        wbt_free(&wbt_config_file_content);
        //wbt_rbtree_print(wbt_config_rbtree.root);

        char tmp[1024];
        snprintf(tmp, sizeof(tmp), "webit: master process (%.*s)", len, (char *)wbt_config_file_path.ptr );
        wbt_set_proc_title(tmp);

        return WBT_OK;
    } else {
        wbt_free(&wbt_config_file_content);
        wbt_log_add("Syntax error on config file: line %d, charactor %d\n", wbt_conf_line, wbt_conf_charactor);
        return WBT_ERROR;
    }
}

const char * wbt_conf_get( const char * name ) {
    wbt_mem_t * tmp = wbt_conf_get_v( name );
    if( tmp == NULL ) {
        return NULL;
    } else {
        return wbt_stdstr( (wbt_str_t *)tmp );
    }
}

wbt_mem_t * wbt_conf_get_v( const char * name ) {
    wbt_str_t config_name;
    config_name.str = (u_char *)name;
    config_name.len = strlen(name);
    wbt_rbtree_node_t * root = wbt_rbtree_get(&wbt_config_rbtree, &config_name);
    if( root == NULL ) {
        return NULL;
    } else {
        return &root->value;
    }
}
