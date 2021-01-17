/*
 * sfsc_command.c
 *
 *  Created on: 27.10.2020
 *      Author: Eric
 */

#include "sfsc_command.h"
#include "sfsc_utils.h"
#include "sfsc_constants.h"

sfsc_int8 execute_command(sfsc_adapter_stats* stats, _sfsc_adapter_data* data,
		_sfsc_commands* commands, sfsc_publisher_or_server service,
		sfsc_CommandRequest* request,
		sfsc_uint8 create, sfsc_command_callback* callback) {
	sfsc_size i = 0;
	for (; i < MAX_SIMULTANIOUS_COMMANDS; i++) {
		if (commands->callbacks[i].reply_id == _REPLY_ID_UNUSED) {
			commands->callbacks[i].reply_id = ++commands->reply_id_counter; //preincrement
			commands->callbacks[i].created = create;
			commands->callbacks[i].callback = callback;
			commands->callbacks[i].service = service;
			commands->callbacks[i].complete = 0;
			break;
		}
	}
	if (i < MAX_SIMULTANIOUS_COMMANDS) {

		sfsc_copy((sfsc_uint8*) request->adapterId.id, stats->adapter_id, UUID_LEN);
		sfsc_SfscServiceDescriptor* target;
		if (create) {
			target = &(request->create_or_delete.create_request);
			request->which_create_or_delete =
			sfsc_CommandRequest_create_request_tag;
		} else {
			target = &(request->create_or_delete.delete_request);
			request->which_create_or_delete =
			sfsc_CommandRequest_delete_request_tag;
		}
		sfsc_copy((sfsc_uint8*) target->adapter_id.id, stats->adapter_id, UUID_LEN);
		sfsc_copy((sfsc_uint8*) target->core_id.id, stats->core_id, UUID_LEN);

		sfsc_uint8* service_id;
		if (service.is_server) {
			service_id = (sfsc_uint8*) service.service.server->service_id.id;
		} else {
			service_id = (sfsc_uint8*) service.service.publisher->service_id.id;
		}
		sfsc_copy((sfsc_uint8*) target->service_id.id, service_id, UUID_LEN);

		sfsc_buffer reply_topic_parts[2] = { { _command_topic,
		_COMMAND_TOPIC_LEN }, { stats->adapter_id, UUID_LEN } };
		sfsc_composite_buffer reply_topic = { reply_topic_parts, 2 };

		if (b_encode_and_publish_with_request_composite_reply_pattern(
				_command_topic, _COMMAND_TOPIC_LEN, &reply_topic,
				commands->callbacks[i].reply_id, &(data->control_pub),
				sfsc_CommandRequest_fields, request)) {
			return ZMTP_OK;
		} else {
			return E_PROTO_ENCODE;
		}
	} else {
		return E_NO_FREE_SLOT;
	}
}

sfsc_int8 system_task_commands(sfsc_adapter_stats* stats,_sfsc_adapter_data* data,
		_sfsc_commands* commands) {
	sfsc_uint8* message = get_message(&(data->control_sub));
	if (message != NULL) {

		sfsc_buffer response_topic_parts[2] = { { _command_topic,
		_COMMAND_TOPIC_LEN }, { stats->adapter_id, UUID_LEN } };
		sfsc_composite_buffer response_topic = { response_topic_parts, 2 };

		if (stats->state == SFSC_STATE_OPERATIONAL) {
			if (data->control_sub.is_message == 1
					&& data->control_sub.last_message == 0
					&& b_composite_bytes_equal(&response_topic, message,
							data->control_sub.buffer_index)) {
				zmtp_pop(&(data->control_sub));
				stats->state = SFSC_STATE_EXPECT_COMMAND_RESULT;
			}
		} else if (data->control_sub.is_message
				== 1&& data->control_sub.last_message == 1
				&& stats->state == SFSC_STATE_EXPECT_COMMAND_RESULT) {
			sfsc_int64 reply_id;
			sfsc_size stripped_len;
			sfsc_uint8* stripped;
			if (b_strip_request_reply_pattern(message,
					data->control_sub.buffer_index, &stripped, &stripped_len,
					&reply_id)) {
				if (reply_id != _REPLY_ID_UNUSED) {
					for (sfsc_size i = 0; i < MAX_SIMULTANIOUS_COMMANDS; i++) {
						if (commands->callbacks[i].reply_id == reply_id) {
							if (!commands->callbacks[i].complete) {
								commands->callbacks[i].complete = 1;
							}
							break;
						}
					}
				}
				zmtp_pop(&(data->control_sub));
				stats->state = SFSC_STATE_OPERATIONAL;
				return ZMTP_OK;
			} else {
				return E_PROTO_DECODE;
			}
		}
	}
	return ZMTP_OK;
}

sfsc_int8 user_task_commands(sfsc_adapter* forward_pointer,_sfsc_commands* commands) {
	for (sfsc_size i = 0; i < MAX_SIMULTANIOUS_COMMANDS; i++) {
		_sfsc_command_callback* c = &commands->callbacks[i];
		if (c->reply_id != _REPLY_ID_UNUSED && c->complete) {
			c->reply_id = _REPLY_ID_UNUSED;
			if (c->callback != NULL) {
				c->callback( forward_pointer,c->service, c->created);
			}
		}
	}
	return ZMTP_OK;
}
