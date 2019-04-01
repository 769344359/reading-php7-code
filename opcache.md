```
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
	if (persistent_script && ZCG(accel_directives).consistency_checks
		&& persistent_script->dynamic_members.hits % ZCG(accel_directives).consistency_checks == 0) {

		unsigned int checksum = zend_accel_script_checksum(persistent_script);
		if (checksum != persistent_script->dynamic_members.checksum ) {
			/* The checksum is wrong */
			zend_accel_error(ACCEL_LOG_INFO, "Checksum failed for '%s':  expected=0x%08x, found=0x%08x",
							 ZSTR_VAL(persistent_script->script.filename), persistent_script->dynamic_members.checksum, checksum);
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

#ifdef HAVE_OPCACHE_FILE_CACHE
	/* Check the second level cache */
	if (!persistent_script && ZCG(accel_directives).file_cache) {
		persistent_script = zend_file_cache_script_load(file_handle);
	}
#endif

	/* If script was not found or invalidated by validate_timestamps */
	if (!persistent_script) {
		uint32_t old_const_num = zend_hash_next_free_element(EG(zend_constants));
		zend_op_array *op_array;

		/* Cache miss.. */
		ZCSG(misses)++;

		/* No memory left. Behave like without the Accelerator */
		if (ZSMMG(memory_exhausted) || ZCSG(restart_pending)) {
			SHM_PROTECT();
			HANDLE_UNBLOCK_INTERRUPTIONS();
#ifdef HAVE_OPCACHE_FILE_CACHE
			if (ZCG(accel_directives).file_cache) {
				return file_cache_compile_file(file_handle, type);
			}
#endif
			return accelerator_orig_compile_file(file_handle, type);
		}

		/* Try and cache the script and assume that it is returned from_shared_memory.
         * If it isn't compile_and_cache_file() changes the flag to 0
         */
       	from_shared_memory = 0;
		persistent_script = opcache_compile_file(file_handle, type, key, &op_array);
		if (persistent_script) {
			persistent_script = cache_script_in_shared_memory(persistent_script, key, key ? key_length : 0, &from_shared_memory);
		}

		/* Caching is disabled, returning op_array;
		 * or something went wrong during compilation, returning NULL
		 */
		if (!persistent_script) {
			SHM_PROTECT();
			HANDLE_UNBLOCK_INTERRUPTIONS();
			return op_array;
		}
		if (from_shared_memory) {
			/* Delete immutable arrays moved into SHM */
			uint32_t new_const_num = zend_hash_next_free_element(EG(zend_constants));
			while (new_const_num > old_const_num) {
				new_const_num--;
				zend_hash_index_del(EG(zend_constants), new_const_num);
			}
		}
	} else {

#if !ZEND_WIN32
		ZCSG(hits)++; /* TBFixed: may lose one hit */
		persistent_script->dynamic_members.hits++; /* see above */
#else
#ifdef _M_X64
		InterlockedIncrement64(&ZCSG(hits));
#else
		InterlockedIncrement(&ZCSG(hits));
#endif
		InterlockedIncrement64(&persistent_script->dynamic_members.hits);
#endif

		/* see bug #15471 (old BTS) */
		if (persistent_script->script.filename) {
			if (!EG(current_execute_data) || !EG(current_execute_data)->opline ||
			    !EG(current_execute_data)->func ||
			    !ZEND_USER_CODE(EG(current_execute_data)->func->common.type) ||
			    EG(current_execute_data)->opline->opcode != ZEND_INCLUDE_OR_EVAL ||
			    (EG(current_execute_data)->opline->extended_value != ZEND_INCLUDE_ONCE &&
			     EG(current_execute_data)->opline->extended_value != ZEND_REQUIRE_ONCE)) {
				if (zend_hash_add_empty_element(&EG(included_files), persistent_script->script.filename) != NULL) {
					/* ext/phar has to load phar's metadata into memory */
					if (persistent_script->is_phar) {
						php_stream_statbuf ssb;
						char *fname = emalloc(sizeof("phar://") + ZSTR_LEN(persistent_script->script.filename));

						memcpy(fname, "phar://", sizeof("phar://") - 1);
						memcpy(fname + sizeof("phar://") - 1, ZSTR_VAL(persistent_script->script.filename), ZSTR_LEN(persistent_script->script.filename) + 1);
						php_stream_stat_path(fname, &ssb);
						efree(fname);
					}
				}
			}
		}
		zend_file_handle_dtor(file_handle);
		from_shared_memory = 1;
	}

	persistent_script->dynamic_members.last_used = ZCG(request_time);

	SHM_PROTECT();
	HANDLE_UNBLOCK_INTERRUPTIONS();

    /* Fetch jit auto globals used in the script before execution */
    if (persistent_script->ping_auto_globals_mask) {
		zend_accel_set_auto_globals(persistent_script->ping_auto_globals_mask);
	}

	return zend_accel_load_script(persistent_script, from_shared_memory);
}
```
