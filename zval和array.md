
## hashtable 
hashtable 包括 ：
- 一个 `gc` 结构 ,是帮助垃圾回收而使用的
- 一个联合体
- 掩码
- 桶指针 `arData`  储存内容的主要的指针
- 析构函数`pDestructor`
- 桶的数目 `nTableSize`

...
```
// src\Zend\zend_types.h
typedef struct _zend_array HashTable;
struct _zend_array {
	zend_refcounted_h gc;
	union {
		struct {
			ZEND_ENDIAN_LOHI_4(
				zend_uchar    flags,
				zend_uchar    nApplyCount,
				zend_uchar    nIteratorsCount,
				zend_uchar    consistency)
		} v;
		uint32_t flags;
	} u;
	uint32_t          nTableMask
	Bucket           *arData;
	uint32_t          nNumUsed;
	uint32_t          nNumOfElements;
	uint32_t          nTableSize;
	uint32_t          nInternalPointer;
	zend_long         nNextFreeElement;
	dtor_func_t       pDestructor;
};
```
然后是Bucket 结构
```
typedef struct _Bucket {
	zval              val;
	zend_ulong        h;                /* hash value (or numeric index)   */
	zend_string      *key;              /* string key or NULL for numerics */
} Bucket;
```
##zval 
###以下我介绍一下zval
- 首先看一下zval的定义

```
typedef struct _zval_struct     zval;

struct _zval_struct {
	zend_value        value;			/* value */
	union {
		struct {
			ZEND_ENDIAN_LOHI_4(
				zend_uchar    type,			/* active type */
				zend_uchar    type_flags,
				zend_uchar    const_flags,
				zend_uchar    reserved)	    /* call info for EX(This) */
		} v;
		uint32_t type_info;
	} u1;
	union {
		uint32_t     next;                 /* hash collision chain */
		uint32_t     cache_slot;           /* literal cache slot */
		uint32_t     lineno;               /* line number (for ast nodes) */
		uint32_t     num_args;             /* arguments number for EX(This) */
		uint32_t     fe_pos;               /* foreach position */
		uint32_t     fe_iter_idx;          /* foreach iterator index */
		uint32_t     access_flags;         /* class constant access flags */
		uint32_t     property_guard;       /* single property guard */
		uint32_t     extra;                /* not further specified */
	} u2;
};
```
继续上面的结构 `_zval_struct` , 我的代码的版本是`php 7.1.8`， 然后看到  ·`_zval_struct` 由以下几个部分组成
-  `zend_val `
- 联合体 `u1`
- 联合体`u2`
u2 有一个next  的值可以实现关联数组相关的操作

>zend_value; 然后到了zend_value
> zend_value  是一个联合体,7.18 继续对zend_value 进行优化,缩小了zend_value的大小,可以节省非常多的内存
```
typedef union _zend_value {
	zend_long         lval;				/* long value */
	double            dval;				/* double value */
	zend_refcounted  *counted;
	zend_string      *str;
	zend_array       *arr;
	zend_object      *obj;
	zend_resource    *res;
	zend_reference   *ref;
	zend_ast_ref     *ast;
	zval             *zv;
	void             *ptr;
	zend_class_entry *ce;
	zend_function    *func;
	struct {
		uint32_t w1;
		uint32_t w2;
	} ww;
} zend_value;
```
## zend_string
```
// dinosaur\php-7.1.8-src\main\php.h
# define XtOffsetOf(s_type, field) offsetof(s_type, field)
// dinosaur\php-7.1.8-src\Zend\zend_string.h
#define _ZSTR_HEADER_SIZE XtOffsetOf(zend_string, val)
#define _ZSTR_STRUCT_SIZE(len) (_ZSTR_HEADER_SIZE + len + 1)

struct _zend_string {
	zend_refcounted_h gc;
	zend_ulong        h;                /* hash value */
	size_t            len;
	char              val[1];
};
typedef struct _zend_string     zend_string;

static zend_always_inline zend_string *zend_string_alloc(size_t len, int persistent){
...
	zend_string *ret = (zend_string *)pemalloc(ZEND_MM_ALIGNED_SIZE(_ZSTR_STRUCT_SIZE(len)), persistent);
...
}
```
`zend_struct` 结构包含以下几个结构
- `gc` 用于引用计数
- `h`  计算hash值
- `size` 要储存的字符串的长度
- `var` 占位
`zend_string` 的 结构和 redis 储存字符串的结构类似,都会有一个记录字符串长度的变量

***

## 第一次插入数组


```
#define ZEND_INIT_SYMTABLE(ht)								\
	ZEND_INIT_SYMTABLE_EX(ht, 8, 0)

#define ZEND_INIT_SYMTABLE_EX(ht, n, persistent)			\
	zend_hash_init(ht, n, NULL, ZVAL_PTR_DTOR, persistent)
```
- `ZEND_INIT_SYMTABLE` 展开

`ZEND_INIT_SYMTABLE(ht)`   展开为 `zend_hash_init(ht,8,NULL,ZVAL_PTR_DTOR,0)`


```
ZEND_API void ZEND_FASTCALL _zend_hash_init(HashTable *ht, uint32_t nSize, dtor_func_t pDestructor, zend_bool persistent ZEND_FILE_LINE_DC)
{
	GC_REFCOUNT(ht) = 1;    // gc  添加计数器
	GC_TYPE_INFO(ht) = IS_ARRAY;  // 添加类型
	ht->u.flags = (persistent ? HASH_FLAG_PERSISTENT : 0) | HASH_FLAG_APPLY_PROTECTION | HASH_FLAG_STATIC_KEYS;
	ht->nTableMask = HT_MIN_MASK;   // 设置默认的掩码
	HT_SET_DATA_ADDR(ht, &uninitialized_bucket);   // 
	ht->nNumUsed = 0;  
	ht->nNumOfElements = 0;
	ht->nInternalPointer = HT_INVALID_IDX;
	ht->nNextFreeElement = 0;
	ht->pDestructor = pDestructor;
	ht->nTableSize = zend_hash_check_size(nSize);  //nSize =8
}
```


首先打印一下堆栈
```
(gdb) bt
#0  zend_hash_real_init_ex (ht=0x189e760, packed=0) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_hash.c:132
#1  0x0000000000bd704d in zend_hash_check_init (ht=0x189e760, packed=0) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_hash.c:163
#2  0x0000000000bd8d0d in _zend_hash_add_or_update_i (ht=0x189e760, key=0x189e7e0, pData=0x7fffffffdac0, flag=2, __zend_filename=0x143d620 "/home/dinosaur/Downloads/php-7.2.2/Zend/zend_hash.h", 
    __zend_lineno=629) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_hash.c:552
#3  0x0000000000bd934e in _zend_hash_str_add (ht=0x189e760, str=0x143de48 "application/x-www-form-urlencoded", len=33, pData=0x7fffffffdac0, 
    __zend_filename=0x143d620 "/home/dinosaur/Downloads/php-7.2.2/Zend/zend_hash.h", __zend_lineno=629) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_hash.c:668
#4  0x0000000000b05497 in  (ht=0x189e760, str=0x143de48 "application/x-www-form-urlencoded", len=33, pData=0x18324a0 <php_post_entries>, size=32)
    at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_hash.h:629
```
```
/home/dinosaur/reading-php7-code/Zend/zend_hash.h
#define HASH_FLAG_INITIALIZED      (1<<3)
```
然后我们主要看函数　`zend_hash_add_or_update_i`

```
static zend_always_inline zval *_zend_hash_add_or_update_i(HashTable *ht, zend_string *key, zval *pData, uint32_t flag ZEND_FILE_LINE_DC)
{
	zend_ulong h;
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p;
	...
	if (UNEXPECTED(!(ht->u.flags & HASH_FLAG_INITIALIZED))) {
		CHECK_INIT(ht, 0);
		goto add_to_hash;
	} else if (ht->u.flags & HASH_FLAG_PACKED) {
		zend_hash_packed_to_hash(ht);
	} 
	...
add_to_hash:
	idx = ht->nNumUsed++;
	ht->nNumOfElements++;
	if (ht->nInternalPointer == HT_INVALID_IDX) {
		ht->nInternalPointer = idx;
	}
	zend_hash_iterators_update(ht, HT_INVALID_IDX, idx);
	p = ht->arData + idx;
	p->key = key;
	if (!ZSTR_IS_INTERNED(key)) {
		zend_string_addref(key);
		ht->u.flags &= ~HASH_FLAG_STATIC_KEYS;
		zend_string_hash_val(key);
	}
	p->h = h = ZSTR_H(key);
	ZVAL_COPY_VALUE(&p->val, pData);
	nIndex = h | ht->nTableMask;
	Z_NEXT(p->val) = HT_HASH(ht, nIndex);
	HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(idx);

	return &p->val;
}
```
首先gdb 打印出标志位
```
(gdb) p/t  ht->u.flags
$7 = 10001
```

首先会进入到第一个分支
```
	if (UNEXPECTED(!(ht->u.flags & HASH_FLAG_INITIALIZED))) {
		CHECK_INIT(ht, 0); // 真正初始化
		goto add_to_hash;
	}
```
现在我们来展开宏`CHECK_INIT`
```
#define CHECK_INIT(ht, packed) \
	zend_hash_check_init(ht, packed)
```
所以最后调用的是
```
zend_hash_check_init(ht, packed)
```
而真正初始化的则是第一次插入的时候，其中调用的是函数`zend_hash_check_init`
```
static zend_always_inline void zend_hash_check_init(HashTable *ht, int packed)
{
	HT_ASSERT(GC_REFCOUNT(ht) == 1);
	if (UNEXPECTED(!((ht)->u.flags & HASH_FLAG_INITIALIZED))) {
		zend_hash_real_init_ex(ht, packed);
	}
}
```
然后我们继续看函数`zend_hash_real_init_ex`
其中
```
(gdb) p ht->nTableSize
$10 = 8
```
```
static zend_always_inline void zend_hash_real_init_ex(HashTable *ht, int packed)
{
		...
		(ht)->nTableMask = -(ht)->nTableSize; // nTableSize = 8 ,所以nTableMask = -8
		HT_SET_DATA_ADDR(ht, pemalloc(HT_SIZE(ht), (ht)->u.flags & HASH_FLAG_PERSISTENT));  //（１） 分配内存
		(ht)->u.flags |= HASH_FLAG_INITIALIZED;
		if (EXPECTED(ht->nTableMask == (uint32_t)-8)) {
			Bucket *arData = ht->arData;

			HT_HASH_EX(arData, -8) = -1;
			HT_HASH_EX(arData, -7) = -1;
			HT_HASH_EX(arData, -6) = -1;
			HT_HASH_EX(arData, -5) = -1;
			HT_HASH_EX(arData, -4) = -1;
			HT_HASH_EX(arData, -3) = -1;
			HT_HASH_EX(arData, -2) = -1;
			HT_HASH_EX(arData, -1) = -1;
		}
		...
	
}
```
- 分配内存

下面主要是讨论内存分配
```
HT_SET_DATA_ADDR(ht, pemalloc(HT_SIZE(ht), (ht)->u.flags & HASH_FLAG_PERSISTENT));
```
首先展开宏定义　`HT_SIZE(ht)`
```
#define HT_HASH_EX(data, idx) \
	((uint32_t*)(data))[(int32_t)(idx)]
#define HT_HASH(ht, idx) \
	HT_HASH_EX((ht)->arData, idx)

#define HT_HASH_SIZE(nTableMask) \
	(((size_t)(uint32_t)-(int32_t)(nTableMask)) * sizeof(uint32_t))
#define HT_DATA_SIZE(nTableSize) \
	((size_t)(nTableSize) * sizeof(Bucket))
#define HT_SIZE_EX(nTableSize, nTableMask) \
	(HT_DATA_SIZE((nTableSize)) + HT_HASH_SIZE((nTableMask)))
#define HT_SIZE(ht) \
	HT_SIZE_EX((ht)->nTableSize, (ht)->nTableMask)
```
由上面的宏定义可以一步步展开
- `HT_SIZE(ht)`
- `HT_SIZE_EX((ht)->nTableSize, (ht)->nTableMask)`   // 展开HT_SIZE_EX
- `(size_t)(nTableSize) * sizeof(Bucket) + (((size_t)(uint32_t)-(int32_t)(nTableMask)) * sizeof(uint32_t))`

这个表达式可以几乎等价于
 `(nTableSize) * sizeof(Bucket)　+(-nTableMask)*(uint32_t)`
 然后 我们主要看一下宏`HT_SET_DATA_ADDR`的展开。
 宏定义如下
 ```
 #define HT_SET_DATA_ADDR(ht, ptr) do { \
		(ht)->arData = (Bucket*)(((char*)(ptr)) + HT_HASH_SIZE((ht)->nTableMask)); \
	} while (0)
 ```
 容易知道展开后等价于`arData` 偏移到第一个`Bucket`
 
 ## 插入数据
 分配内存和将arData指向第一个Bucket 之后，继续的操作是插入一个Bucket 结构
 ```
 static zend_always_inline zval *_zend_hash_add_or_update_i(HashTable *ht, zend_string *key, zval *pData, uint32_t flag ZEND_FILE_LINE_DC)
{	
	...
	if (UNEXPECTED(!(ht->u.flags & HASH_FLAG_INITIALIZED))) {
		CHECK_INIT(ht, 0);   // 初始化内存
		goto add_to_hash;  // 添加Bucket
	...
add_to_hash:
	idx = ht->nNumUsed++;   
	ht->nNumOfElements++;
	if (ht->nInternalPointer == HT_INVALID_IDX) {
		ht->nInternalPointer = idx;
	}
	zend_hash_iterators_update(ht, HT_INVALID_IDX, idx);
	p = ht->arData + idx; // 下一条Bucket
	p->key = key;   // zend_string    
	if (!ZSTR_IS_INTERNED(key)) {
		zend_string_addref(key);
		ht->u.flags &= ~HASH_FLAG_STATIC_KEYS;
		zend_string_hash_val(key);
	}
	p->h = h = ZSTR_H(key); // 获取hash值 
	ZVAL_COPY_VALUE(&p->val, pData);
	nIndex = h | ht->nTableMask;  // hash取余 
	Z_NEXT(p->val) = HT_HASH(ht, nIndex);  // 将前一个的Bucket 的偏移值记录下来 到next 里面
	HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(idx);  // 在左边 记录下 Bucket 的 arData 偏移量

	return &p->val;

 ```

