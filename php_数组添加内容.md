```c
ZEND_API int add_next_index_zval(zval *arg, zval *value) /* {{{ */
{
	return zend_hash_next_index_insert(Z_ARRVAL_P(arg), value) ? SUCCESS : FAILURE;
}
```

> 宏 `Z_ARRVAL_P`
首先看宏展开
```c
#define Z_ARR(zval)		(zval).value.arr
#define Z_ARRVAL(zval)		 Z_ARR(zval)
#define Z_ARRVAL_P(zval_p)	 Z_ARRVAL(*(zval_p))
```
即`Z_ARRVAL_P(arg)`展开后是
```c
(*arg).value.arr
```

>  zend_hash_next_index_insert 
```
// php-7.1.8-src\Zend\zend_hash.h
#define zend_hash_next_index_insert(ht, pData) \
		_zend_hash_next_index_insert(ht, pData ZEND_FILE_LINE_CC)
```

`zend_hash_next_index_insert`  展开后是 `_zend_hash_next_index_insert`

然后 `_zend_hash_next_index_insert` 其实只是对`_zend_hash_index_add_or_update_i`包裹了一层

```c
ZEND_API zval* ZEND_FASTCALL _zend_hash_next_index_insert(HashTable *ht, zval *pData ZEND_FILE_LINE_DC)
{
	return _zend_hash_index_add_or_update_i(ht, ht->nNextFreeElement, pData, HASH_ADD | HASH_ADD_NEXT ZEND_FILE_LINE_RELAY_CC);
}
```
