


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
----
首先要结束的是revalidate_freq   就是要校验多少秒后需要更新文件
###  revalidate_freq   
- 默认值是2
> opcache.validate_timestamps	"1"	PHP_INI_ALL
[相关参数文档](https://www.php.net/manual/en/opcache.configuration.php)

- gdb 打印
```
(gdb) p accel_globals.accel_directives.revalidate_freq
$3 = 2
```
- 相关函数: `persistent_compile_file`
- 相关逻辑: 
```
/* If script is found then validate_timestamps if option is enabled */
/*如果缓存了该文件 且开启了validate_timestamps ,也就是validate_timestamps=1 */
if(persistent_script && ZCG(accel_directives).validate_timestamps){   
  if (validate_timestamp_and_record(persistent_script, file_handle) == FAILURE) {
    ...
    persistent_script = NULL;      // 如果校验更新时间戳失败 会将变量置空
    ...
  }
}

...
/* If script was not found or invalidated by validate_timestamps */
/*如果找不到缓存或者校验时间戳失败*/
if (!persistent_script) {
        ...
		persistent_script = opcache_compile_file(file_handle, type, key, &op_array);   // 编译生成opcode
        ...
}
        
```
和revalidate_freq  相关的流程就是 如果找到了缓存就校验时间戳是否通过,如果时间戳不通过就置将persistent_script置空,一旦persistent_script置空,就会需要走原来的编译流程生成opcode

