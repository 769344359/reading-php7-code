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
