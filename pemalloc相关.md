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

```
展开后
```
ZEND_MM_MAX_SMALL_SIZE   3072   // 3/4页
ZEND_MM_MAX_LARGE_SIZE   2*1024*1024 - 4Kb   即  511 页

```
> 分配内存小于 3/4 page时

我们重新,当分配的内存小于3/4page 时候，调用函数`zend_mm_alloc_small`

> 调用链：
```
|---zend_mm_alloc_small
   |---zend_mm_alloc_heap
      |---zend_mm_small_size_to_bin
         |---zend_mm_small_size_to_bit
```
当时小页的时候会分成 **30**个等级分配内存,
分别是 8 16 24  32 40 48 56 64 80 96 112 128 160 192 224 256 320 384 ... 一直到3072
```
/* num, size, count, pages */
#define ZEND_MM_BINS_INFO(_, x, y) \
	_( 0,    8,  512, 1, x, y) \
	_( 1,   16,  256, 1, x, y) \
	_( 2,   24,  170, 1, x, y) \
	_( 3,   32,  128, 1, x, y) \
	_( 4,   40,  102, 1, x, y) \
	_( 5,   48,   85, 1, x, y) \
	_( 6,   56,   73, 1, x, y) \
	_( 7,   64,   64, 1, x, y) \
	_( 8,   80,   51, 1, x, y) \
	_( 9,   96,   42, 1, x, y) \
	_(10,  112,   36, 1, x, y) \
	_(11,  128,   32, 1, x, y) \
	_(12,  160,   25, 1, x, y) \
	_(13,  192,   21, 1, x, y) \
	_(14,  224,   18, 1, x, y) \
	_(15,  256,   16, 1, x, y) \
	_(16,  320,   64, 5, x, y) \
	_(17,  384,   32, 3, x, y) \
	_(18,  448,    9, 1, x, y) \
	_(19,  512,    8, 1, x, y) \
	_(20,  640,   32, 5, x, y) \
	_(21,  768,   16, 3, x, y) \
	_(22,  896,    9, 2, x, y) \
	_(23, 1024,    8, 2, x, y) \
	_(24, 1280,   16, 5, x, y) \
	_(25, 1536,    8, 3, x, y) \
	_(26, 1792,   16, 7, x, y) \
	_(27, 2048,    8, 4, x, y) \
	_(28, 2560,    8, 5, x, y) \
	_(29, 3072,    4, 3, x, y)
```


 
