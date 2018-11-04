```
(gdb) bt
#0  zend_update_property_stringl (scope=0x15bcfb0, object=0x7fffef61e2a0, 
    name=0x7fffeed4e95f "name", name_length=4, value=0x7fffef66dd18 "hello_e", 
    value_len=7) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_API.c:4045
#1  0x00007fffeed2f766 in zim_amqp_exchange_class_setName (
    execute_data=0x7fffef61e280, return_value=0x7fffffffa370)
    at /home/dinosaur/php-amqp/amqp_exchange.c:122
#2  0x00000000009b1dfa in ZEND_DO_FCALL_SPEC_RETVAL_UNUSED_HANDLER ()
    at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:907
#3  0x0000000000a37ced in execute_ex (ex=0x7fffef61e030)
    at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:59749
#4  0x0000000000a3d0ab in zend_execute (op_array=0x7fffef67c400, 
    return_value=0x0)
    at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:63760
#5  0x000000000094cd22 in zend_execute_scripts (type=8, retval=0x0, 
    file_count=3) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend.c:1496
#6  0x00000000008b0b4a in php_execute_script (primary_file=0x7fffffffca30)
    at /home/dinosaur/Downloads/php-7.2.2/main/main.c:2590
#7  0x0000000000a3fd23 in do_cli (argc=2, argv=0x1441e70)
    at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1011
#8  0x0000000000a40ee0 in main (argc=2, argv=0x1441e70)
    at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1404
```
