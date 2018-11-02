```

/* regular data types */
#define IS_UNDEF					0
#define IS_NULL						1
#define IS_FALSE					2
#define IS_TRUE						3
#define IS_LONG						4
#define IS_DOUBLE					5
#define IS_STRING					6
#define IS_ARRAY					7
#define IS_OBJECT					8
#define IS_RESOURCE					9
#define IS_REFERENCE				10

/* constant expressions */
#define IS_CONSTANT					11
#define IS_CONSTANT_AST				12

/* fake types */
#define _IS_BOOL					13
#define IS_CALLABLE					14
#define IS_ITERABLE					19
#define IS_VOID						18

/* internal types */
#define IS_INDIRECT             	15
#define IS_PTR						17
#define _IS_ERROR					20
```

堆栈
```
(gdb) bt
#0  ZEND_COALESCE_SPEC_CV_HANDLER () at /home/vagrant/php-7.2.5/Zend/zend_vm_execute.h:34257
#1  0x0000000000aebedf in execute_ex (ex=0x7ffff621e030) at /home/vagrant/php-7.2.5/Zend/zend_vm_execute.h:61996
#2  0x0000000000aee3d4 in zend_execute (op_array=0x7ffff6280300, return_value=0x0) at /home/vagrant/php-7.2.5/Zend/zend_vm_execute.h:63760
#3  0x0000000000a068f1 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/vagrant/php-7.2.5/Zend/zend.c:1496
#4  0x000000000096ef77 in php_execute_script (primary_file=0x7fffffffd250) at /home/vagrant/php-7.2.5/main/main.c:2590
#5  0x0000000000af0dcb in do_cli (argc=2, argv=0x1631ae0) at /home/vagrant/php-7.2.5/sapi/cli/php_cli.c:1011
#6  0x0000000000af1f7d in main (argc=2, argv=0x1631ae0) at /home/vagrant/php-7.2.5/sapi/cli/php_cli.c:1404

```
