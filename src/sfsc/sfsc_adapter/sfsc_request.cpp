/*
 * sfsc_request.c
 *
 *  Created on: 27.10.2020
 *      Author: Eric
 */

#include "sfsc_request.h"
#include "sfsc_utils.h"
#include "sfsc_query.h"
#include "../platform/sfsc_platform.h"

#define _REPLY_QUEUED 0

static sfsc_bool b_send_ack(zmtp_socket* socket, sfsc_buffer ack_topic,
sfsc_int32 ack_id) {
	sfsc_RequestOrAcknowledge msg = sfsc_RequestOrAcknowledge_init_default;
	msg.which_request_or_acknowledge =
	sfsc_RequestOrAcknowledge_acknowledge_tag;
	msg.request_or_acknowledge.acknowledge.acknowledge_id = ack_id;
	return b_encode_and_publish(ack_topic.content, ack_topic.length, socket,
	sfsc_RequestOrAcknowledge_fields, &msg);
}

static sfsc_int8 generic_request(sfsc_adapter_stats* stats,
		_sfsc_adapter_data* adapter, _sfsc_requests* requests,
		sfsc_buffer topic, sfsc_buffer payload,
		sfsc_uint64 timeout, sfsc_uint8 is_channel_request,
		sfsc_uint8* descriptor_space, sfsc_uint16 descriptor_space_lenght,
		relative_sfsc_service_descriptor* descriptor,
		void* callback, void* arg) {
	sfsc_size i = 0;
	for (; i < MAX_SIMULTANIOUS_REQUESTS; i++) {
		if (requests->callbacks[i].reply_id == _REPLY_ID_UNUSED) {
			requests->callbacks[i].reply_id = ++requests->reply_id_counter; //preincrement

			requests->callbacks[i].arg = arg;
			requests->callbacks[i].valid_until = time_ms() + timeout;
			if (is_channel_request) {
				requests->callbacks[i].is_channel_request = 1;
				requests->callbacks[i].extra.channel.target_buffer =
						descriptor_space;
				requests->callbacks[i].extra.channel.taget_buffer_size =
						descriptor_space_lenght;
				requests->callbacks[i].extra.channel.descriptor=descriptor;
				requests->callbacks[i].extra.channel.callback =
						(sfsc_channel_request_callback*) callback;
			} else {
				requests->callbacks[i].is_channel_request = 0;
				requests->callbacks[i].extra.normal_callback =
						(sfsc_request_callback*) callback;
			}
			break;
		}
	}
	if (i < MAX_SIMULTANIOUS_REQUESTS) {
		sfsc_buffer reply_topic_parts[2] = { { _data_request_reply_topic,
		_DATA_REQUEST_REPLY_TOPIC_LEN }, { stats->adapter_id, UUID_LEN } };
		sfsc_composite_buffer reply_topic = { reply_topic_parts, 2 };

		sfsc_RequestOrAcknowledge msg =
		sfsc_RequestOrAcknowledge_Acknowledge_init_default;
		msg.which_request_or_acknowledge =
		sfsc_RequestOrAcknowledge_request_tag;
		msg.request_or_acknowledge.request.expected_reply_id =
				requests->callbacks[i].reply_id;
		msg.request_or_acknowledge.request.reply_topic.topic.funcs.encode =
				b_encode_composite_buffer_for_pb;
		msg.request_or_acknowledge.request.reply_topic.topic.arg = &reply_topic;

		//important: outside the if or else it will go out of scope!
		sfsc_ChannelFactoryRequest channel_request =
		sfsc_ChannelFactoryRequest_init_default;
		encode_submsg_to_bytes submsg = { sfsc_ChannelFactoryRequest_fields,
				&channel_request };

		if (is_channel_request) {
			channel_request.payload.funcs.encode = b_encode_buffer_for_pb;
			channel_request.payload.arg = &payload;
			msg.request_or_acknowledge.request.request_payload.funcs.encode =
					b_encode_submsg_to_bytes_callback;
			msg.request_or_acknowledge.request.request_payload.arg = &submsg;
		} else {
			msg.request_or_acknowledge.request.request_payload.funcs.encode =
					b_encode_buffer_for_pb;
			msg.request_or_acknowledge.request.request_payload.arg = &payload;
		}
		if (b_encode_and_publish(topic.content, topic.length,
				&(adapter->data_pub), sfsc_RequestOrAcknowledge_fields, &msg)) {
			return ZMTP_OK;
		} else {
			return E_PROTO_ENCODE;
		}

	} else {
		return E_NO_FREE_SLOT;
	}
}

sfsc_int8 sfsc_internal_request(sfsc_adapter_stats* stats,
		_sfsc_adapter_data* adapter, _sfsc_requests* requests,
		sfsc_buffer topic, sfsc_buffer payload,
		sfsc_uint64 timeout, sfsc_request_callback* callback, void* arg) {
	return generic_request(stats, adapter, requests, topic, payload, timeout, 0,
	NULL, 0, NULL, (void*) callback, arg);
}

sfsc_int8 sfsc_internal_channel_request(sfsc_adapter_stats* stats,
		_sfsc_adapter_data* adapter, _sfsc_requests* requests,
		sfsc_buffer topic, sfsc_buffer payload,
		sfsc_uint64 timeout, relative_sfsc_service_descriptor* descriptor,sfsc_uint8* descriptor_space,
		sfsc_uint16 descriptor_space_lenght,
		sfsc_channel_request_callback* callback, void* arg) {
	return generic_request(stats, adapter, requests, topic, payload, timeout, 1,
			descriptor_space, descriptor_space_lenght,descriptor, (void*) callback, arg);
}

static void invoke_request_callback(sfsc_adapter* forward_pointer,
		_sfsc_request* r, sfsc_uint8* payload, sfsc_size payload_len,
		sfsc_bool is_timeout,sfsc_bool* b_auto_advance) {
	if (r->is_channel_request) {
		if (r->extra.channel.callback != NULL) {
			if (!is_timeout) {
				sfsc_size length;
				sfsc_int8 decode_result = decode_channel_result(payload,
						payload_len, r->extra.channel.descriptor,
						r->extra.channel.target_buffer,
						r->extra.channel.taget_buffer_size, &length);
				if (decode_result == ZMTP_OK) {
					r->extra.channel.callback(forward_pointer, 0, ZMTP_OK,
							r->extra.channel.descriptor,
							r->extra.channel.target_buffer, length, r->arg,b_auto_advance);
				} else {
					r->extra.channel.callback(forward_pointer, 0, decode_result,
					NULL, NULL, length, r->arg, b_auto_advance);
				}
			} else {
				r->extra.channel.callback(forward_pointer, 1, 0, NULL, NULL, 0,
						r->arg,NULL);
			}
		}
	} else {
		if (r->extra.normal_callback != NULL) {
			sfsc_buffer buf=sfsc_buffer_DEFAULT_INIT;
			if (!is_timeout) {
				buf.content = payload;
				buf.length = payload_len;
				r->extra.normal_callback(forward_pointer,
						buf, 0, r->arg,b_auto_advance);
			} else {
				r->extra.normal_callback(forward_pointer,
				buf, 1, r->arg,NULL);
			}
		}
	}
}

void user_task_request_timeout(sfsc_adapter* forward_pointer,
		_sfsc_requests* requests) {
	sfsc_uint64 now = time_ms();
	for (sfsc_size i = 0; i < MAX_SIMULTANIOUS_REQUESTS; i++) {
		_sfsc_request* r = &(requests->callbacks[i]);
		if (r->reply_id != _REPLY_ID_UNUSED) {
			if (r->valid_until != _REPLY_QUEUED && r->valid_until < now) {
				invoke_request_callback(forward_pointer, r, NULL, 0, 1,NULL);
				r->reply_id = _REPLY_ID_UNUSED;
			}
		}
	}
}

sfsc_int8 system_task_data_sub_receive_reply(_sfsc_adapter_data* adapter,
		_sfsc_requests* requests,
		sfsc_uint8* message) {
	sfsc_size payload_len;
	sfsc_buffer ack_topic = sfsc_buffer_DEFAULT_INIT;
	sfsc_int32 reply_id;
	sfsc_int32 ack_id;
//1. decode with a generic stream to get reply payload_offset (like stripping)
	sfsc_uint8* payload;
	if (b_strip_reqrepack_pattern_reply(message, adapter->data_sub.buffer_index,
			&payload, &payload_len, &reply_id, &ack_topic, &ack_id)) {
		//2. find the index of the request
		sfsc_size request_index = 0;
		for (; request_index < MAX_SIMULTANIOUS_REQUESTS; request_index++) {
			if (requests->callbacks[request_index].reply_id == reply_id) {
				break;
			}
		}
		if (request_index < MAX_SIMULTANIOUS_REQUESTS) {
			sfsc_uint8 empty_payload_compensation = request_index;
			if (payload_len > 0) {
				//3. set payload_offset-- and then *payload_offset=request_index
				// 	 This has to work since each proto field has 2 bytes before it we can abuse
				payload--;
				payload_len++;
				*payload = request_index;
			} else {
				//Except of corse the payload is 0!
				//Don't worry, we got a backup plan
				payload = &empty_payload_compensation;
				payload_len = 1;
			}
			//4. store the modified payload in the user ring
			if (b_write_payload(&(adapter->user_buffer), payload,
					payload_len)) {
				//5. tell the requests that it's queued in the user ring so it won't time out
				requests->callbacks[request_index].valid_until =
				_REPLY_QUEUED;
				if (b_send_ack(&(adapter->data_pub), ack_topic, ack_id)) {
					return ZMTP_OK;
				} else {
					return E_PROTO_ENCODE;
				}
			} else {
				forget_last_written_if_topic(&(adapter->user_buffer));
				return W_MESSAGE_DROP;
			}

		} else {
			forget_last_written_if_topic(&(adapter->user_buffer));
			//Indicates we didn't expect this reply
			return E_UNEXPECTED;
		}
	} else {
		forget_last_written_if_topic(&(adapter->user_buffer));
		return E_PROTO_DECODE;
	}
}

sfsc_int8 user_task_data_handle_reply(sfsc_adapter* forward_pointer,
		_sfsc_requests* requests,
		sfsc_uint8* next_topic, sfsc_size topic_len, sfsc_uint8* next_payload,
		sfsc_size payload_len,sfsc_bool* b_auto_advance) {
	sfsc_uint8 request_index = *next_payload;
	payload_len--;
	if (payload_len == 0) {
		next_payload = NULL;
	} else {
		next_payload++;
	}
	_sfsc_request* r = &requests->callbacks[request_index];
	if (r->reply_id != _REPLY_ID_UNUSED) {
		invoke_request_callback(forward_pointer, r, next_payload, payload_len,
				0,b_auto_advance);
		r->reply_id = _REPLY_ID_UNUSED;
	}
	return ZMTP_OK;
}

