


堆栈:
```
(gdb) bt
#0  opcache_compile_file (file_handle=0x7fffffffe120, type=8, key=0x7fffea6cdccc <accel_globals+556> "test.php:2403152:2281072", op_array_p=0x7fffffffabf8)
    at /data/soft/php-7.3.1/ext/opcache/ZendAccelerator.c:1707
#1  0x00007fffea41da03 in persistent_compile_file (file_handle=0x7fffffffe120, type=8) at /data/soft/php-7.3.1/ext/opcache/ZendAccelerator.c:2095
#2  0x0000000000a98f37 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /data/soft/php-7.3.1/Zend/zend.c:1562
#3  0x0000000000a093dd in php_execute_script (primary_file=0x7fffffffe120) at /data/soft/php-7.3.1/main/main.c:2630
#4  0x0000000000b6bbca in do_cli (argc=2, argv=0x1754d00) at /data/soft/php-7.3.1/sapi/cli/php_cli.c:997
#5  0x0000000000b6cb36 in main (argc=2, argv=0x1754d00) at /data/soft/php-7.3.1/sapi/cli/php_cli.c:1389

```
###  revalidate_freq   
```
(gdb) p accel_globals.accel_directives.revalidate_freq
$3 = 2
```
```
if(persistent_script && ZCG(accel_directives).validate_timestamps){
  if (validate_timestamp_and_record(persistent_script, file_handle) == FAILURE) {
    ...
    persistent_script = NULL;
    ...
  }
}
```
