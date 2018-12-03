```
// /home/dinosaur/Downloads/php-7.2.2/Zend/zend_types.h
#define Z_CACHE_SLOT(zval)			(zval).u2.cache_slot
#define Z_CACHE_SLOT_P(zval_p)		Z_CACHE_SLOT(*(zval_p))

(gdb) p  (char *)fbc->common.function_name.val 
$8 = 0x7fffef602b58 "sayHi"

```
