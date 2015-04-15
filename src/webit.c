/* 
 * File:   webit.c
 * Author: Fcten
 *
 * Created on 2014年8月20日, 下午2:10
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <grp.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include "webit.h"
#include "common/wbt_string.h"
#include "common/wbt_memory.h"
#include "common/wbt_log.h"
#include "common/wbt_module.h"
#include "common/wbt_rbtree.h"
#include "common/wbt_config.h"
#include "os/linux/wbt_sigsegv.h"

extern char **environ;
char *last;

int wbt_argc;
char** wbt_argv;

int wating_to_exit = 0;

void initProcTitle() {
    size_t size = 0;
    int i;
    for (i = 0; environ[i]; ++i) {
        size += strlen(environ[i])+1; 
    }   
 
    char *raw = malloc(size*sizeof(char));
    for (i = 0; environ[i]; ++i) {
        memcpy(raw, environ[i], strlen(environ[i]) + 1); 
        environ[i] = raw;
        raw += strlen(environ[i]) + 1;
    }   
 
    last = wbt_argv[0];
    for (i = 0; i < wbt_argc; ++i) {
        last += strlen(wbt_argv[i]) + 1;   
    }   
    for (i = 0; environ[i]; ++i) {
        last += strlen(environ[i]) + 1;
    }
}
 
void setProcTitle(const char *title) {
    wbt_argv[1] = 0;
    char *p = wbt_argv[0];
    memset(p, 0x00, last - p); 
    strncpy(p, title, last - p); 
}

void wbt_signal(int signo, siginfo_t *info, void *context) {
    wbt_log_debug("received singal: %d", signo);

    pid_t pid;
    int   stat;

    switch( signo ) {
        case SIGCHLD:
            /* 仅父进程：子进程退出，从列表中移除
             * 如果同时有大量进程退出，SIGCHLD 信号可能会丢失，
             * 所以不能使只根据 info 中的 pid 判断
             */
            while((pid = waitpid(-1, &stat, WNOHANG)) > 0){
               wbt_proc_remove(pid);
            }

            break;
        case SIGTERM:
            /* 父进程：停止所有子进程并退出 */
            /* 子进程：停止事件循环并退出 */
            wating_to_exit = 1;
            break;
            /* 仅父进程：reload 信号 */
        case SIGINT:
            /* 仅调试模式，退出 */
            wating_to_exit = 1;
            break;
    }
}

void wbt_worker_process() {
    /* 设置进程标题 */
    if( !wbt_conf.run_mode ) {
        //setProcTitle("Webit: worker process");
    }

    /* 设置需要监听的信号(后台模式) */
    struct sigaction act;
    sigset_t set;

    act.sa_sigaction = wbt_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGTERM);

    if (sigprocmask(SIG_UNBLOCK, &set, NULL) == -1) {
        wbt_log_add("sigprocmask() failed\n");
    }

    sigaction(SIGINT, &act, NULL); /* 退出信号 */
    sigaction(SIGTERM, &act, NULL); /* 退出信号 */
    
    wbt_log_add("Webit startup (pid: %d)\n", getpid());

    /* 降低 worker 进程的权限 
    const char * user = wbt_conf_get("user");
    if ( user != NULL && geteuid() == 0 ) {
        if (setgid(33) == -1) {
            wbt_log_debug("setgid(%d) failed", 33)
            return;
        }

        if (initgroups("www-data", 33) == -1) {
            wbt_log_debug("initgroups(www-data, %d) failed", 33);
            return;
        }

        if (setuid(33) == -1) {
            wbt_log_debug("setuid(%d) failed", 33);
            return;
        }
    } */

    /* 进入事件循环 */
    wbt_event_dispatch();
    
    wbt_exit(0);
}

void wbt_master_process() {
    /* 设置进程标题 */
    if( !wbt_conf.run_mode ) {
        //setProcTitle("Webit: master process (default)");
    }

    /* 设置需要监听的信号(后台模式) */
    struct sigaction act;
    sigset_t set;

    act.sa_sigaction = wbt_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGTERM);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        wbt_log_add("sigprocmask() failed\n");
    }

    sigemptyset(&set);
    
    sigaction(SIGCHLD, &act, NULL); /* 子进程退出 */
    sigaction(SIGTERM, &act, NULL); /* 命令 Webit 退出 */
    /* 自定义的 reload 信号 */

    time_t prev_time;
    int count = 0;
    while(!wating_to_exit) {
        /* 防止 worker process 出错导致 fork 炸弹 */
        wbt_time_update();
        if( cur_mtime - prev_time <= 1000 ) {
            if( ( count ++ ) > 9 ) {
                wbt_log_add("try to fork child too fast\n");
                /* 触发限制后，每 5s 尝试一次 
                 * 由于阻塞了相关信号，这会导致信号处理出现延迟
                 */
                sleep(5);
            }
        } else {
            count = 0;
        }
        prev_time = cur_mtime;
        
        /* 创建子进程直至指定数量 */
        wbt_proc_create(wbt_worker_process, wbt_conf.process);
        
        sigsuspend(&set);
    }

    /* 结束所有子进程 */
    pid_t child;
    while( ( child = wbt_proc_pop() ) != 0 ) {
        kill( child, SIGTERM );
    }
    
    wbt_exit(0);
}

void wbt_exit(int exit_code) {
    wating_to_exit = 1;

    /* 卸载所有模块 */
    wbt_module_exit();

    wbt_log_add("Webit exit (pid: %d)\n", getpid());
    
    exit(exit_code);
}

int main(int argc, char** argv) {
    /* 保存传入参数 */
    wbt_argc = argc;
    wbt_argv = argv;
    
    setup_sigsegv();

    /* 初始化所有组件 */
    if( wbt_module_init() != WBT_OK ) {
        return 1;
    }

    /* 设置程序允许打开的最大文件句柄数 */
    struct rlimit rlim;
    rlim.rlim_cur = 65535;
    rlim.rlim_max = 65535;
    setrlimit(RLIMIT_NOFILE, &rlim);

    /* 接下来的 chroot 会导致程序无法访问 /etc/timezone
     * TODO 读取 /etc/timezone 的内容并保存
     */
    if( setenv("TZ", "Asia/Shanghai", 1) != 0 ) {
        perror("setenv");
        return;
    }
    tzset();

    initProcTitle();

    if( !wbt_conf.run_mode ) {
        /* 转入后台运行 */
        if( daemon(1,0) < 0 ) {
            perror("error daemon");  
            return 1;
        }
    }

    /* 限制可以访问的目录
     * 这个操作会导致 daemon() 不能正常运行
     */
    const char * wwwroot = wbt_stdstr(&wbt_conf.root);
    if( chroot( wwwroot ) != 0 ) {
        wbt_log_add("%s not exists.\n", wwwroot);
        return;
    } else {
        wbt_log_add("Root path: %s\n", wwwroot);
    }

    if( !wbt_conf.run_mode ) {
        wbt_master_process();
    } else {
        wbt_worker_process();
    }
    
    return 0;
}

