##  send_command 
`mysqlnd` 的`send_command` 是mysql比较核心的函数，我们先看这个函数
- 首先是宏`MYSQLND_METHOD` 
我们可以看到这个宏的定义
```
// /ext/mysqlnd/mysqlnd.h
#define MYSQLND_METHOD(class, method) 	 mysqlnd_##class##_##method##_pub

```
容易知道这只是一个通过链接符链接的宏而已，目的就是生成一个唯一的函数
所以这个宏会生产一个函数`mysqlnd_mysqlnd_protocol_send_command_pub`,
而这个函数包括10个参数
```
MYSQLND_PROTOCOL_PAYLOAD_DECODER_FACTORY * payload_decoder_factory, //1
		const enum php_mysqlnd_server_command command, // 2
		const zend_uchar * const arg, const size_t arg_len,  // 3
		const zend_bool silent,    // 4 

		struct st_mysqlnd_connection_state * connection_state,  // 5
		MYSQLND_ERROR_INFO	* error_info,    //6
		MYSQLND_UPSERT_STATUS * upsert_status,  // 7
		MYSQLND_STATS * stats,   // 8
		func_mysqlnd_conn_data__send_close send_close,  // 9
		void * send_close_ctx   // 10
```
> 我们来介绍第一个参数 `MYSQLND_PROTOCOL_PAYLOAD_DECODER_FACTORY * payload_decoder_factory`
```
 typedef struct st_mysqlnd_protocol_payload_decoder_factory	MYSQLND_PROTOCOL_PAYLOAD_DECODER_FACTORY;
 
 struct st_mysqlnd_protocol_payload_decoder_factory
{
	MYSQLND_CONN_DATA * conn;
	zend_bool persistent;
	MYSQLND_CLASS_METHODS_TYPE(mysqlnd_protocol_payload_decoder_factory) m;
};
```
很显然第一个参数是一个工厂，包括链接信息 ， 是否是长链接的标志位 ， 以及一个 宏 `MYSQLND_CLASS_METHODS_TYPE`
> 第二个参数 是mysql 命令的枚举类型
这个枚举类型和通讯协议有一点点关系
```

/*
  After adding new elements please update
  `mysqlnd_command_to_text` in mysqlnd_wireprotocol.c
*/
enum php_mysqlnd_server_command
{
	COM_SLEEP = 0,
	COM_QUIT,
	COM_INIT_DB,
	COM_QUERY,
	COM_FIELD_LIST,
	COM_CREATE_DB,
	COM_DROP_DB,
	COM_REFRESH,
	COM_SHUTDOWN,
	COM_STATISTICS,
	COM_PROCESS_INFO,
	COM_CONNECT,
	COM_PROCESS_KILL,
	COM_DEBUG,
	COM_PING,
	COM_TIME = 15,
	COM_DELAYED_INSERT,
	COM_CHANGE_USER,
	COM_BINLOG_DUMP,
	COM_TABLE_DUMP,
	COM_CONNECT_OUT = 20,
	COM_REGISTER_SLAVE,
	COM_STMT_PREPARE = 22,  // 0x16
	COM_STMT_EXECUTE = 23,  // 0x17
	COM_STMT_SEND_LONG_DATA = 24,
	COM_STMT_CLOSE = 25,
	COM_STMT_RESET = 26,
	COM_SET_OPTION = 27,
	COM_STMT_FETCH = 28,
	COM_DAEMON = 29,
	COM_BINLOG_DUMP_GTID = 30,
	COM_RESET_CONNECTION = 31,
	COM_STMT_EXECUTE_BATCH = 32,
	COM_END,
	/* Here follow own, non-protocol, commands */
	COM_REAP_RESULT=240,	/* own command */
	COM_ENABLE_SSL,			/* own command */
	COM_HANDSHAKE,			/* own command */
};
```
注意到第二个参数是一个枚举类型， 而且com_stmt_prepare 是二进制数字 `0x16`


> 参数 const zend_uchar * const arg, const size_t arg_len,
> 		const zend_bool silent,
> 不知道有什么用


> 到了下一个参数 `struct st_mysqlnd_connection_state * connection_state`
他包括一个枚举类型和 一个methods_type

```
struct st_mysqlnd_connection_state
{
	enum mysqlnd_connection_state state;

	MYSQLND_CLASS_METHODS_TYPE(mysqlnd_connection_state) *m;
};

```
***
我们来正式看看函数的主要步骤
- 1  `mysqlnd_connection_state`
```
/*
		/-----> CONN_CLOSE  <---------------\
		|           ^                         \
		|           |                         \
	CONN_READY -> CONN_QUERY_SENT -> CONN_FETCHING_DATA
		^                                      |
		\-------------------------------------/
*/
typedef enum mysqlnd_connection_state
{
	CONN_ALLOCED = 0,
	CONN_READY = 1,
	CONN_QUERY_SENT = 2,
	CONN_SENDING_LOAD_DATA = 3,
	CONN_FETCHING_DATA = 4,
	CONN_NEXT_RESULT_PENDING = 5,
	CONN_QUIT_SENT = 6 /* object is "destroyed" at this stage */
} enum_mysqlnd_connection_state;
```
`connection_state` 是一个枚举类型,

***
我们来看看`mysqlnd_protocol_send_command` 这个函数的核心调用
```
MYSQLND_PACKET_COMMAND * cmd_packet = NULL;
...
if (! PACKET_WRITE(cmd_packet)) {
...
```
`PACKET_WRITE`  的展示是
```
#define PACKET_WRITE(packet)	((packet)->header.m->write_to_net((packet)))
```  


```


/* {{{ mysqlnd_protocol::send_command */
static enum_func_status
MYSQLND_METHOD(mysqlnd_protocol, send_command)(
		MYSQLND_PROTOCOL_PAYLOAD_DECODER_FACTORY * payload_decoder_factory,
		const enum php_mysqlnd_server_command command,
		const zend_uchar * const arg, const size_t arg_len,
		const zend_bool silent,

		struct st_mysqlnd_connection_state * connection_state,
		MYSQLND_ERROR_INFO	* error_info,
		MYSQLND_UPSERT_STATUS * upsert_status,
		MYSQLND_STATS * stats,
		func_mysqlnd_conn_data__send_close send_close,
		void * send_close_ctx)
{
	enum_func_status ret = PASS;
	MYSQLND_PACKET_COMMAND * cmd_packet = NULL;
	enum mysqlnd_connection_state state;
	DBG_ENTER("mysqlnd_protocol::send_command");
	DBG_INF_FMT("command=%s silent=%u", mysqlnd_command_to_text[command], silent);
	DBG_INF_FMT("server_status=%u", UPSERT_STATUS_GET_SERVER_STATUS(upsert_status));
	DBG_INF_FMT("sending %u bytes", arg_len + 1); /* + 1 is for the command */
	state = connection_state->m->get(connection_state);  // 一个get 方法

	switch (state) {
		case CONN_READY:
			break;
		case CONN_QUIT_SENT:
			SET_CLIENT_ERROR(error_info, CR_SERVER_GONE_ERROR, UNKNOWN_SQLSTATE, mysqlnd_server_gone);
			DBG_ERR("Server is gone");
			DBG_RETURN(FAIL);
		default:
			SET_CLIENT_ERROR(error_info, CR_COMMANDS_OUT_OF_SYNC, UNKNOWN_SQLSTATE, mysqlnd_out_of_sync);
			DBG_ERR_FMT("Command out of sync. State=%u", state);
			DBG_RETURN(FAIL);
	}

	UPSERT_STATUS_SET_AFFECTED_ROWS_TO_ERROR(upsert_status);
	SET_EMPTY_ERROR(error_info);

	cmd_packet = payload_decoder_factory->m.get_command_packet(payload_decoder_factory, FALSE);
	if (!cmd_packet) {
		SET_OOM_ERROR(error_info);
		DBG_RETURN(FAIL);
	}

	cmd_packet->command = command;
	if (arg && arg_len) {
		cmd_packet->argument.s = (char *) arg;
		cmd_packet->argument.l = arg_len;
	}

	MYSQLND_INC_CONN_STATISTIC(stats, STAT_COM_QUIT + command - 1 /* because of COM_SLEEP */ );

	if (! PACKET_WRITE(cmd_packet)) {
		if (!silent) {
			DBG_ERR_FMT("Error while sending %s packet", mysqlnd_command_to_text[command]);
			php_error(E_WARNING, "Error while sending %s packet. PID=%d", mysqlnd_command_to_text[command], getpid());
		}
		connection_state->m->set(connection_state, CONN_QUIT_SENT);
		send_close(send_close_ctx);
		DBG_ERR("Server is gone");
		ret = FAIL;
	}
	PACKET_FREE(cmd_packet);
	DBG_RETURN(ret);
}
/* }}} */
```

