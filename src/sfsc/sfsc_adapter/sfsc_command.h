/*
 * sfsc_command.h
 *
 *  Created on: 27.10.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_ADAPTER_SFSC_COMMAND_H_
#define SRC_SFSC_ADAPTER_SFSC_COMMAND_H_

#include "sfsc_adapter.h"
#include "sfsc_adapter_data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __sfsc_command_callback {
	sfsc_int64 reply_id;
	sfsc_bool complete;
	sfsc_bool created;
	sfsc_publisher_or_server service;
	sfsc_command_callback* callback;
} _sfsc_command_callback;

#define _sfsc_command_callback_INIT_DEFAULT {_REPLY_ID_UNUSED,0,0,sfsc_publisher_or_server_INIT_DEFAULT,NULL}

typedef struct __sfsc_commands {
	sfsc_int64 reply_id_counter;
	_sfsc_command_callback callbacks[MAX_SIMULTANIOUS_COMMANDS];
} _sfsc_commands;
#define _sfsc_commands_DEFAULT_INIT {0,{_sfsc_command_callback_INIT_DEFAULT}}


sfsc_int8 user_task_commands(sfsc_adapter* forward_pointer,_sfsc_commands* commands);
sfsc_int8 system_task_commands(sfsc_adapter_stats* stats,_sfsc_adapter_data* data, _sfsc_commands* commands);
sfsc_int8 execute_command(sfsc_adapter_stats* stats,_sfsc_adapter_data* data,_sfsc_commands* commands, sfsc_publisher_or_server service,
		sfsc_CommandRequest* request,
		sfsc_uint8 create, sfsc_command_callback* callback);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_COMMAND_H_ */
