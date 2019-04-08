编译的时候会执行`zend_compile_file` 这个函数指针,这个函数指针就像是java的接口(interface)或者是golang 的接口.  
一般没开opcache走的是`phar_compile_file`  这个函数  
###  没有开启opcache堆栈:  
```
(gdb) bt
#0  compile_file (file_handle=0x7fffffffd1d0, type=8) at Zend/zend_language_scanner.l:622
#1  0x00000000006f5abf in phar_compile_file (file_handle=0x7fffffffd1d0, type=8) at /home/vagrant/php-7.2.13/ext/phar/phar.c:3329
#2  0x0000000000919d28 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/vagrant/php-7.2.13/Zend/zend.c:1496
#3  0x000000000087d9d9 in php_execute_script (primary_file=0x7fffffffd1d0) at /home/vagrant/php-7.2.13/main/main.c:2590
#4  0x0000000000a0d115 in do_cli (argc=2, argv=0x13f9790) at /home/vagrant/php-7.2.13/sapi/cli/php_cli.c:1011
#5  0x0000000000a0e2d2 in main (argc=2, argv=0x13f9790) at /home/vagrant/php-7.2.13/sapi/cli/php_cli.c:1403

```

开启opcache后不会走 `phar_compile_file`,而是走函数`persistent_compile_file`  
### opcache开启后堆栈:  
```
#0  persistent_compile_file (file_handle=0x7fffffffd1d0, type=8) at /home/vagrant/php-7.2.13/ext/opcache/ZendAccelerator.c:1740
#1  0x0000000000919d28 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/vagrant/php-7.2.13/Zend/zend.c:1496
#2  0x000000000087d9d9 in php_execute_script (primary_file=0x7fffffffd1d0) at /home/vagrant/php-7.2.13/main/main.c:2590
#3  0x0000000000a0d115 in do_cli (argc=2, argv=0x13f9790) at /home/vagrant/php-7.2.13/sapi/cli/php_cli.c:1011
#4  0x0000000000a0e2d2 in main (argc=2, argv=0x13f9790) at /home/vagrant/php-7.2.13/sapi/cli/php_cli.c:1403
```
```
//路径 php-src/ext/opcache/ZendAccelerator.c
/* zend_compile() replacement */
zend_op_array *persistent_compile_file(zend_file_handle *file_handle, int type)
{
	zend_persistent_script *persistent_script = NULL;
	char *key = NULL;
	int key_length;
	int from_shared_memory; /* if the script we've got is stored in SHM */

	if (!file_handle->filename || !ZCG(enabled) || !accel_startup_ok) {
		/* The Accelerator is disabled, act as if without the Accelerator */
		return accelerator_orig_compile_file(file_handle, type);
#ifdef HAVE_OPCACHE_FILE_CACHE
	} else if (file_cache_only) {
		return file_cache_compile_file(file_handle, type);
#endif
	} else if ((!ZCG(counted) && !ZCSG(accelerator_enabled)) ||
	           (ZCSG(restart_in_progress) && accel_restart_is_active())) {
#ifdef HAVE_OPCACHE_FILE_CACHE
		if (ZCG(accel_directives).file_cache) {
			return file_cache_compile_file(file_handle, type);
		}
#endif
		return accelerator_orig_compile_file(file_handle, type);
	}

	/* In case this callback is called from include_once, require_once or it's
	 * a main FastCGI request, the key must be already calculated, and cached
	 * persistent script already found */
	if (ZCG(cache_persistent_script) &&
	    ((!EG(current_execute_data) &&
	      file_handle->filename == SG(request_info).path_translated &&
	      ZCG(cache_opline) == NULL) ||
	     (EG(current_execute_data) &&
	      EG(current_execute_data)->func &&
	      ZEND_USER_CODE(EG(current_execute_data)->func->common.type) &&
	      ZCG(cache_opline) == EG(current_execute_data)->opline))) {

		persistent_script = ZCG(cache_persistent_script);
		if (ZCG(key_len)) {
			key = ZCG(key);
			key_length = ZCG(key_len);
		}

	} else {
		if (!ZCG(accel_directives).revalidate_path) {
			/* try to find cached script by key */
			key = accel_make_persistent_key(file_handle->filename, strlen(file_handle->filename), &key_length);
			if (!key) {
				return accelerator_orig_compile_file(file_handle, type);
			}
			persistent_script = zend_accel_hash_str_find(&ZCSG(hash), key, key_length);
		} else if (UNEXPECTED(is_stream_path(file_handle->filename) && !is_cacheable_stream_path(file_handle->filename))) {
			return accelerator_orig_compile_file(file_handle, type);
		}

		if (!persistent_script) {
			/* try to find cached script by full real path */
			zend_accel_hash_entry *bucket;

			/* open file to resolve the path */
		    if (file_handle->type == ZEND_HANDLE_FILENAME &&
        		accelerator_orig_zend_stream_open_function(file_handle->filename, file_handle) == FAILURE) {
				if (type == ZEND_REQUIRE) {
					zend_message_dispatcher(ZMSG_FAILED_REQUIRE_FOPEN, file_handle->filename);
					zend_bailout();
				} else {
					zend_message_dispatcher(ZMSG_FAILED_INCLUDE_FOPEN, file_handle->filename);
				}
				return NULL;
		    }

			if (file_handle->opened_path) {
				bucket = zend_accel_hash_find_entry(&ZCSG(hash), file_handle->opened_path);

				if (bucket) {
					persistent_script = (zend_persistent_script *)bucket->data;

					if (key && !persistent_script->corrupted) {
						HANDLE_BLOCK_INTERRUPTIONS();
						SHM_UNPROTECT();
						zend_shared_alloc_lock();
						zend_accel_add_key(key, key_length, bucket);
						zend_shared_alloc_unlock();
						SHM_PROTECT();
						HANDLE_UNBLOCK_INTERRUPTIONS();
					}
				}
			}
		}
	}

	/* clear cache */
	ZCG(cache_opline) = NULL;
	ZCG(cache_persistent_script) = NULL;

	if (persistent_script && persistent_script->corrupted) {
		persistent_script = NULL;
	}

	/* Make sure we only increase the currently running processes semaphore
     * once each execution (this function can be called more than once on
     * each execution)
     */
	if (!ZCG(counted)) {
		if (accel_activate_add() == FAILURE) {
#ifdef HAVE_OPCACHE_FILE_CACHE
			if (ZCG(accel_directives).file_cache) {
				return file_cache_compile_file(file_handle, type);
			}
#endif
			return accelerator_orig_compile_file(file_handle, type);
		}
		ZCG(counted) = 1;
	}

	/* Revalidate accessibility of cached file */
	if (EXPECTED(persistent_script != NULL) &&
	    UNEXPECTED(ZCG(accel_directives).validate_permission) &&
	    file_handle->type == ZEND_HANDLE_FILENAME &&
	    UNEXPECTED(access(ZSTR_VAL(persistent_script->script.filename), R_OK) != 0)) {
		if (type == ZEND_REQUIRE) {
			zend_message_dispatcher(ZMSG_FAILED_REQUIRE_FOPEN, file_handle->filename);
			zend_bailout();
		} else {
			zend_message_dispatcher(ZMSG_FAILED_INCLUDE_FOPEN, file_handle->filename);
		}
		return NULL;
	}

	HANDLE_BLOCK_INTERRUPTIONS();
	SHM_UNPROTECT();

	/* If script is found then validate_timestamps if option is enabled */
	if (persistent_script && ZCG(accel_directives).validate_timestamps) {
		if (validate_timestamp_and_record(persistent_script, file_handle) == FAILURE) {
			zend_shared_alloc_lock();
			if (!persistent_script->corrupted) {
				persistent_script->corrupted = 1;
				persistent_script->timestamp = 0;
				ZSMMG(wasted_shared_memory) += persistent_script->dynamic_members.memory_consumption;
				if (ZSMMG(memory_exhausted)) {
					zend_accel_restart_reason reason =
						zend_accel_hash_is_full(&ZCSG(hash)) ? ACCEL_RESTART_HASH : ACCEL_RESTART_OOM;
					zend_accel_schedule_restart_if_necessary(reason);
				}
			}
			zend_shared_alloc_unlock();
			persistent_script = NULL;
		}
	}

	/* if turned on - check the compiled script ADLER32 checksum */
	if (persistent_script && ZCG(accel_directives).consisten
