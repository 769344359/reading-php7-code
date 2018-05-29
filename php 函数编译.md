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
- 断点
```
Breakpoint 1, zend_ast_create_decl (kind=66, flags=0, start_lineno=2, doc_comment=0x0, name=0x7ffff6268be0, child0=0x7ffff6281060, child1=0x0, child2=0x7ffff6281090, child3=0x0)
    at /home/vagrant/php-7.2.5/Zend/zend_ast.c:81
81              ast = zend_ast_alloc(sizeof(zend_ast_decl));
```
然后打印出名字
```
(gdb) p  (char*)name->val
$3 = 0x7ffff6268bf8 "sayHello"
```


