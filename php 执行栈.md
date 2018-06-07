```c
ZEND_API void zend_execute(zend_op_array *op_array, zval *return_value)
{
	zend_execute_data *execute_data;

	if (EG(exception) != NULL) {
		return;
	}

	execute_data = zend_vm_stack_push_call_frame(ZEND_CALL_TOP_CODE | ZEND_CALL_HAS_SYMBOL_TABLE,
		(zend_function*)op_array, 0, zend_get_called_scope(EG(current_execute_data)), zend_get_this_object(EG(current_execute_data)));
	if (EG(current_execute_data)) {
		execute_data->symbol_table = zend_rebuild_symbol_table();
	} else {
		execute_data->symbol_table = &EG(symbol_table);
	}
	EX(prev_execute_data) = EG(current_execute_data);
	i_init_execute_data(execute_data, op_array, return_value);
	zend_execute_ex(execute_data);
	zend_vm_stack_free_call_frame(execute_data);
}

```

在编译完opcode后,会调用 zend_execute 函数并进入执行阶段.

执行阶段会先分配相应的栈

> 第一步 计算栈的大小
- `zend_vm_stack_push_call_frame`

计算栈大小会调用内联函数 `zend_vm_calc_used_stack`,该函数会计算出所需要的内存
```c
static zend_always_inline zend_execute_data *zend_vm_stack_push_call_frame(uint32_t call_info, zend_function *func, uint32_t num_args, zend_class_entry *called_scope, zend_object *object)
{
	uint32_t used_stack = zend_vm_calc_used_stack(num_args, func);  // 计算运行栈的大小

	return zend_vm_stack_push_call_frame_ex(used_stack, call_info,   // 实际分配内存会委托给 这个函数
		func, num_args, called_scope, object);
}
```
- 栈大小的计算方法
