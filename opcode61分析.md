```
// /home/dinosaur/Downloads/php-7.2.2/Zend/zend_types.h
#define Z_CACHE_SLOT(zval)			(zval).u2.cache_slot
#define Z_CACHE_SLOT_P(zval_p)		Z_CACHE_SLOT(*(zval_p))

(gdb) p  (char *)fbc->common.function_name.val 
$8 = 0x7fffef602b58 "sayHi"

```
堆栈
```
(gdb) bt
#0  0x00000000009b6105 in ZEND_INIT_FCALL_SPEC_CONST_HANDLER () at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:2272
#1  0x0000000000a37ee4 in execute_ex (ex=0x7fffef61e030) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:59845
#2  0x0000000000a3d0ab in zend_execute (op_array=0x7fffef683300, return_value=0x0) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:63760
#3  0x000000000094cd22 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend.c:1496
#4  0x00000000008b0b4a in php_execute_script (primary_file=0x7fffffffca10) at /home/dinosaur/Downloads/php-7.2.2/main/main.c:2590
#5  0x0000000000a3fd23 in do_cli (argc=2, argv=0x1441f40) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1011
#6  0x0000000000a40ee0 in main (argc=2, argv=0x1441f40) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1404

```


ZEND_FREE_SPEC_TMPVAR_LABEL
ZEND_FE_FREE_SPEC_TMPVAR_LABEL

