首先是函数的编译
函数的结构是如下
这是个`union` 
`common` 算是 内部函数和 用户自定义函数共同的部分

用户自定义函数会使用 `zend_op_array` 而内部函数或者扩展会使用 `zend_internal_function` 
```c
union _zend_function {
	zend_uchar type;	/* MUST be the first element of this struct! */
	uint32_t   quick_arg_flags;

	struct {
		zend_uchar type;  /* never used */
		zend_uchar arg_flags[3]; /* bitset of arg_info.pass_by_reference */
		uint32_t fn_flags;
		zend_string *function_name;
		zend_class_entry *scope;
		union _zend_function *prototype;
		uint32_t num_args;
		uint32_t required_num_args;
		zend_arg_info *arg_info;
	} common;

	zend_op_array op_array;
	zend_internal_function internal_function;
};
```

> 语法分析

php 的语法分析是使用bison ，代码如下
```
function_declaration_statement:
	function returns_ref T_STRING backup_doc_comment '(' parameter_list ')' return_type
	backup_fn_flags '{' inner_statement_list '}' backup_fn_flags
		{ $$ = zend_ast_create_decl(ZEND_AST_FUNC_DECL, $2 | $13, $1, $4,
		      zend_ast_get_str($3), $6, NULL, $11, $8); CG(extra_fn_flags) = $9; }
;
```

```c
//  php-7.1.8-src\Zend\zend_ast.h
#define ZEND_AST_SPECIAL_SHIFT      6
enum _zend_ast_kind {
	/* special nodes */
	ZEND_AST_ZVAL = 1 << ZEND_AST_SPECIAL_SHIFT,   // 64    
	ZEND_AST_ZNODE,                                // 65

	/* declaration nodes */
	ZEND_AST_FUNC_DECL,                            // 66
	...
	}
```
> 例子
```php
<?php
function sayHello(){
	echo 'hi';
}
sayHello();
```
- 堆栈
```c
(gdb) bt
#0  zend_ast_create_decl (kind=66, flags=0, start_lineno=2, doc_comment=0x0, name=0x7ffff6268be0, child0=0x7ffff6281060, child1=0x0, child2=0x7ffff6281090, child3=0x0)
    at /home/vagrant/php-7.2.5/Zend/zend_ast.c:81
#1  0x00000000009a3173 in zendparse () at /home/vagrant/php-7.2.5/Zend/zend_language_parser.c:4843
#2  0x00000000009a8f82 in zend_compile (type=2) at Zend/zend_language_scanner.l:585
#3  0x00000000009a91cd in compile_file (file_handle=0x7fffffffd300, type=8) at Zend/zend_language_scanner.l:635
#4  0x000000000077c7d8 in phar_compile_file (file_handle=0x7fffffffd300, type=8) at /home/vagrant/php-7.2.5/ext/phar/phar.c:3320
#5  0x0000000000a06883 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/vagrant/php-7.2.5/Zend/zend.c:1490
#6  0x000000000096ef77 in php_execute_script (primary_file=0x7fffffffd300) at /home/vagrant/php-7.2.5/main/main.c:2590
#7  0x0000000000af0dcb in do_cli (argc=2, argv=0x16319f0) at /home/vagrant/php-7.2.5/sapi/cli/php_cli.c:1011
#8  0x0000000000af1f7d in main (argc=2, argv=0x16319f0) at /home/vagrant/php-7.2.5/sapi/cli/php_cli.c:1404
```
- 断点
```c
Breakpoint 1, zend_ast_create_decl (kind=66, flags=0, start_lineno=2, doc_comment=0x0, name=0x7ffff6268be0, child0=0x7ffff6281060, child1=0x0, child2=0x7ffff6281090, child3=0x0)
    at /home/vagrant/php-7.2.5/Zend/zend_ast.c:81
81              ast = zend_ast_alloc(sizeof(zend_ast_decl));
```
然后打印出名字
```c
(gdb) p  (char*)name->val
$3 = 0x7ffff6268bf8 "sayHello"
```

> 编译完成并放进 `GC(function_table)`

编译出语法树并生成`op_array` 后，会将`op_array` 放进 `GC(function_table)` 里面

- 堆栈
```
(gdb) bt
#0  zend_begin_func_decl (result=0x0, op_array=0x7ffff620d018, decl=0x7ffff6281118) at /home/vagrant/php-7.2.5/Zend/zend_compile.c:5856
#1  0x00000000009df554 in zend_compile_func_decl (result=0x0, ast=0x7ffff6281118) at /home/vagrant/php-7.2.5/Zend/zend_compile.c:5934
#2  0x00000000009e60e2 in zend_compile_stmt (ast=0x7ffff6281118) at /home/vagrant/php-7.2.5/Zend/zend_compile.c:8154
#3  0x00000000009e5d5a in zend_compile_top_stmt (ast=0x7ffff6281118) at /home/vagrant/php-7.2.5/Zend/zend_compile.c:8072
#4  0x00000000009e5d3c in zend_compile_top_stmt (ast=0x7ffff6281018) at /home/vagrant/php-7.2.5/Zend/zend_compile.c:8067
#5  0x00000000009a9062 in zend_compile (type=2) at Zend/zend_language_scanner.l:601
#6  0x00000000009a91cd in compile_file (file_handle=0x7fffffffd300, type=8) at Zend/zend_language_scanner.l:635
#7  0x000000000077c7d8 in phar_compile_file (file_handle=0x7fffffffd300, type=8) at /home/vagrant/php-7.2.5/ext/phar/phar.c:3320
#8  0x0000000000a06883 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/vagrant/php-7.2.5/Zend/zend.c:1490
#9  0x000000000096ef77 in php_execute_script (primary_file=0x7fffffffd300) at /home/vagrant/php-7.2.5/main/main.c:2590
#10 0x0000000000af0dcb in do_cli (argc=2, argv=0x16319f0) at /home/vagrant/php-7.2.5/sapi/cli/php_cli.c:1011
#11 0x0000000000af1f7d in main (argc=2, argv=0x16319f0) at /home/vagrant/php-7.2.5/sapi/cli/php_cli.c:1404
```

