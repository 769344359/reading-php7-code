
## hashtable 
hashtable 包括 ：
- 一个 `gc` 结构 ,是帮助垃圾回收而使用的
- 一个联合体
- 掩码
- 桶指针 `arData`
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
