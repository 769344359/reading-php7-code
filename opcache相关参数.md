


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
和revalidate_freq  相关的流程就是:
如果找到了缓存且开启了validate_timestamps就校验时间戳是否通过
1 通过就没什么事
2 如果时间戳不通过就置将persistent_script置空,一旦persistent_script置空,就会需要走原来的编译流程生成opcode

那么这个核心函数就是校验时间戳的函数`validate_timestamp_and_record`

这个函数`validate_timestamp_and_record`
```
int validate_timestamp_and_record(zend_persistent_script *persistent_script, zend_file_handle *file_handle)
{
	if (persistent_script->timestamp == 0) {   // 0 是个特殊值表示不校验  返回校验通过
		return SUCCESS; /* Don't check timestamps of preloaded scripts */
	} else if (ZCG(accel_directives).revalidate_freq &&
	    persistent_script->dynamic_members.revalidate >= ZCG(request_time)) {   // 如果php.ini 中配置revalidate_freq不为0 且脚本下一次验证时间大于请求时间 返回成功
		return SUCCESS;
	} else if (do_validate_timestamps(persistent_script, file_handle) == FAILURE) {
		return FAILURE;
	} else {
		persistent_script->dynamic_members.revalidate = ZCG(request_time) + ZCG(accel_directives).revalidate_freq;  // 如果脚本经过了时间戳校验函数do_validate_timestamps 的校验,就更新该文件的下一次校验时间为请求时间+ 重新校验间隔revalidate_freq
		return SUCCESS;
	}
}
```
然后我们仔细再进入 do_validate_timestamps 函数

```
static inline int do_validate_timestamps(zend_persistent_script *persistent_script, zend_file_handle *file_handle)
{
	zend_file_handle ps_handle;
	zend_string *full_path_ptr = NULL;

	/** check that the persistent script is indeed the same file we cached
	 * (if part of the path is a symlink than it possible that the user will change it)
	 * See bug #15140
	 */
	if (file_handle->opened_path) {        // 校验软链相关 如果路径不相同了那就校验不通过,需要重新生成
		if (persistent_script->script.filename != file_handle->opened_path &&
		    !zend_string_equal_content(persistent_script->script.filename, file_handle->opened_path)) {
			return FAILURE;
		}
	} else {
		full_path_ptr = accelerator_orig_zend_resolve_path(file_handle->filename, strlen(file_handle->filename));
		if (full_path_ptr &&
		    persistent_script->script.filename != full_path_ptr &&
		    !zend_string_equal_content(persistent_script->script.filename, full_path_ptr)) {
			zend_string_release_ex(full_path_ptr, 0);
			return FAILURE;
		}
		file_handle->opened_path = full_path_ptr;
	}

	if (persistent_script->timestamp == 0) {
		if (full_path_ptr) {
			zend_string_release_ex(full_path_ptr, 0);
			file_handle->opened_path = NULL;
		}
		return FAILURE;
	}

	if (zend_get_file_handle_timestamp(file_handle, NULL) == persistent_script->timestamp) {   // 通过glibc 的stat 获取文件的修改时间跟记录的时间一致就校验通过
		if (full_path_ptr) {
			zend_string_release_ex(full_path_ptr, 0);
			file_handle->opened_path = NULL;
		}
		return SUCCESS;
	}
	if (full_path_ptr) {
		zend_string_release_ex(full_path_ptr, 0);
		file_handle->opened_path = NULL;
	}

	ps_handle.type = ZEND_HANDLE_FILENAME;
	ps_handle.filename = ZSTR_VAL(persistent_script->script.filename);
	ps_handle.opened_path = persistent_script->script.filename;

	if (zend_get_file_handle_timestamp(&ps_handle, NULL) == persistent_script->timestamp) {
		return SUCCESS;
	}

	return FAILURE;
}
```


看完函数`do_validate_timestamps` 几乎可以确认只有文件的修改时间和上一次编译时候stat读出来的timestamp是相等才会返回true



---

相关参考:  
- [stat函数](https://linux.die.net/man/2/stat)
- 这次分析的php源码版本7.3.1
