
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
初始化数组


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
	ht->nTableSize = zend_hash_check_size(nSize);
}
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
		CHECK_INIT(ht, 0);
		goto add_to_hash;
	}
```

