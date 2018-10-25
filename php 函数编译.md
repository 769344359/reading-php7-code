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
```c
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


(gdb) p (char *)decl->name->val
$76 = 0x7ffff6268bf8 "sayHello"
```

```
static void zend_begin_func_decl(znode *result, zend_op_array *op_array, zend_ast_decl *decl) /* {{{ */
{
	zend_ast *params_ast = decl->child[0];
	zend_string *unqualified_name, *name, *lcname, *key;
	zend_op *opline;

	unqualified_name = decl->name;   // 获得函数名   sayHello
	op_array->function_name = name = zend_prefix_with_ns(unqualified_name);   // 加上命名空间 拼装成完全限定名
	lcname = zend_string_tolower(name);

	if (FC(imports_function)) {
		zend_string *import_name = zend_hash_find_ptr_lc(
			FC(imports_function), ZSTR_VAL(unqualified_name), ZSTR_LEN(unqualified_name));
		if (import_name && !zend_string_equals_ci(lcname, import_name)) {
			zend_error_noreturn(E_COMPILE_ERROR, "Cannot declare function %s "
				"because the name is already in use", ZSTR_VAL(name));
		}
	}

	if (zend_string_equals_literal(lcname, ZEND_AUTOLOAD_FUNC_NAME)
		&& zend_ast_get_list(params_ast)->children != 1
	) {
		zend_error_noreturn(E_COMPILE_ERROR, "%s() must take exactly 1 argument",
			ZEND_AUTOLOAD_FUNC_NAME);
	}

	key = zend_build_runtime_definition_key(lcname, decl->lex_pos);
	zend_hash_update_ptr(CG(function_table), key, op_array);  // 添加函数 到CG

	if (op_array->fn_flags & ZEND_ACC_CLOSURE) {    // 闭包处理
		opline = zend_emit_op_tmp(result, ZEND_DECLARE_LAMBDA_FUNCTION, NULL, NULL);
		opline->op1_type = IS_CONST;
		LITERAL_STR(opline->op1, key);
	} else {            
		opline = get_next_op(CG(active_op_array));
		opline->opcode = ZEND_DECLARE_FUNCTION;
		opline->op1_type = IS_CONST;
		LITERAL_STR(opline->op1, zend_string_copy(lcname));
		/* RTD key is placed after lcname literal in op1 */
		zend_add_literal_string(CG(active_op_array), &key);
		SET_UNUSED(opline->op2);
	}

	zend_string_release(lcname);
}
```

> 参数

```c
/* arg_info for user functions */
typedef struct _zend_arg_info {
	zend_string *name;
	zend_string *class_name;
	zend_uchar type_hint;
	zend_uchar pass_by_reference;
	zend_bool allow_null;
	zend_bool is_variadic;
} zend_arg_info;
```

### 例子

> php 源码
```php
<?php
function sayHello(String $tem,$b){
	$a = 1 +2+$tem+$b;
}
$tem = 2;
sayHello($tem);
$b  = 1+ 3;
```
> 编译语法树
```c
Breakpoint 1, zend_compile_params (ast=0x7ffff6281098, return_type_ast=0x0) at /home/vagrant/php-7.2.5/Zend/zend_compile.c:5416
5416            zend_ast_list *list = zend_ast_get_list(ast);

(gdb) p  (char *)((zend_ast_zval *)list->child[0]->child[1])->val.value.str.val
$74 = 0x7ffff6202718 "tem"
(gdb) p  (char *)((zend_ast_zval *)list->child[1]->child[1])->val.value.str.val
$77 = 0x7ffff6202758 "b"
(gdb) p  (char *)((zend_ast_zval *)list->child[0]->child[0])->val.value.str.val
$88 = 0x7ffff6202718 "String"
```

> 编译参数列表

```
void zend_compile_params(zend_ast *ast, zend_ast *return_type_ast) /* {{{ */
{
	zend_ast_list *list = zend_ast_get_list(ast);
	uint32_t i;
	zend_op_array *op_array = CG(active_op_array);
	zend_arg_info *arg_infos;
	
	if (return_type_ast) {      // 如果是有返回值 
		/* Use op_array->arg_info[-1] for return type */
		arg_infos = safe_emalloc(sizeof(zend_arg_info), list->children + 1, 0);
		arg_infos->name = NULL;
		arg_infos->pass_by_reference = (op_array->fn_flags & ZEND_ACC_RETURN_REFERENCE) != 0;
		arg_infos->is_variadic = 0;
		arg_infos->type_hint = 0;
		arg_infos->allow_null = 0;
		arg_infos->class_name = NULL;

		if (return_type_ast->attr & ZEND_TYPE_NULLABLE) {
			arg_infos->allow_null = 1;
			return_type_ast->attr &= ~ZEND_TYPE_NULLABLE;
		}

		zend_compile_typename(return_type_ast, arg_infos);

		if (arg_infos->type_hint == IS_VOID && arg_infos->allow_null) {
			zend_error_noreturn(E_COMPILE_ERROR, "Void type cannot be nullable");
		}

		arg_infos++;
		op_array->fn_flags |= ZEND_ACC_HAS_RETURN_TYPE;
	} else { 
		if (list->children == 0) {    // 没有参数
			return;
		}
		arg_infos = safe_emalloc(sizeof(zend_arg_info), list->children, 0);   // 有参数
	}

	for (i = 0; i < list->children; ++i) {      // 遍历添加 arg_info
		zend_ast *param_ast = list->child[i];
		zend_ast *type_ast = param_ast->child[0];
		zend_ast *var_ast = param_ast->child[1];
		zend_ast *default_ast = param_ast->child[2];
		zend_string *name = zend_ast_get_str(var_ast);
		zend_bool is_ref = (param_ast->attr & ZEND_PARAM_REF) != 0;
		zend_bool is_variadic = (param_ast->attr & ZEND_PARAM_VARIADIC) != 0;

		znode var_node, default_node;
		zend_uchar opcode;
		zend_op *opline;
		zend_arg_info *arg_info;

		if (zend_is_auto_global(name)) {
			zend_error_noreturn(E_COMPILE_ERROR, "Cannot re-assign auto-global variable %s",
				ZSTR_VAL(name));
		}

		var_node.op_type = IS_CV;
		var_node.u.op.var = lookup_cv(CG(active_op_array), zend_string_copy(name));

		if (EX_VAR_TO_NUM(var_node.u.op.var) != i) {
			zend_error_noreturn(E_COMPILE_ERROR, "Redefinition of parameter $%s",
				ZSTR_VAL(name));
		} else if (zend_string_equals_literal(name, "this")) {
			zend_error_noreturn(E_COMPILE_ERROR, "Cannot use $this as parameter");
		}

		if (op_array->fn_flags & ZEND_ACC_VARIADIC) {
			zend_error_noreturn(E_COMPILE_ERROR, "Only the last parameter can be variadic");
		}

		if (is_variadic) {
			opcode = ZEND_RECV_VARIADIC;
			default_node.op_type = IS_UNUSED;
			op_array->fn_flags |= ZEND_ACC_VARIADIC;

			if (default_ast) {
				zend_error_noreturn(E_COMPILE_ERROR,
					"Variadic parameter cannot have a default value");
			}
		} else if (default_ast) {
			/* we cannot substitute constants here or it will break ReflectionParameter::getDefaultValueConstantName() and ReflectionParameter::isDefaultValueConstant() */
			uint32_t cops = CG(compiler_options);
			CG(compiler_options) |= ZEND_COMPILE_NO_CONSTANT_SUBSTITUTION | ZEND_COMPILE_NO_PERSISTENT_CONSTANT_SUBSTITUTION;
			opcode = ZEND_RECV_INIT;
			default_node.op_type = IS_CONST;
			zend_const_expr_to_zval(&default_node.u.constant, default_ast);
			CG(compiler_options) = cops;
		} else {
			opcode = ZEND_RECV;
			default_node.op_type = IS_UNUSED;
			op_array->required_num_args = i + 1;
		}

		opline = zend_emit_op(NULL, opcode, NULL, &default_node);
		SET_NODE(opline->result, &var_node);
		opline->op1.num = i + 1;

		arg_info = &arg_infos[i];
		arg_info->name = zend_string_copy(name);
		arg_info->pass_by_reference = is_ref;
		arg_info->is_variadic = is_variadic;
		arg_info->type_hint = 0;
		arg_info->allow_null = 1;
		arg_info->class_name = NULL;

		if (type_ast) {
			zend_bool has_null_default = default_ast
				&& (Z_TYPE(default_node.u.constant) == IS_NULL
					|| (Z_TYPE(default_node.u.constant) == IS_CONSTANT
						&& strcasecmp(Z_STRVAL(default_node.u.constant), "NULL") == 0));
			zend_bool is_explicitly_nullable = (type_ast->attr & ZEND_TYPE_NULLABLE) == ZEND_TYPE_NULLABLE;

			op_array->fn_flags |= ZEND_ACC_HAS_TYPE_HINTS;
			arg_info->allow_null = has_null_default || is_explicitly_nullable;

			type_ast->attr &= ~ZEND_TYPE_NULLABLE;
			zend_compile_typename(type_ast, arg_info);

			if (arg_info->type_hint == IS_VOID) {
				zend_error_noreturn(E_COMPILE_ERROR, "void cannot be used as a parameter type");
			}

			if (type_ast->kind == ZEND_AST_TYPE) {
				if (arg_info->type_hint == IS_ARRAY) {
					if (default_ast && !has_null_default
						&& Z_TYPE(default_node.u.constant) != IS_ARRAY
						&& !Z_CONSTANT(default_node.u.constant)
					) {
						zend_error_noreturn(E_COMPILE_ERROR, "Default value for parameters "
							"with array type can only be an array or NULL");
					}
				} else if (arg_info->type_hint == IS_CALLABLE && default_ast) {
					if (!has_null_default && !Z_CONSTANT(default_node.u.constant)) {
						zend_error_noreturn(E_COMPILE_ERROR, "Default value for parameters "
							"with callable type can only be NULL");
					}
				}
			} else {
				if (default_ast && !has_null_default && !Z_CONSTANT(default_node.u.constant)) {
					if (arg_info->class_name) {
						zend_error_noreturn(E_COMPILE_ERROR, "Default value for parameters "
							"with a class type can only be NULL");
					} else switch (arg_info->type_hint) {
						case IS_DOUBLE:
							if (Z_TYPE(default_node.u.constant) != IS_DOUBLE && Z_TYPE(default_node.u.constant) != IS_LONG) {
								zend_error_noreturn(E_COMPILE_ERROR, "Default value for parameters "
									"with a float type can only be float, integer, or NULL");
							}
							break;
						
						case IS_ITERABLE:
							if (Z_TYPE(default_node.u.constant) != IS_ARRAY) {
								zend_error_noreturn(E_COMPILE_ERROR, "Default value for parameters "
									"with iterable type can only be an array or NULL");
							}
							break;
							
						default:
							if (!ZEND_SAME_FAKE_TYPE(arg_info->type_hint, Z_TYPE(default_node.u.constant))) {
								zend_error_noreturn(E_COMPILE_ERROR, "Default value for parameters "
									"with a %s type can only be %s or NULL",
									zend_get_type_by_const(arg_info->type_hint), zend_get_type_by_const(arg_info->type_hint));
							}
							break;
					}
				}
			}

			/* Allocate cache slot to speed-up run-time class resolution */
			if (opline->opcode == ZEND_RECV_INIT) {
				if (arg_info->class_name) {
					zend_alloc_cache_slot(opline->op2.constant);
				} else {
					Z_CACHE_SLOT(op_array->literals[opline->op2.constant]) = -1;
				}
			} else {
				if (arg_info->class_name) {
					opline->op2.num = op_array->cache_size;
					op_array->cache_size += sizeof(void*);
				} else {
					opline->op2.num = -1;
				}
			}
		} else {
			if (opline->opcode == ZEND_RECV_INIT) {
				Z_CACHE_SLOT(op_array->literals[opline->op2.constant]) = -1;
			} else {
				opline->op2.num = -1;
			}
		}	
	}

	/* These are assigned at the end to avoid unitialized memory in case of an error */
	op_array->num_args = list->children;
	op_array->arg_info = arg_infos;

	/* Don't count the variadic argument */
	if (op_array->fn_flags & ZEND_ACC_VARIADIC) {
		op_array->num_args--;
	}
	zend_set_function_arg_flags((zend_function*)op_array);
}
/* }}} */
```

> 内置类型 
```
(gdb) p builtin_types
$23 = {{name = 0x123c667 "int", name_len = 3, type = 4 '\004'}, {name = 0x123c661 "float", name_len = 5, type = 5 '\005'}, {name = 0x123c683 "string", name_len = 6, type = 6 '\006'}, {
    name = 0x123c656 "bool", name_len = 4, type = 13 '\r'}, {name = 0x123c68f "void", name_len = 4, type = 18 '\022'}, {name = 0x123c694 "iterable", name_len = 8, type = 19 '\023'}, {
    name = 0x123c69d "object", name_len = 6, type = 8 '\b'}, {name = 0x0, name_len = 0, type = 0 '\000'}}
```
