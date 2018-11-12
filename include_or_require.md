```
(gdb) bt
#0  zend_include_or_eval (inc_filename=0x7fffef602b80, type=2) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_execute.c:2763
#1  0x00000000009b861b in ZEND_INCLUDE_OR_EVAL_SPEC_CONST_HANDLER () at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:3437
#2  0x0000000000a380d4 in execute_ex (ex=0x7fffef61e030) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:59938
#3  0x0000000000a3d0ab in zend_execute (op_array=0x7fffef67c400, return_value=0x0) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:63760
#4  0x000000000094cd22 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend.c:1496
#5  0x00000000008b0b4a in php_execute_script (primary_file=0x7fffffffca40) at /home/dinosaur/Downloads/php-7.2.2/main/main.c:2590
#6  0x0000000000a3fd23 in do_cli (argc=2, argv=0x1441f40) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1011
#7  0x0000000000a40ee0 in main (argc=2, argv=0x1441f40) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1404

```
```
(gdb) bt
#0  open64 () at ../sysdeps/unix/syscall-template.S:84
#1  0x00000000008d94c5 in _php_stream_fopen (filename=0x7fffef6627f8 "/home/dinosaur/test/aa.php", mode=0x1064fb1 "rb", opened_path=0x7fffffffa260, options=16512, __php_stream_call_depth=2, 
    __zend_filename=0x10688f0 "/home/dinosaur/Downloads/php-7.2.2/main/streams/plain_wrapper.c", __zend_lineno=1080, __zend_orig_filename=0x10645f8 "/home/dinosaur/Downloads/php-7.2.2/main/main.c", 
    __zend_orig_lineno=1414) at /home/dinosaur/Downloads/php-7.2.2/main/streams/plain_wrapper.c:1017
#2  0x00000000008d9805 in php_plain_files_stream_opener (wrapper=0x13f5000 <php_plain_files_wrapper>, path=0x7fffef6627f8 "/home/dinosaur/test/aa.php", mode=0x1064fb1 "rb", options=16512, 
    opened_path=0x7fffffffa260, context=0x0, __php_stream_call_depth=1, __zend_filename=0x1067d78 "/home/dinosaur/Downloads/php-7.2.2/main/streams/streams.c", __zend_lineno=2029, 
    __zend_orig_filename=0x10645f8 "/home/dinosaur/Downloads/php-7.2.2/main/main.c", __zend_orig_lineno=1414) at /home/dinosaur/Downloads/php-7.2.2/main/streams/plain_wrapper.c:1080
#3  0x00000000008d2b24 in _php_stream_open_wrapper_ex (path=0x7fffef6627f8 "/home/dinosaur/test/aa.php", mode=0x1064fb1 "rb", options=16520, opened_path=0x7fffffffa260, context=0x0, 
    __php_stream_call_depth=0, __zend_filename=0x10645f8 "/home/dinosaur/Downloads/php-7.2.2/main/main.c", __zend_lineno=1414, __zend_orig_filename=0x0, __zend_orig_lineno=0)
    at /home/dinosaur/Downloads/php-7.2.2/main/streams/streams.c:2027
#4  0x00000000008ae2a0 in php_stream_open_for_zend_ex (filename=0x7fffef602b58 "aa.php", handle=0x7fffffffa200, mode=137) at /home/dinosaur/Downloads/php-7.2.2/main/main.c:1414
#5  0x00000000008ae23e in php_stream_open_for_zend (filename=0x7fffef602b58 "aa.php", handle=0x7fffffffa200) at /home/dinosaur/Downloads/php-7.2.2/main/main.c:1406
#6  0x000000000097608f in zend_stream_open (filename=0x7fffef602b58 "aa.php", handle=0x7fffffffa200) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_stream.c:131
#7  0x000000000097626f in zend_stream_fixup (file_handle=0x7fffffffa200, buf=0x7fffffff9ed8, len=0x7fffffff9ee0) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_stream.c:177
#8  0x00000000008ebeb0 in open_file_for_scanning (file_handle=0x7fffffffa200) at Zend/zend_language_scanner.l:513
#9  0x00000000008ec489 in compile_file (file_handle=0x7fffffffa200, type=2) at Zend/zend_language_scanner.l:627
#10 0x00000000007296f0 in phar_compile_file (file_handle=0x7fffffffa200, type=2) at /home/dinosaur/Downloads/php-7.2.2/ext/phar/phar.c:3320
#11 0x00000000008ec619 in compile_filename (type=2, filename=0x7fffef602b80) at Zend/zend_language_scanner.l:662
#12 0x00000000009b0524 in zend_include_or_eval (inc_filename=0x7fffef602b80, type=2) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_execute.c:2826
#13 0x00000000009b861b in ZEND_INCLUDE_OR_EVAL_SPEC_CONST_HANDLER () at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:3437
#14 0x0000000000a380d4 in execute_ex (ex=0x7fffef61e030) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:59938
#15 0x0000000000a3d0ab in zend_execute (op_array=0x7fffef67c400, return_value=0x0) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:63760
#16 0x000000000094cd22 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend.c:1496
#17 0x00000000008b0b4a in php_execute_script (primary_file=0x7fffffffca40) at /home/dinosaur/Downloads/php-7.2.2/main/main.c:2590
#18 0x0000000000a3fd23 in do_cli (argc=2, argv=0x1441f40) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1011
#19 0x0000000000a40ee0 in main (argc=2, argv=0x1441f40) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1404

```
```
static ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL ZEND_INCLUDE_OR_EVAL_SPEC_CONST_HANDLER(ZEND_OPCODE_HANDLER_ARGS)
{
	USE_OPLINE
	zend_op_array *new_op_array;

	zval *inc_filename;

	SAVE_OPLINE();
	inc_filename = EX_CONSTANT(opline->op1);
	new_op_array = zend_include_or_eval(inc_filename, opline->extended_value);

	if (UNEXPECTED(EG(exception) != NULL)) {
		if (new_op_array != ZEND_FAKE_OP_ARRAY && new_op_array != NULL) {
			destroy_op_array(new_op_array);
			efree_size(new_op_array, sizeof(zend_op_array));
		}
		UNDEF_RESULT();
		HANDLE_EXCEPTION();
	} else if (new_op_array == ZEND_FAKE_OP_ARRAY) {
		if (RETURN_VALUE_USED(opline)) {
			ZVAL_TRUE(EX_VAR(opline->result.var));
		}
	} else if (EXPECTED(new_op_array != NULL)) {
		zval *return_value = NULL;
		zend_execute_data *call;

		if (RETURN_VALUE_USED(opline)) {
			return_value = EX_VAR(opline->result.var);
			ZVAL_NULL(return_value);
		}

		new_op_array->scope = EX(func)->op_array.scope;

		call = zend_vm_stack_push_call_frame(ZEND_CALL_NESTED_CODE | ZEND_CALL_HAS_SYMBOL_TABLE,
			(zend_function*)new_op_array, 0,
			Z_TYPE(EX(This)) != IS_OBJECT ? Z_CE(EX(This)) : NULL,
			Z_TYPE(EX(This)) == IS_OBJECT ? Z_OBJ(EX(This)) : NULL);

		if (EX_CALL_INFO() & ZEND_CALL_HAS_SYMBOL_TABLE) {
			call->symbol_table = EX(symbol_table);
		} else {
			call->symbol_table = zend_rebuild_symbol_table();
		}

		call->prev_execute_data = execute_data;
		i_init_code_execute_data(call, new_op_array, return_value);
		if (EXPECTED(zend_execute_ex == execute_ex)) {
			ZEND_VM_ENTER();
		} else {
			ZEND_ADD_CALL_FLAG(call, ZEND_CALL_TOP);
			zend_execute_ex(call);
			zend_vm_stack_free_call_frame(call);
		}

		destroy_op_array(new_op_array);
		efree_size(new_op_array, sizeof(zend_op_array));
		if (UNEXPECTED(EG(exception) != NULL)) {
			zend_rethrow_exception(execute_data);
			UNDEF_RESULT();
			HANDLE_EXCEPTION();
		}
	} else if (RETURN_VALUE_USED(opline)) {
		ZVAL_FALSE(EX_VAR(opline->result.var));
	}
	ZEND_VM_SET_OPCODE(opline + 1);
	ZEND_VM_CONTINUE();
}
```

```
static zend_never_inline zend_op_array* ZEND_FASTCALL zend_include_or_eval(zval *inc_filename, int type) /* {{{ */
{
	zend_op_array *new_op_array = NULL;
	zval tmp_inc_filename;

	ZVAL_UNDEF(&tmp_inc_filename);
	if (Z_TYPE_P(inc_filename) != IS_STRING) {
		ZVAL_STR(&tmp_inc_filename, zval_get_string(inc_filename));
		inc_filename = &tmp_inc_filename;
	}

	if (type != ZEND_EVAL && strlen(Z_STRVAL_P(inc_filename)) != Z_STRLEN_P(inc_filename)) {
		if (type == ZEND_INCLUDE_ONCE || type == ZEND_INCLUDE) {
			zend_message_dispatcher(ZMSG_FAILED_INCLUDE_FOPEN, Z_STRVAL_P(inc_filename));
		} else {
			zend_message_dispatcher(ZMSG_FAILED_REQUIRE_FOPEN, Z_STRVAL_P(inc_filename));
		}
	} else {
		switch (type) {
			case ZEND_INCLUDE_ONCE:
			case ZEND_REQUIRE_ONCE: {
					zend_file_handle file_handle;
					zend_string *resolved_path;

					resolved_path = zend_resolve_path(Z_STRVAL_P(inc_filename), (int)Z_STRLEN_P(inc_filename));
					if (resolved_path) {
						if (zend_hash_exists(&EG(included_files), resolved_path)) {
							goto already_compiled;
						}
					} else {
						resolved_path = zend_string_copy(Z_STR_P(inc_filename));
					}

					if (SUCCESS == zend_stream_open(ZSTR_VAL(resolved_path), &file_handle)) {

						if (!file_handle.opened_path) {
							file_handle.opened_path = zend_string_copy(resolved_path);
						}

						if (zend_hash_add_empty_element(&EG(included_files), file_handle.opened_path)) {
							zend_op_array *op_array = zend_compile_file(&file_handle, (type==ZEND_INCLUDE_ONCE?ZEND_INCLUDE:ZEND_REQUIRE));
							zend_destroy_file_handle(&file_handle);
							zend_string_release(resolved_path);
							if (Z_TYPE(tmp_inc_filename) != IS_UNDEF) {
								zend_string_release(Z_STR(tmp_inc_filename));
							}
							return op_array;
						} else {
							zend_file_handle_dtor(&file_handle);
already_compiled:
							new_op_array = ZEND_FAKE_OP_ARRAY;
						}
					} else {
						if (type == ZEND_INCLUDE_ONCE) {
							zend_message_dispatcher(ZMSG_FAILED_INCLUDE_FOPEN, Z_STRVAL_P(inc_filename));
						} else {
							zend_message_dispatcher(ZMSG_FAILED_REQUIRE_FOPEN, Z_STRVAL_P(inc_filename));
						}
					}
					zend_string_release(resolved_path);
				}
				break;
			case ZEND_INCLUDE:
			case ZEND_REQUIRE:
				new_op_array = compile_filename(type, inc_filename);
				break;
			case ZEND_EVAL: {
					char *eval_desc = zend_make_compiled_string_description("eval()'d code");
					new_op_array = zend_compile_string(inc_filename, eval_desc);
					efree(eval_desc);
				}
				break;
			EMPTY_SWITCH_DEFAULT_CASE()
		}
	}
	if (Z_TYPE(tmp_inc_filename) != IS_UNDEF) {
		zend_string_release(Z_STR(tmp_inc_filename));
	}
	return new_op_array;
}
/* }}} */
```
`include` 和`require` 和`require_once` `include_once` 会调用
```
(gdb) bt
#0  php_resolve_path (filename=0x7fffef602d98 "aaa.php", filename_length=7, path=0x1481e78 ".:/usr/local/phpfork/lib/php") at /home/dinosaur/Downloads/php-7.2.2/main/fopen_wrappers.c:476
#1  0x00000000008ae472 in php_resolve_path_for_zend (filename=0x7fffef602d98 "aaa.php", filename_len=7) at /home/dinosaur/Downloads/php-7.2.2/main/main.c:1455
#2  0x00000000006fbe55 in phar_find_in_include_path (filename=0x7fffef602d98 "aaa.php", filename_len=7, pphar=0x7fffffff9c88) at /home/dinosaur/Downloads/php-7.2.2/ext/phar/util.c:260
#3  0x0000000000729199 in phar_resolve_path (filename=0x7fffef602d98 "aaa.php", filename_len=7) at /home/dinosaur/Downloads/php-7.2.2/ext/phar/phar.c:3254
#4  0x00000000008d29e1 in _php_stream_open_wrapper_ex (path=0x7fffef602d98 "aaa.php", mode=0x1064fb1 "rb", options=137, opened_path=0x7fffffffa270, context=0x0, __php_stream_call_depth=0, 
    __zend_filename=0x10645f8 "/home/dinosaur/Downloads/php-7.2.2/main/main.c", __zend_lineno=1414, __zend_orig_filename=0x0, __zend_orig_lineno=0)
    at /home/dinosaur/Downloads/php-7.2.2/main/streams/streams.c:2002
#5  0x00000000008ae2a0 in php_stream_open_for_zend_ex (filename=0x7fffef602d98 "aaa.php", handle=0x7fffffffa210, mode=137) at /home/dinosaur/Downloads/php-7.2.2/main/main.c:1414
#6  0x00000000008ae23e in php_stream_open_for_zend (filename=0x7fffef602d98 "aaa.php", handle=0x7fffffffa210) at /home/dinosaur/Downloads/php-7.2.2/main/main.c:1406
#7  0x000000000097608f in zend_stream_open (filename=0x7fffef602d98 "aaa.php", handle=0x7fffffffa210) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_stream.c:131
#8  0x000000000097626f in zend_stream_fixup (file_handle=0x7fffffffa210, buf=0x7fffffff9ee8, len=0x7fffffff9ef0) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_stream.c:177
#9  0x00000000008ebeb0 in open_file_for_scanning (file_handle=0x7fffffffa210) at Zend/zend_language_scanner.l:513
#10 0x00000000008ec489 in compile_file (file_handle=0x7fffffffa210, type=2) at Zend/zend_language_scanner.l:627
#11 0x00000000007296f0 in phar_compile_file (file_handle=0x7fffffffa210, type=2) at /home/dinosaur/Downloads/php-7.2.2/ext/phar/phar.c:3320
#12 0x00000000008ec619 in compile_filename (type=2, filename=0x7fffef602dc0) at Zend/zend_language_scanner.l:662
#13 0x00000000009b0524 in zend_include_or_eval (inc_filename=0x7fffef602dc0, type=2) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_execute.c:2826
#14 0x00000000009b861b in ZEND_INCLUDE_OR_EVAL_SPEC_CONST_HANDLER () at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:3437
#15 0x0000000000a380d4 in execute_ex (ex=0x7fffef61e030) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:59938
#16 0x0000000000a3d0ab in zend_execute (op_array=0x7fffef67c400, return_value=0x0) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend_vm_execute.h:63760
#17 0x000000000094cd22 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/dinosaur/Downloads/php-7.2.2/Zend/zend.c:1496
#18 0x00000000008b0b4a in php_execute_script (primary_file=0x7fffffffca50) at /home/dinosaur/Downloads/php-7.2.2/main/main.c:2590
#19 0x0000000000a3fd23 in do_cli (argc=2, argv=0x1441f40) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1011
#20 0x0000000000a40ee0 in main (argc=2, argv=0x1441f40) at /home/dinosaur/Downloads/php-7.2.2/sapi/cli/php_cli.c:1404

```
