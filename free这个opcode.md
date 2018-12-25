堆栈
```
#0  ZEND_FREE_SPEC_TMPVAR_HANDLER () at /home/vagrant/php-7.2.13/Zend/zend_vm_execute.h:48496
#1  0x0000000000a097ad in execute_ex (ex=0x7ffff441e030) at /home/vagrant/php-7.2.13/Zend/zend_vm_execute.h:63137
#2  0x0000000000a0a534 in zend_execute (op_array=0x7ffff447e300, return_value=0x0) at /home/vagrant/php-7.2.13/Zend/zend_vm_execute.h:63776
#3  0x0000000000919d96 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/vagrant/php-7.2.13/Zend/zend.c:1502
#4  0x000000000087d9d9 in php_execute_script (primary_file=0x7fffffffd240) at /home/vagrant/php-7.2.13/main/main.c:2590
#5  0x0000000000a0d115 in do_cli (argc=2, argv=0x13f96f0) at /home/vagrant/php-7.2.13/sapi/cli/php_cli.c:1011
#6  0x0000000000a0e2d2 in main (argc=2, argv=0x13f96f0) at /home/vagrant/php-7.2.13/sapi/cli/php_cli.c:1403
```
首先定义
```
#define EX_VAR(n)				ZEND_CALL_VAR(execute_data, n)
```
这次要说明的是free 这个`opcode`
```
static ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL ZEND_FREE_SPEC_TMPVAR_HANDLER(ZEND_OPCODE_HANDLER_ARGS)
{
	USE_OPLINE

	SAVE_OPLINE();
	zval_ptr_dtor_nogc(EX_VAR(opline->op1.var));
	ZEND_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}
```

opline 的类型:
```
struct _zend_execute_data {
	const zend_op       *opline;           /* executed opline                */
	zend_execute_data   *call;             /* current call                   */
	zval                *return_value;
	zend_function       *func;             /* executed function              */
	zval                 This;             /* this + call_info + num_args    */
	zend_execute_data   *prev_execute_data;
	zend_array          *symbol_table;
#if ZEND_EX_USE_RUN_TIME_CACHE
	void               **run_time_cache;   /* cache op_array->run_time_cache */
#endif
};

```
opline 的类型是 zend_op  
zend_op 的结构:
```
struct _zend_op {
	const void *handler;
	znode_op op1;
	znode_op op2;
	znode_op result;
	uint32_t extended_value;
	uint32_t lineno;
	zend_uchar opcode;
	zend_uchar op1_type;
	zend_uchar op2_type;
	zend_uchar result_type;
};
```
zend_op 和znode_op 长得非常像,不要看错,下面看znode_op 结构

```
typedef union _znode_op {
	uint32_t      constant;
	uint32_t      var;
	uint32_t      num;
	uint32_t      opline_num; /*  Needs to be signed */
#if ZEND_USE_ABS_JMP_ADDR
	zend_op       *jmp_addr;
#else
	uint32_t      jmp_offset;
#endif
#if ZEND_USE_ABS_CONST_ADDR
	zval          *zv;
#endif
} znode_op;
```

下面我们回头说回这个opcode的函数`ZEND_FREE_SPEC_TMPVAR_HANDLER`
```
static ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL ZEND_FREE_SPEC_TMPVAR_HANDLER(ZEND_OPCODE_HANDLER_ARGS)
{
	USE_OPLINE

	SAVE_OPLINE();
	zval_ptr_dtor_nogc(EX_VAR(opline->op1.var));
	ZEND_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}
```

我们展开宏EX_VAR
```
#define EX_VAR(n)				ZEND_CALL_VAR(execute_data, n)
#define ZEND_CALL_VAR(call, n) \
	((zval*)(((char*)(call)) + ((int)(n))))
	
```
即最后展开成:
```
((zval*)(((char*)(execute_data)) + ((int)(n))))
```

举例:
```
(gdb) p opline->op1.var
$1 = 112
```
即展开成
```
((zval*)(((char*)(execute_data)) + 112))
```
即将execute_data 偏移112 字节的东西转换成指向zval的指针
