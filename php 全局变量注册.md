post 和get 等全局变量主要在这里解析出来
```
SAPI_API SAPI_TREAT_DATA_FUNC(php_default_treat_data)
{
	char *res = NULL, *var, *val, *separator = NULL;
	const char *c_var;
	zval array;
	int free_buffer = 0;
	char *strtok_buf = NULL;
	zend_long count = 0;
        ...
	var = php_strtok_r(res, separator, &strtok_buf);
	while (var) {
		val = strchr(var, '=');

		if (arg == PARSE_COOKIE) {
			/* Remove leading spaces from cookie names, needed for multi-cookie header where ; can be followed by a space */
			while (isspace(*var)) {
				var++;
			}
			if (var == val || *var == '\0') {
				goto next_cookie;
			}
		}

		if (++count > PG(max_input_vars)) {
			php_error_docref(NULL, E_WARNING, "Input variables exceeded " ZEND_LONG_FMT ". To increase the limit change max_input_vars in php.ini.", PG(max_input_vars));
			break;
		}

		if (val) { /* have a value */
			size_t val_len;
			size_t new_val_len;

			*val++ = '\0';
			php_url_decode(var, strlen(var));
			val_len = php_url_decode(val, strlen(val));
			val = estrndup(val, val_len);
			if (sapi_module.input_filter(arg, var, &val, val_len, &new_val_len)) {
				php_register_variable_safe(var, val, new_val_len, &array);
			}
			efree(val);
		} else {
			size_t val_len;
			size_t new_val_len;

			php_url_decode(var, strlen(var));
			val_len = 0;
			val = estrndup("", val_len);
			if (sapi_module.input_filter(arg, var, &val, val_len, &new_val_len)) {
				php_register_variable_safe(var, val, new_val_len, &array);
			}
			efree(val);
		}
                ...
}
```

首先引入query string 的概念
> The HTTP query string is formed using a concatenation of the
   question mark character ('?', US-ASCII value 0x003F), the JSON object
   value associated with the object being searched for, the equal sign
   character ('=', US-ASCII value 0x003D), and the search pattern.
query string 也是在这里解析的
