
	/* $Id: fpm_shm.c,v 1.3 2008/05/24 17:38:47 anight Exp $ */
	/* (c) 2007,2008 Andrei Nigmatulin, Jerome Loyet */

#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include "fpm_shm.h"
#include "zlog.h"


/* MAP_ANON is deprecated, but not in macosx */
#if defined(MAP_ANON) && !defined(MAP_ANONYMOUS)
#define MAP_ANONYMOUS MAP_ANON
#endif

static size_t fpm_shm_size = 0;

void *fpm_shm_alloc(size_t size) /* {{{ */
{
	void *mem;

	mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);    //核心代码mmap 分配共享内存

#ifdef MAP_FAILED
	if (mem == MAP_FAILED) {
		zlog(ZLOG_SYSERROR, "unable to allocate %zu bytes in shared memory: %s", size, strerror(errno));
		return NULL;
	}
#endif

	if (!mem) {
		zlog(ZLOG_SYSERROR, "unable to allocate %zu bytes in shared memory", size);
		return NULL;
	}

	fpm_shm_size += size;          // 内部全部变量添加
	return mem;
}
/* }}} */

int fpm_shm_free(void *mem, size_t size) /* {{{ */      // 释放匿名内存
{
	if (!mem) {
		zlog(ZLOG_ERROR, "mem is NULL");
		return 0;
	}

	if (munmap(mem, size) == -1) {
		zlog(ZLOG_SYSERROR, "Unable to free shm");
		return 0;
	}

	if (fpm_shm_size - size > 0) {
		fpm_shm_size -= size;
	} else {
		fpm_shm_size = 0;
	}

	return 1;
}
/* }}} */

size_t fpm_shm_get_size_allocated() /* {{{*/           //获得分配的总的匿名内存的大小
{
	return fpm_shm_size;
}
/* }}} */
