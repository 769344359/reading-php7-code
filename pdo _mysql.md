# mysql pod
## 介绍 mysqlnd
首先mysqlnd 是php 自带的mysql 扩展，`mysqlnd`是`mysql native driver` 的简称,主要的文件都在ext/mysqlnd 的目录下
 执行 命令
 
 ```
 cd /ext/mysqldn
 find . | wc -l
 54 
 ```
 
即该目录下有 **54** 个文件


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
```
/* {{{ php_mysqlnd_prepare_read */
static enum_func_status
php_mysqlnd_prepare_read(void * _packet)
{
	MYSQLND_PACKET_PREPARE_RESPONSE *packet= (MYSQLND_PACKET_PREPARE_RESPONSE *) _packet;
	MYSQLND_ERROR_INFO * error_info = packet->header.error_info;
	MYSQLND_PFC * pfc = packet->header.protocol_frame_codec;
	MYSQLND_VIO * vio = packet->header.vio;
	MYSQLND_STATS * stats = packet->header.stats;
	MYSQLND_CONNECTION_STATE * connection_state = packet->header.connection_state;
	/* In case of an error, we should have place to put it */
	size_t buf_len = pfc->cmd_buffer.length;
	zend_uchar *buf = (zend_uchar *) pfc->cmd_buffer.buffer;
	zend_uchar *p = buf;
	const zend_uchar * const begin = buf;
	unsigned int data_size;

	DBG_ENTER("php_mysqlnd_prepare_read");

	if (FAIL == mysqlnd_read_packet_header_and_body(&(packet->header), pfc, vio, stats, error_info, connection_state, buf, buf_len, "prepare", PROT_PREPARE_RESP_PACKET)) {
		DBG_RETURN(FAIL);
	}
	BAIL_IF_NO_MORE_DATA;

	data_size = packet->header.size;
	packet->error_code = uint1korr(p);
	p++;
	BAIL_IF_NO_MORE_DATA;

	if (ERROR_MARKER == packet->error_code) {
		php_mysqlnd_read_error_from_line(p, data_size - 1,
										 packet->error_info.error,
										 sizeof(packet->error_info.error),
										 &packet->error_info.error_no,
										 packet->error_info.sqlstate
										);
		DBG_RETURN(PASS);
	}

	if (data_size != PREPARE_RESPONSE_SIZE_41 &&
		data_size != PREPARE_RESPONSE_SIZE_50 &&
		!(data_size > PREPARE_RESPONSE_SIZE_50)) {
		DBG_ERR_FMT("Wrong COM_STMT_PREPARE response size. Received %u", data_size);
		php_error(E_WARNING, "Wrong COM_STMT_PREPARE response size. Received %u", data_size);
		DBG_RETURN(FAIL);
	}

	packet->stmt_id = uint4korr(p);
	p += 4;
	BAIL_IF_NO_MORE_DATA;

	/* Number of columns in result set */
	packet->field_count = uint2korr(p);
	p += 2;
	BAIL_IF_NO_MORE_DATA;

	packet->param_count = uint2korr(p);
	p += 2;
	BAIL_IF_NO_MORE_DATA;

	if (data_size > 9) {
		/* 0x0 filler sent by the server for 5.0+ clients */
		p++;
		BAIL_IF_NO_MORE_DATA;

		packet->warning_count = uint2korr(p);
	}

	DBG_INF_FMT("Prepare packet read: stmt_id=%u fields=%u params=%u",
				packet->stmt_id, packet->field_count, packet->param_count);

	BAIL_IF_NO_MORE_DATA;

	DBG_RETURN(PASS);
premature_end:
	DBG_ERR_FMT("PREPARE packet %d bytes shorter than expected", p - begin - packet->header.size);
	php_error_docref(NULL, E_WARNING, "PREPARE packet "MYSQLND_SZ_T_SPEC" bytes shorter than expected",
					 p - begin - packet->header.size);
	DBG_RETURN(FAIL);
}
/* }}} */
```

