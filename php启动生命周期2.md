# zend_compile
先介绍php 的抽象语法树的结构,分别是 `zend_ast` , `zend_ast_list` 以及 `zend_ast_zval` 
```
struct _zend_ast {
	zend_ast_kind kind; /* Type of the node (ZEND_AST_* enum constant) */
	zend_ast_attr attr; /* Additional attribute, use depending on node type */
	uint32_t lineno;    /* Line number */
	zend_ast *child[1]; /* Array of children (using struct hack) */
};

/* Same as zend_ast, but with children count, which is updated dynamically */
typedef struct _zend_ast_list {
	zend_ast_kind kind;
	zend_ast_attr attr;
	uint32_t lineno;
	uint32_t children;
	zend_ast *child[1];
} zend_ast_list;

/* Lineno is stored in val.u2.lineno */
typedef struct _zend_ast_zval {
	zend_ast_kind kind;
	zend_ast_attr attr;
	zval val;
} zend_ast_zval;
```
<<<<<<< HEAD
这三个struct 的前两个元素都是一样的，而zend_ast_zval 则专门多了一个属性zval 来储存值。zend_ast_list 则比zend_ast 多了children 来记录list个数

其中调用顺序如下
```
|
|---zendparse
|---zend_compile_top_stmt
|  |---zend_compile_stmt
|---zend_emit_final_return
|---pass_two  
```
其中`zendparse`　生成抽象语法树，`ｚend_compile_top_stmt`　根据语法树生成opcode.`pass_two`　则是给相应的opcode 添加相应的handler
***
```
void zend_compile_stmt(zend_ast *ast){ /* {{{ */
		switch (ast->kind) {
			...
			break;
		case ZEND_AST_RETURN:
			zend_compile_return(ast);
			break;
		case ZEND_AST_ECHO:
			zend_compile_echo(ast);
			break;
			...
		default:
		{
			znode result;
			zend_compile_expr(&result, ast);
			zend_do_free(&result);
		}
		...
			
}
```
深度优先便利语法树，然后生成opcode
=======
抽象语法树由上面 三个基本struct 组成，其中

- `zend_ast`  是一个基本节点
- 而`zend_ast_list` 比 `zend_ast` 多一个 children 的属性
- `zend_ast_zval` 是抽象语法树的叶子
>>>>>>> 56204ce134d40e221ebcf9c92d8045ddec30f8e3
