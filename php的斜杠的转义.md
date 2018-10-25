```php
<?php
$a = '\\aaa';
$b = '\aaa';
$c = "\aaa";
$d = "\\aaa";
echo $a;
echo $b;
echo $c;
echo $d;
```
这四个变量分别输出什么呢?
```
	ZVAL_STRINGL(zendlval, yytext+bprefix+1, yyleng-bprefix-2);

	/* convert escape sequences */
	s = t = Z_STRVAL_P(zendlval);
	end = s+Z_STRLEN_P(zendlval);
	while (s<end) {
		if (*s=='\\') {
			s++;

			switch(*s) {
				case '\\':
				case '\'':
					*t++ = *s;
					Z_STRLEN_P(zendlval)--;
					break;
				default:
					*t++ = '\\';
					*t++ = *s;
					break;
			}
		} else {
			*t++ = *s;
		}

		if (*s == '\n' || (*s == '\r' && (*(s+1) != '\n'))) {
			CG(zend_lineno)++;
		}
		s++;
	}
	*t = 0;
```
