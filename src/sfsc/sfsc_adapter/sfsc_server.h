/*
 * sfsc_server.h
 *
 *  Created on: 27.10.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_ADAPTER_SFSC_SERVER_H_
#define SRC_SFSC_ADAPTER_SFSC_SERVER_H_

#include "sfsc_adapter.h"
#include "sfsc_adapter_data.h"
#include "sfsc_command.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _sfsc_pending_ack_DEFAULT_INIT {_ACK_ID_UNUSED,0,0,sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_init_default,sfsc_buffer_DEFAULT_INIT,0,NULL,0,NULL,NULL, NULL}
typedef struct __sfsc_pending_ack {
	sfsc_int32 ack_id;
	sfsc_uint8 tries;
	sfsc_uint64 valid_until;
	sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings ack_settings;
	sfsc_buffer reply_topic;
	sfsc_int32 expected_reply_id;
	union {
		sfsc_buffer* payload;
		sfsc_channel_answer* channel_answer;
	} content;
	sfsc_bool is_channel;
	void* arg;
	sfsc_answer_ack_callback* on_ack;
	sfsc_server* server;
} _sfsc_pending_ack;

#define _sfsc_acks_DEFAULT_INIT {0,{_sfsc_pending_ack_DEFAULT_INIT}}
typedef struct __sfsc_acks {
	sfsc_int32 ack_id_counter;
	_sfsc_pending_ack callbacks[MAX_PENDING_ACKS];
} _sfsc_acks;

sfsc_int8 sfsc_internal_register_server(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter, _sfsc_commands* commands,
		sfsc_server* servers[], sfsc_server* server, sfsc_buffer name,
		sfsc_buffer custom_tags, sfsc_buffer output_message_type,
		sfsc_buffer input_message_type, sfsc_command_callback* callback);
sfsc_int8 sfsc_internal_unregister_server(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,
		_sfsc_commands* commands, sfsc_server* servers[], sfsc_server* server,
		sfsc_command_callback* callback);
sfsc_int8 sfsc_internal_answer_request(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter, _sfsc_acks* pending_acks,
		sfsc_server* server,
		sfsc_int32 expected_reply_id, sfsc_buffer reply_topic,
		sfsc_buffer* payload, void* arg,
		sfsc_answer_ack_callback* on_ack);
sfsc_int8 sfsc_internal_answer_channel_request(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,
		_sfsc_acks* pending_acks, sfsc_server* server,
		sfsc_int32 expected_reply_id, sfsc_buffer reply_topic,
		sfsc_channel_answer* answer_data, void* arg,
		sfsc_answer_ack_callback* on_ack);

sfsc_int8 user_task_ack(sfsc_adapter* forward_pointer,_sfsc_adapter_data* adapter, _sfsc_acks* pending_acks);
sfsc_int8 system_task_data_sub_acks(_sfsc_adapter_data* adapter,
		_sfsc_acks* pending_acks, sfsc_uint8* message);
sfsc_int8 user_task_data_check_servers(sfsc_adapter* forward_pointer,
		sfsc_server* servers[],
		sfsc_uint8* next_topic,
		sfsc_size topic_len, sfsc_uint8* next_payload,
		sfsc_size payload_len, sfsc_uint8* consumed,sfsc_bool* b_auto_advance);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_SERVER_H_ */
