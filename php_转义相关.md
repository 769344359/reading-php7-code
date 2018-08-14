# 前言
php 的非常多的语法特性和别的语言不一样,本次则是分析单双引号的字面量字符串的转义问题
>> **问题 1** php的字面量是在什么时候转义的?

>>**问题 2** 单引号和双引号有什么不同?

# 前置知识
php 的官方代码是由c 写成的,所以源码都是从main 开始的
- php 的生命周期  
生命周期大概是这样:  

启动sapi-启动php解析器-词法分析-语法分析-生成opcode-执行opcode...
# 文件路径
`Zend\zend_language_scanner.l`

# 单引号
根据上面的问题,我们看一下这个文件`Zend\zend_language_scanner.l`
```
//  Zend\zend_language_scanner.l
<ST_IN_SCRIPTING>b?['] {
	register char *s, *t;
	char *end;
	int bprefix = (yytext[0] != '\'') ? 1 : 0;
	...
	ZVAL_STRINGL(zendlval, yytext+bprefix+1, yyleng-bprefix-2);

	/* convert escape sequences */
	s = t = Z_STRVAL_P(zendlval);
	end = s+Z_STRLEN_P(zendlval);
	while (s<end) {
		if (*s=='\\') {
			s++;

			switch(*s) {
				case '\\':     //      对 \\ 以及 \' 这两个字符转义
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
	RETURN_TOKEN(T_CONSTANT_ENCAPSED_STRING);
}
```
由上面的代码可知 php代码 会在单引号的时候转义 `\\` 以及 `\'`
```
<?php
$a = '\\';
$b = 'aab\'c';
echo $a;   //   输出为 \
echo $b;   //   输出为 aab'c
```
# 双引号 
当词法分析的时候,在双引号扩起来的部分会经过函数`zend_scan_escape_string` 处理完成转义
```
// Zend\zend_language_scanner.l
<ST_IN_SCRIPTING>b?["] {
	int bprefix = (yytext[0] != '"') ? 1 : 0;

	while (YYCURSOR < YYLIMIT) {
		switch (*YYCURSOR++) {
			case '"':
				yyleng = YYCURSOR - SCNG(yy_text);
				zend_scan_escape_string(zendlval, yytext+bprefix+1, yyleng-bprefix-2, '"');  // 核心转义函数
				RETURN_TOKEN(T_CONSTANT_ENCAPSED_STRING);
```
下面是转义函数
```
// Zend\zend_language_scanner.l
static int zend_scan_escape_string(zval *zendlval, char *str, int len, char quote_type)
{
	register char *s, *t;
	char *end;

	ZVAL_STRINGL(zendlval, str, len);

	/* convert escape sequences */
	s = t = Z_STRVAL_P(zendlval);
	end = s+Z_STRLEN_P(zendlval);
	while (s<end) {
		if (*s=='\\') {
			s++;
			if (s >= end) {
				*t++ = '\\';
				break;
			}

			switch(*s) {
				case 'n':
					*t++ = '\n';    \\  \n 转义
					Z_STRLEN_P(zendlval)--;
					break;
				case 'r':
					*t++ = '\r';    \\  \r 转义
					Z_STRLEN_P(zendlval)--;
					break;
				case 't':
					*t++ = '\t';
					Z_STRLEN_P(zendlval)--;
					break;
				case 'f':
					*t++ = '\f';
					Z_STRLEN_P(zendlval)--;
					break;
				case 'v':
					*t++ = '\v';
					Z_STRLEN_P(zendlval)--;
					break;
				case 'e':
					*t++ = '\e';
					Z_STRLEN_P(zendlval)--;
					break;
				case '"':         \\ 双引号转义  没有break 注意一下
				case '`':
					if (*s != quote_type) {
						*t++ = '\\';
						*t++ = *s;
						break;
					}
				case '\\':    \\ 反斜杠转义
				case '$':
					*t++ = *s;
					Z_STRLEN_P(zendlval)--;
					break;
				case 'x':     // 后面还有很长的case 不列出
	...
	return SUCCESS;
}
```

```
<?php
$a= "\\abc";
echo $a;
```
转义后
```
(gdb) p (char *)zendlval->value.str.val
$2 = 0x7ffff6202798 "\\abcc"
(gdb) p zendlval->value.str.val[0]
$4 = 92 '\\'
(gdb) p zendlval->value.str.val[1]
$5 = 97 'a'
(gdb) p zendlval->value.str.val[2]
$6 = 98 'b'
(gdb) p zendlval->value.str.val[3]
$7 = 99 'c'
(gdb) p zendlval->value.str.val[4]
$8 = 0 '\000'
```

由上面可知`双引号`里面的字符串转义也是在`词法分析`阶段做的,转义的具体内容可以看switch ...case... 内容

# 总结
1 `单引号` 和`双引号`内的字符串都是在`词法分析`完成,因此`opcache` 等扩展可以优化掉这部分损耗
2 不仅仅是双引号会转义字符串,单引号也会转义某些字符串
