# zend_compile
先介绍php 的抽象语法树的结构
```
struct _zend_ast {
	zend_ast_kind kind; /* Type of the node (ZEND_AST_* enum constant) */
	zend_ast_attr attr; /* Additional attribute, use depending on node type */
	uint32_t lineno;    /* Line number */
	zend_ast *child[1]; /* Array of children (using struct hack) */
};
```
