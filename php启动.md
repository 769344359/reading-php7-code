> 1 main 函数

`sapi\fpm\fpm\fpm_main.c` 的大约 1570 行

> 2 调用`startup` 初始化扩展和其他内容 
这个函数会启动所有的扩展的初始化函数 也就是`PHP_MINIT`
```
	/* startup after we get the above ini override se we get things right */
	if (cgi_sapi_module.startup(&cgi_sapi_module) == FAILURE) {    // 初始化
```
> 3 初始化完成之后调用 `fpm_init`
```
	if (0 > fpm_init(argc, argv, fpm_config ? fpm_config : CGIG(fpm_config), fpm_prefix, fpm_pid, test_conf, php_allow_to_run_as_root, force_daemon, force_stderr)) {   

		if (fpm_globals.send_config_pipe[1]) {
			int writeval = 0;
			zlog(ZLOG_DEBUG, "Sending \"0\" (error) to parent via fd=%d", fpm_globals.send_config_pipe[1]);
			zend_quiet_write(fpm_globals.send_config_pipe[1], &writeval, sizeof(writeval));
			close(fpm_globals.send_config_pipe[1]);
		}
		return FPM_EXIT_CONFIG;
	}
```
- 然后我们看看`fpm_init`  具体定义,`fpm_init` 会初始化非常多的东西 ,我有空会重新看一下这些初始化函数的具体内容
```
//  sapi\fpm\fpm\fpm.c
int fpm_init(int argc, char **argv, char *config, char *prefix, char *pid, int test_conf, int run_as_root, int force_daemon, int force_stderr) /* {{{ */
{
    ...

	if (0 > fpm_php_init_main()           ||
	    0 > fpm_stdio_init_main()         ||
	    0 > fpm_conf_init_main(test_conf, force_daemon) ||
	    0 > fpm_unix_init_main()          ||
	    0 > fpm_scoreboard_init_main()    ||
	    0 > fpm_pctl_init_main()          ||
	    0 > fpm_env_init_main()           ||
	    0 > fpm_signals_init_main()       ||
	    0 > fpm_children_init_main()      ||
	    0 > fpm_sockets_init_main()       ||
	    0 > fpm_worker_pool_init_main()   ||
	    0 > fpm_event_init_main()) {

           ...
}
```
> 4  调用`fpm_run`

- 这里估计会调用`fork` (linux有fork 函数但是Windows我就不知道了) 函数
- 具体定义
```
//  sapi\fpm\fpm\fpm.c

/*	children: return listening socket
	parent: never return */
int fpm_run(int *max_requests) /* {{{ */
{
	struct fpm_worker_pool_s *wp;

	/* create initial children in all pools */
	for (wp = fpm_worker_all_pools; wp; wp = wp->next) {
		int is_parent;

		is_parent = fpm_children_create_initial(wp);

		if (!is_parent) {
			goto run_child;
		}

		/* handle error */
		if (is_parent == 2) {
			fpm_pctl(FPM_PCTL_STATE_TERMINATING, FPM_PCTL_ACTION_SET);
			fpm_event_loop(1);
		}
	}

	/* run event loop forever */
	fpm_event_loop(0);

run_child: /* only workers reach this point */

	fpm_cleanups_run(FPM_CLEANUP_CHILD);

	*max_requests = fpm_globals.max_requests;
	return fpm_globals.listening_socket;
}
```
> 5 调用请求相关的初始化函数`fpm_init_request`
```
·	/* library is already initialized, now init our request */
	request = fpm_init_request(fcgi_fd);
```
> 6 调用完 `fpm_init_reques` 后，我们继续往main函数下走，走到我今天要讨论的 核心循环
```
		while (EXPECTED(fcgi_accept_request(request) >= 0)) {  // (1)  内部主要是accept 函数 
			char *primary_script = NULL;
			request_body_fd = -1;
			SG(server_context) = (void *) request;  // （2） 设置上下文
			init_request_info();

			fpm_request_info();

			/* request startup only after we've done all we can to
			 *            get path_translated */
			if (UNEXPECTED(php_request_startup() == FAILURE)) {
				fcgi_finish_request(request, 1);
				SG(server_context) = NULL;
				php_module_shutdown();
				return FPM_EXIT_SOFTWARE;
			}

			/* check if request_method has been sent.
			 * if not, it's certainly not an HTTP over fcgi request */
			if (UNEXPECTED(!SG(request_info).request_method)) {
				goto fastcgi_request_done;
			}

			if (UNEXPECTED(fpm_status_handle_request())) {
				goto fastcgi_request_done;
			}

			/* If path_translated is NULL, terminate here with a 404 */
			if (UNEXPECTED(!SG(request_info).path_translated)) {
				zend_try {
					zlog(ZLOG_DEBUG, "Primary script unknown");
					SG(sapi_headers).http_response_code = 404;
					PUTS("File not found.\n");
				} zend_catch {
				} zend_end_try();
				goto fastcgi_request_done;
			}

			if (UNEXPECTED(fpm_php_limit_extensions(SG(request_info).path_translated))) {
				SG(sapi_headers).http_response_code = 403;
				PUTS("Access denied.\n");
				goto fastcgi_request_done;
			}

			/*
			 * have to duplicate SG(request_info).path_translated to be able to log errrors
			 * php_fopen_primary_script seems to delete SG(request_info).path_translated on failure
			 */
			primary_script = estrdup(SG(request_info).path_translated);

			/* path_translated exists, we can continue ! */
			if (UNEXPECTED(php_fopen_primary_script(&file_handle) == FAILURE)) {  //（3） 转换路径
				zend_try {
					zlog(ZLOG_ERROR, "Unable to open primary script: %s (%s)", primary_script, strerror(errno));
					if (errno == EACCES) {
						SG(sapi_headers).http_response_code = 403;
						PUTS("Access denied.\n");
					} else {
						SG(sapi_headers).http_response_code = 404;
						PUTS("No input file specified.\n");
					}
				} zend_catch {
				} zend_end_try();
				/* we want to serve more requests if this is fastcgi
				 * so cleanup and continue, request shutdown is
				 * handled later */

				goto fastcgi_request_done;
			}

			fpm_request_executing();

			php_execute_script(&file_handle);  // （4）核心函数 执行脚本

fastcgi_request_done:
			if (EXPECTED(primary_script)) {
				efree(primary_script);
			}

			if (UNEXPECTED(request_body_fd != -1)) {
				close(request_body_fd);
			}
			request_body_fd = -2;

			if (UNEXPECTED(EG(exit_status) == 255)) {
				if (CGIG(error_header) && *CGIG(error_header)) {
					sapi_header_line ctr = {0};

					ctr.line = CGIG(error_header);
					ctr.line_len = strlen(CGIG(error_header));
					sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
				}
			}

			fpm_request_end();
			fpm_log_write(NULL);

			efree(SG(request_info).path_translated);
			SG(request_info).path_translated = NULL;

			php_request_shutdown((void *) 0);

			requests++;
			if (UNEXPECTED(max_requests && (requests == max_requests))) {
				fcgi_request_set_keep(request, 0);
				fcgi_finish_request(request, 0);
				break;
			}
			/* end of fastcgi loop */
		}
```
-  (1) 首先我们看到 while 循环里面有宏 `EXPECTED` 以及函数`fcgi_accept_request`
`while (EXPECTED(fcgi_accept_request(request) >= 0)) {  // (1)  内部主要是accept 函数 `

```
// 这里去掉了win32 相关的代码
int fcgi_accept_request(fcgi_request *req)
{
    ...
		int listen_socket = req->listen_socket;
	...
		req->hook.on_accept();

		FCGI_LOCK(req->listen_socket);
		req->fd = accept(listen_socket, (struct sockaddr *)&sa, &len);
		FCGI_UNLOCK(req->listen_socket);

	...
		if (fcgi_read_request(req)) {
			return req->fd;
	...
}
```
不考虑网络模型的话（epoll kpoll 等等），其实就是一个`accept` 函数 然后返回一个文件描述符`return req->fd;`

- （2） 调用完`fcgi_accept_request` 后就对全局变量赋值`fcgi_accept_request`
- （3） 路径转换 `php_fopen_primary_script`
```
			/* path_translated exists, we can continue ! */
			if (UNEXPECTED(php_fopen_primary_script(&file_handle) == FAILURE)) {
```
- （4） 核心函数 ，执行脚本

---
分割线会详细讲述

> `zend_file_handle` 结构 ，我们很容易可以看出是一个描述文件的结构其实和文件描述符是类似的
```
typedef struct _zend_file_handle {
	union {
		int           fd;
		FILE          *fp;
		zend_stream   stream;
	} handle;
	const char        *filename;
	zend_string       *opened_path;
	zend_stream_type  type;
	zend_bool free_filename;
} zend_file_handle;
```
> 执行函数`php_execute_script`，会调用 `zend_execute_scripts`
```
PHPAPI int php_execute_script(zend_file_handle *primary_file){
	...
	retval = (zend_execute_scripts(ZEND_REQUIRE, NULL, 3, prepend_file_p, primary_file, append_file_p) == SUCCESS);
	...

}
```
然后我们看一下`zend_execute_scripts`定义

> `zend_execute_scripts` 定义
```
ZEND_API int zend_execute_scripts(int type, zval *retval, int file_count, ...) /* {{{ */
{
	...
		op_array = zend_compile_file(file_handle, type); // 编译获得opcode
	...
		if (op_array) {
			zend_execute(op_array, retval);    // 执行opcode 的函数
			zend_exception_restore();
			zend_try_exception_handler();
			if (EG(exception)) {
				zend_exception_error(EG(exception), E_ERROR);
			}
			...
		} 	
	...
}
```

> `zend_compile_file` 是一个函数指针，指向 他等于 `compile_file`

然后我们看一下函数`compile_file` 的定义，其中`compile_file` 会调用函数`zend_compile`
```
ZEND_API zend_op_array *compile_file(zend_file_handle *file_handle, int type)
{
	zend_lex_state original_lex_state;
	zend_op_array *op_array = NULL;
	zend_save_lexical_state(&original_lex_state);

	if (open_file_for_scanning(file_handle)==FAILURE) {
		if (type==ZEND_REQUIRE) {
			zend_message_dispatcher(ZMSG_FAILED_REQUIRE_FOPEN, file_handle->filename);
			zend_bailout();
		} else {
			zend_message_dispatcher(ZMSG_FAILED_INCLUDE_FOPEN, file_handle->filename);
		}
	} else {
		op_array = zend_compile(ZEND_USER_FUNCTION);
	}

	zend_restore_lexical_state(&original_lex_state);
	return op_array;
}
```

> 核心函数 `zend_compile`  是由 lex/yacc  或者bison 这类工具去生成相应的c 源文件，所以你会在 `zend_language_scanner.c` 也会看到`zend_compile` ，但是其实是在`php-7.1.8-src\Zend\zend_language_scanner.l` 文件下生成的

```
// php-7.1.8-src\Zend\zend_language_scanner.l 下的`zend_compile` 函数
static zend_op_array *zend_compile(int type)   // 核心函数 貌似没有注释
{
	zend_op_array *op_array = NULL;
	...
	if (!zendparse()) {         // 重点函数  yyparse 生成语法树
		...
		zend_op_array *original_active_op_array = CG(active_op_array);  // 保存现在已经有了的op_array

		op_array = emalloc(sizeof(zend_op_array));         
		init_op_array(op_array, type, INITIAL_OP_ARRAY_SIZE);  // 初始化op_array
		CG(active_op_array) = op_array;      // CG(ast) 指向 op_array
		...
		zend_compile_top_stmt(CG(ast));     // 生成opcode
		CG(zend_lineno) = last_lineno;       // 记录行号
		zend_emit_final_return(type == ZEND_USER_FUNCTION); // 自定义函数插入return opcode
		...
		CG(active_op_array) = original_active_op_array;   
	}
	...
	return op_array;
}
```
`zend_compile 函数主要包括以下几个内容：`

- `zendparse` 词法分析语法分析并生成语法树语法树会保存在 CG(ast) 上面
	- CG = compile global
	- ast = Abstract Syntax Tree (抽象语法树)
- `zend_compile_top_stmt`  生成opcode
- `zend_emit_final_return` 插入`return` 这个 opcode


