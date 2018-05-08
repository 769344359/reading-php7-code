[opcode](https://en.wikipedia.org/wiki/Opcode)

[operand](https://en.wikipedia.org/wiki/Operand)

上面是维基百科的操作数和操作码的解释

编译相关主要包括这几个结构
- `zend_op`
- `znode_op`
- `zend_op_array`
- `zend_execute_data`

下面先看`zend_op`

```
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

