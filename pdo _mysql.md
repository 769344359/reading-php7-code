# mysql pod

## 介绍 mysqlnd
首先mysqlnd 是php 自带的mysql 扩展，`mysqlnd`是`mysql native driver` 的简称 
在文件 `/ext/mysqlnd/mysqlnd_ps.c`

 
```   
    mysqlnd_stmt_read_prepare_response(MYSQLND_STMT * s)
{
	
	MYSQLND_PACKET_PREPARE_RESPONSE * prepare_resp;
    ...
	if (FAIL == PACKET_READ(prepare_resp)) {
		goto done;
	}
    ...
}
```
> 先看宏`MYSQLND_PACKET_PREPARE_RESPONSE`
```
/* COM_PREPARE response packet */
typedef struct st_mysqlnd_packet_prepare_response {
	MYSQLND_PACKET_HEADER	header;
	/* also known as field_count 0x00=OK , 0xFF=error */
	unsigned char	error_code;
	zend_ulong	stmt_id;
	unsigned int	field_count;
	unsigned int	param_count;
	unsigned int	warning_count;

	/* present in case of error */
	MYSQLND_ERROR_INFO	error_info;
} MYSQLND_PACKET_PREPARE_RESPONSE;
```
- 看到define 部分 其实调用了 read_from_net 这个方法

而read_from_net 就是`php_mysqlnd_prepare_read` 
-  首先看到宏  `PACKET_READ`
```
/ext/mysqlnd/mysqlnd_wireprotocol.c
#define PACKET_READ(packet)		((packet)->header.m->read_from_net((packet)))

```
由于 packet 的类型是`MYSQLND_PACKET_PREPARE_RESPONSE` ，所以`packet->header`即
`MYSQLND_PACKET_HEADER` 类型。

我们来看一下这个结构图的定义：
```
typedef struct st_mysqlnd_packet_header {
    ...
	mysqlnd_packet_methods *m;
    ...
	MYSQLND_CONN_DATA * conn;
    ...
} MYSQLND_PACKET_HEADER;
```
好了，我们现在到了最后一步
```
// 注意这个静态全局变量
/* {{{ packet_methods */
static mysqlnd_packet_methods packet_methods[PROT_LAST] =
{
	...
	{
		sizeof(MYSQLND_PACKET_PREPARE_RESPONSE),
		php_mysqlnd_prepare_read, /* read */
		NULL, /* write */
		php_mysqlnd_prepare_free_mem,
	}, /* PROT_PREPARE_RESP_PACKET */
    ...
};
/* }}} */

```
接下来我们只需要往下找`php_mysqlnd_prepare_read` 即可。

