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

	ZVAL_UNDEF(&array);
	switch (arg) {
		case PARSE_POST:
		case PARSE_GET:
		case PARSE_COOKIE:
			array_init(&array);
			switch (arg) {
				case PARSE_POST:
					zval_ptr_dtor(&PG(http_globals)[TRACK_VARS_POST]);
					ZVAL_COPY_VALUE(&PG(http_globals)[TRACK_VARS_POST], &array);
					break;
				case PARSE_GET:
					zval_ptr_dtor(&PG(http_globals)[TRACK_VARS_GET]);
					ZVAL_COPY_VALUE(&PG(http_globals)[TRACK_VARS_GET], &array);
					break;
				case PARSE_COOKIE:
					zval_ptr_dtor(&PG(http_globals)[TRACK_VARS_COOKIE]);
					ZVAL_COPY_VALUE(&PG(http_globals)[TRACK_VARS_COOKIE], &array);
					break;
			}
			break;
		default:
			ZVAL_COPY_VALUE(&array, destArray);
			break;
	}

	if (arg == PARSE_POST) {
		sapi_handle_post(&array);
		return;
	}

	if (arg == PARSE_GET) {		/* GET data */
		c_var = SG(request_info).query_string;
		if (c_var && *c_var) {
			res = (char *) estrdup(c_var);
			free_buffer = 1;
		} else {
			free_buffer = 0;
		}
	} else if (arg == PARSE_COOKIE) {		/* Cookie data */
		c_var = SG(request_info).cookie_data;
		if (c_var && *c_var) {
			res = (char *) estrdup(c_var);
			free_buffer = 1;
		} else {
			free_buffer = 0;
		}
	} else if (arg == PARSE_STRING) {		/* String data */
		res = str;
		free_buffer = 1;
	}

	if (!res) {
		return;
	}

	switch (arg) {
		case PARSE_GET:
		case PARSE_STRING:
			separator = (char *) estrdup(PG(arg_separator).input);
			break;
		case PARSE_COOKIE:
			separator = ";\0";
			break;
	}

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
next_cookie:
		var = php_strtok_r(NULL, separator, &strtok_buf);
	}

	if (arg != PARSE_COOKIE) {
		efree(separator);
	}

	if (free_buffer) {
		efree(res);
	}
}
```
