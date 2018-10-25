```c
ZEND_API int ZEND_FASTCALL add_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	zval op1_copy, op2_copy;
	int converted = 0;

	while (1) {
		switch (TYPE_PAIR(Z_TYPE_P(op1), Z_TYPE_P(op2))) {
			case TYPE_PAIR(IS_LONG, IS_LONG):
				fast_long_add_function(result, op1, op2);
				return SUCCESS;
			case TYPE_PAIR(IS_LONG, IS_DOUBLE):
				ZVAL_DOUBLE(result, ((double)Z_LVAL_P(op1)) + Z_DVAL_P(op2));
				return SUCCESS;

			case TYPE_PAIR(IS_DOUBLE, IS_LONG):
				ZVAL_DOUBLE(result, Z_DVAL_P(op1) + ((double)Z_LVAL_P(op2)));
				return SUCCESS;

			case TYPE_PAIR(IS_DOUBLE, IS_DOUBLE):
				ZVAL_DOUBLE(result, Z_DVAL_P(op1) + Z_DVAL_P(op2));
				return SUCCESS;

			case TYPE_PAIR(IS_ARRAY, IS_ARRAY):
				if ((result == op1) && (result == op2)) {
					/* $a += $a */
					return SUCCESS;
				}
				if (result != op1) {
					ZVAL_DUP(result, op1);
				}
				zend_hash_merge(Z_ARRVAL_P(result), Z_ARRVAL_P(op2), zval_add_ref, 0);
				return SUCCESS;

			default:
				if (Z_ISREF_P(op1)) {
					op1 = Z_REFVAL_P(op1);
				} else if (Z_ISREF_P(op2)) {
					op2 = Z_REFVAL_P(op2);
				} else if (!converted) {
					ZEND_TRY_BINARY_OBJECT_OPERATION(ZEND_ADD, add_function);

					if (EXPECTED(op1 != op2)) {
						zendi_convert_scalar_to_number(op1, op1_copy, result, 0);
						zendi_convert_scalar_to_number(op2, op2_copy, result, 0);
					} else {
						zendi_convert_scalar_to_number(op1, op1_copy, result, 0);
						op2 = op1;
					}
					converted = 1;
				} else {
					zend_throw_error(NULL, "Unsupported operand types");
					return FAILURE; /* unknown datatype */
				}
		}
	}
}
/* }}} */
```
