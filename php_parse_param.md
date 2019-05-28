前几天遇到一个warning 是因为函数`array_slice` 传的参数是null 导致报错了
函数原型:`array_slice ( array $array , int $offset [, int $length = NULL [, bool $preserve_keys = FALSE ]] ) : array`
完整的代码:
```
/* {{{ proto array array_slice(array input, int offset [, int length [, bool preserve_keys]])
   Returns elements specified by offset and length */
PHP_FUNCTION(array_slice)
{

...
	ZEND_PARSE_PARAMETERS_START(2, 4)
		Z_PARAM_ARRAY(input)
		Z_PARAM_LONG(offset)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(z_length)
		Z_PARAM_BOOL(preserve_keys)
	ZEND_PARSE_PARAMETERS_END();
...
}
/* }}} */
```


核心的部分是一个宏`Z_PARAM_ARRAY`
```
Z_PARAM_ARRAY(input)
```
```
#define Z_PARAM_ARRAY_EX2(dest, check_null, deref, separate) \
		Z_PARAM_PROLOGUE(deref, separate); \
		if (UNEXPECTED(!zend_parse_arg_array(_arg, &dest, check_null, 0))) { \
			_expected_type = Z_EXPECTED_ARRAY; \
			error_code = ZPP_ERROR_WRONG_ARG; \
			break; \
		}

#define Z_PARAM_ARRAY_EX(dest, check_null, separate) \
	Z_PARAM_ARRAY_EX2(dest, check_null, separate, separate)

#define Z_PARAM_ARRAY(dest) \
	Z_PARAM_ARRAY_EX(dest, 0, 0)
```
我们把宏`Z_PARAM_ARRAY(input)` 展开
```
Z_PARAM_PROLOGUE(0, 0); \
		if (UNEXPECTED(!zend_parse_arg_array(0, &dest, 0, 0))) { \
			_expected_type = Z_EXPECTED_ARRAY; \
			error_code = ZPP_ERROR_WRONG_ARG; \
			break; \
		}
```

堆栈:
```
(gdb) bt
#0  zend_parse_arg_array (arg=0x7ffff0c200d0, dest=0x7fffffffa938, check_null=0, or_object=0)
    at /data/soft/php-7.3.1/Zend/zend_API.h:1244
#1  0x00000000008a899f in zif_array_slice (execute_data=0x7ffff0c20080, return_value=0x7fffffffaa50)
    at /data/soft/php-7.3.1/ext/standard/array.c:3477
#2  0x0000000000afb919 in ZEND_DO_ICALL_SPEC_RETVAL_UNUSED_HANDLER ()
    at /data/soft/php-7.3.1/Zend/zend_vm_execute.h:645
#3  0x0000000000b63b0f in execute_ex (ex=0x7ffff0c20030)
    at /data/soft/php-7.3.1/Zend/zend_vm_execute.h:55414
#4  0x0000000000b69161 in zend_execute (op_array=0x7ffff0c7a400, return_value=0x0)
    at /data/soft/php-7.3.1/Zend/zend_vm_execute.h:60834
#5  0x0000000000a98fa5 in zend_execute_scripts (type=8, retval=0x0, file_count=3)
    at /data/soft/php-7.3.1/Zend/zend.c:1568
#6  0x0000000000a093dd in php_execute_script (primary_file=0x7fffffffe130)
    at /data/soft/php-7.3.1/main/main.c:2630
#7  0x0000000000b6bbca in do_cli (argc=2, argv=0x1754d00) at /data/soft/php-7.3.1/sapi/cli/php_cli.c:997
#8  0x0000000000b6cb36 in main (argc=2, argv=0x1754d00) at /data/soft/php-7.3.1/sapi/cli/php_cli.c:1389
```
