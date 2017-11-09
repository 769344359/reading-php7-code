
	/* $Id: fpm_worker_pool.h,v 1.13 2008/08/26 15:09:15 anight Exp $ */
	/* (c) 2007,2008 Andrei Nigmatulin */

#ifndef FPM_WORKER_POOL_H
#define FPM_WORKER_POOL_H 1

#include "fpm_conf.h"
#include "fpm_shm.h"    // 匿名内存头

struct fpm_worker_pool_s;
struct fpm_child_s;
struct fpm_child_stat_s;
struct fpm_shm_s;

enum fpm_address_domain {
	FPM_AF_UNIX = 1,               // 地址类型  unxi socket 或者是   tcp / ip 协议族
	FPM_AF_INET = 2
};
///    线程池结构
struct fpm_worker_pool_s {
	struct fpm_worker_pool_s *next;     // 链表next
	struct fpm_worker_pool_config_s *config;     // 大部分配置 估计是共享的  也可能不是共享的   
	char *user, *home;									/* for setting env USER and HOME */
	enum fpm_address_domain listen_address_domain;    // 监听的类型
	int listening_socket;								// socket fd 
	int set_uid, set_gid;					// uid 和 组id			/* config uid and gid */
	int socket_uid, socket_gid, socket_mode;    //  不知道什么

	/* runtime */
	struct fpm_child_s *children;
	int running_children;
	int idle_spawn_rate;
	int warn_max_children;
#if 0
	int warn_lq;
#endif
	struct fpm_scoreboard_s *scoreboard;
	int log_fd;
	char **limit_extensions;

	/* for ondemand PM */
	struct fpm_event_s *ondemand_event;
	int socket_event_set;

#ifdef HAVE_FPM_ACL
	void *socket_acl;
#endif
};

struct fpm_worker_pool_s *fpm_worker_pool_alloc();
void fpm_worker_pool_free(struct fpm_worker_pool_s *wp);
int fpm_worker_pool_init_main();

extern struct fpm_worker_pool_s *fpm_worker_all_pools;

#endif

