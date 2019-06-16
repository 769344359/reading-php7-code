当有isset/empty的时候
```
(gdb) bt
#0  ZEND_ISSET_ISEMPTY_DIM_OBJ_SPEC_CV_CONST_HANDLER () at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:38134
#1  0x0000000000a3b254 in execute_ex (ex=0x7fffef61f030) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:62314
#2  0x0000000000a3d0ab in zend_execute (op_array=0x7fffef684300, return_value=0x0) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:63760
#3  0x000000000094cd22 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend.c:1496
#4  0x00000000008b0b4a in php_execute_script (primary_file=0x7fffffffc9a0) at /home/dinosaur/Downloads/php-7.2.2/main/main.c:2590
#5  0x0000000000a3fd23 in do_cli (argc=2, argv=0x1442010) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1011
#6  0x0000000000a40ee0 in main (argc=2, argv=0x1442010) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1404

```
当没有isset/empty的时候：
```
(gdb) bt
#0  zend_fetch_dimension_address_inner (ht=0x7fffe70e61e0, dim=0x7fffe70e61b0, dim_type=1, type=0) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_execute.c:1548
#1  0x0000000000a0484e in ZEND_FETCH_DIM_R_SPEC_CV_CONST_HANDLER () at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:35926
#2  0x0000000000a3b054 in execute_ex (ex=0x7fffef61f030) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:62218
#3  0x0000000000a3d0ab in zend_execute (op_array=0x7fffef684300, return_value=0x0) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:63760
#4  0x000000000094cd22 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend.c:1496
#5  0x00000000008b0b4a in php_execute_script (primary_file=0x7fffffffc9a0) at /home/dinosaur/Downloads/php-7.2.2/main/main.c:2590
#6  0x0000000000a3fd23 in do_cli (argc=2, argv=0x1442010) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1011
#7  0x0000000000a40ee0 in main (argc=2, argv=0x1442010) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1404
```
