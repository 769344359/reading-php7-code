[opcode](https://en.wikipedia.org/wiki/Opcode)

[operand](https://en.wikipedia.org/wiki/Operand)

上面是维基百科的操作数和操作码的解释

编译相关主要包括这几个结构
- `zend_op`
- `znode_op`
- `zend_op_array`
- `zend_execute_data`

> zend_op

下面先看`zend_op`

```c
// php-7.1.8-src\Zend\zend_compile.h
typedef struct _zend_op zend_op;
struct _zend_op {
	const void *handler;
	znode_op op1;
	znode_op op2;
	znode_op result;
	uint32_t extended_value;
	uint32_t lineno;
	zend_uchar opcode;
	zend_uchar op1_type;
	zend_uchar op2_type;
	zend_uchar result_type;
};
```
zend_op 结构主要包括三个成员
- op1 
- op2
- result

> znode_op

这个结构主要用来储存相对的偏移
```c
typedef union _znode_op {
	uint32_t      constant;
	uint32_t      var;
	uint32_t      num;
	uint32_t      opline_num; /*  Needs to be signed */
#if ZEND_USE_ABS_JMP_ADDR
	zend_op       *jmp_addr;
#else
	uint32_t      jmp_offset;
#endif
#if ZEND_USE_ABS_CONST_ADDR
	zval          *zv;
#endif
} znode_op;
```

```c
struct _zend_op_array {
	/* Common elements */
	zend_uchar type;
	zend_uchar arg_flags[3]; /* bitset of arg_info.pass_by_reference */
	uint32_t fn_flags;
	zend_string *function_name;
	zend_class_entry *scope;
	zend_function *prototype;
	uint32_t num_args;
	uint32_t required_num_args;
	zend_arg_info *arg_info;
	/* END of common elements */

	uint32_t *refcount;

	uint32_t last;
	zend_op *opcodes;   //指针

	int last_var;
	uint32_t T;
	zend_string **vars;

	int last_live_range;
	int last_try_catch;
	zend_live_range *live_range;
	zend_try_catch_element *try_catch_array;

	/* static variables support */
	HashTable *static_variables;

	zend_string *filename;
	uint32_t line_start;
	uint32_t line_end;
	zend_string *doc_comment;
	uint32_t early_binding; /* the linked list of delayed declarations */

	int last_literal;
	zval *literals;

	int  cache_size;
	void **run_time_cache;

	void *reserved[ZEND_MAX_RESERVED_RESOURCES];
};
```
`zend_execute_data`
```c
struct _zend_execute_data {
	const zend_op       *opline;           /* executed opline                */
	zend_execute_data   *call;             /* current call                   */
	zval                *return_value;
	zend_function       *func;             /* executed function              */
	zval                 This;             /* this + call_info + num_args    */
	zend_execute_data   *prev_execute_data;
	zend_array          *symbol_table;
#if ZEND_EX_USE_RUN_TIME_CACHE
	void               **run_time_cache;   /* cache op_array->run_time_cache */
#endif
#if ZEND_EX_USE_LITERALS
	zval                *literals;         /* cache op_array->literals       */
#endif
};
```
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

```c
struct _zend_executor_globals {
	zval uninitialized_zval;
	zval error_zval;

	/* symbol table cache */
	zend_array *symtable_cache[SYMTABLE_CACHE_SIZE];
	zend_array **symtable_cache_limit;
	zend_array **symtable_cache_ptr;

	zend_array symbol_table;		/* main symbol table */

	HashTable included_files;	/* files already included */

	JMP_BUF *bailout;

	int error_reporting;
	int exit_status;

	HashTable *function_table;	/* function symbol table */
	HashTable *class_table;		/* class table */
	HashTable *zend_constants;	/* constants table */

	zval          *vm_stack_top;
	zval          *vm_stack_end;
	zend_vm_stack  vm_stack;

	struct _zend_execute_data *current_execute_data;
	zend_class_entry *fake_scope; /* used to avoid checks accessing properties */

	zend_long precision;

	int ticks_count;

	HashTable *in_autoload;
	zend_function *autoload_func;
	zend_bool full_tables_cleanup;

	/* for extended information support */
	zend_bool no_extensions;

	zend_bool vm_interrupt;
	zend_bool timed_out;
	zend_long hard_timeout;

#ifdef ZEND_WIN32
	OSVERSIONINFOEX windows_version_info;
#endif

	HashTable regular_list;
	HashTable persistent_list;

	int user_error_handler_error_reporting;
	zval user_error_handler;
	zval user_exception_handler;
	zend_stack user_error_handlers_error_reporting;
	zend_stack user_error_handlers;
	zend_stack user_exception_handlers;

	zend_error_handling_t  error_handling;
	zend_class_entry      *exception_class;

	/* timeout support */
	zend_long timeout_seconds;

	int lambda_count;

	HashTable *ini_directives;
	HashTable *modified_ini_directives;
	zend_ini_entry *error_reporting_ini_entry;

	zend_objects_store objects_store;
	zend_object *exception, *prev_exception;
	const zend_op *opline_before_exception;
	zend_op exception_op[3];

	struct _zend_module_entry *current_module;

	zend_bool active;
	zend_bool valid_symbol_table;

	zend_long assertions;

	uint32_t           ht_iterators_count;     /* number of allocatd slots */
	uint32_t           ht_iterators_used;      /* number of used slots */
	HashTableIterator *ht_iterators;
	HashTableIterator  ht_iterators_slots[16];

	void *saved_fpu_cw_ptr;
#if XPFPA_HAVE_CW
	XPFPA_CW_DATATYPE saved_fpu_cw;
#endif

	zend_function trampoline;
	zend_op       call_trampoline_op;

	void *reserved[ZEND_MAX_RESERVED_RESOURCES];
};
```

`EG` 宏主要包括这个结构， 其中包括编译好的函数

