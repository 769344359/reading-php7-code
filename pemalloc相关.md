# pemalloc 相关
在c标准库中有malloc 函数负责分配内存，而php 也使用c实现了一类malloc 函数
下面主要是分析`pemalloc` 函数
```
php-7.1.8-src\Zend\zend_alloc.h
#define pemalloc(size, persistent) ((persistent)?__zend_malloc(size):emalloc(size))
#define emalloc(size)						_emalloc((size) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
```
一般的内存persistent =0

所以是 调用的是函数`emalloc(size)`

将`emalloc`展开，得到

`_emalloc(size)`


然后我们看一下函数 `emalloc`

```
ZEND_API void* ZEND_FASTCALL _emalloc(size_t size ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{

#if ZEND_MM_CUSTOM
	if (UNEXPECTED(AG(mm_heap)->use_custom_heap)) {
		if (ZEND_DEBUG && AG(mm_heap)->use_custom_heap == ZEND_MM_CUSTOM_HEAP_DEBUG) {
			return AG(mm_heap)->custom_heap.debug._malloc(size ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
		} else {
			return AG(mm_heap)->custom_heap.std._malloc(size);
		}
	}
#endif
	return zend_mm_alloc_heap(AG(mm_heap), size ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);  // 核心函数
}
```
首先看宏`AG`,
```
#ifdef ZTS
static int alloc_globals_id;
# define AG(v) ZEND_TSRMG(alloc_globals_id, zend_alloc_globals *, v)
#else
# define AG(v) (alloc_globals.v)
static zend_alloc_globals alloc_globals;
#endif
```
> 内存分配
现在我们看一下函数`zend_mm_alloc_heap`
```
static zend_always_inline void *zend_mm_alloc_heap(zend_mm_heap *heap, size_t size ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{
	void *ptr;
	if (size <= ZEND_MM_MAX_SMALL_SIZE) {
		ptr = zend_mm_alloc_small(heap, size, ZEND_MM_SMALL_SIZE_TO_BIN(size) ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
		return ptr;
	} else if (size <= ZEND_MM_MAX_LARGE_SIZE) {
		ptr = zend_mm_alloc_large(heap, size ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
		return ptr;
	} else {
		return zend_mm_alloc_huge(heap, size ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
	}
}
```
其中宏`ZEND_MM_MAX_SAMLL_SIZE` 定义为
```
php-7.1.8-src\Zend\zend_alloc_sizes.h
#define ZEND_MM_CHUNK_SIZE (2 * 1024 * 1024)               /* 2 MB  */
#define ZEND_MM_PAGE_SIZE  (4 * 1024)                      /* 4 KB  */
#define ZEND_MM_FIRST_PAGE (1)
#define ZEND_MM_MAX_SMALL_SIZE      3072   // 一页为 4 kb 即小页为 3/4 页
#define ZEND_MM_MAX_LARGE_SIZE      (ZEND_MM_CHUNK_SIZE - (ZEND_MM_PAGE_SIZE * ZEND_MM_FIRST_PAGE))

// Zend/zend_alloc.c
#define ZEND_MM_SMALL_SIZE_TO_BIN(size)  zend_mm_small_size_to_bin(size)
```
展开后
```
ZEND_MM_MAX_SMALL_SIZE   3072   // 3/4页
ZEND_MM_MAX_LARGE_SIZE   2*1024*1024 - 4Kb   即  511 页

```
> 分配内存小于 3/4 page时

我们重新,当分配的内存小于3/4page 时候，调用函数`zend_mm_alloc_small`

函数`zend_mm_alloc_small`　定义：
```
static zend_always_inline void *zend_mm_alloc_small(zend_mm_heap *heap, size_t size, int bin_num ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{
#if ZEND_MM_STAT
	do {
		size_t size = heap->size + bin_data_size[bin_num];
		size_t peak = MAX(heap->peak, size);
		heap->size = size;
		heap->peak = peak;
	} while (0);
#endif

	if (EXPECTED(heap->free_slot[bin_num] != NULL)) {
		zend_mm_free_slot *p = heap->free_slot[bin_num];
		heap->free_slot[bin_num] = p->next_free_slot;
		return (void*)p;
	} else {
		return zend_mm_alloc_small_slow(heap, bin_num ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
	}
}
```




 
