/*
 * sfsc_query_handeling.c
 *
 *  Created on: 06.10.2020
 *      Author: Eric
 */

#include "sfsc_query.h"
#include "sfsc_utils.h"
#include "../proto/pb_decode.h"

#define _MAX_EVENT_ID 0x7fffffffffffffff

#define _copy_target_name 1
#define _copy_target_custom_tags 2
#define _copy_target_publisher_tags_topic 3
#define _copy_target_publisher_tags_output_message_type 4
#define _copy_target_server_tags_topic 5
#define _copy_target_server_tags_input_message_type 6
#define _copy_target_server_tags_output_message_type 7

#define _GET_DECODING_TARGET(stream) (_decoding_target*)((((generic_decode_state*) stream->state)->extra))

#define _OK_QUERY_NEXT 1

const relative_sfsc_service_descriptor relative_sfsc_service_descriptor_default=relative_sfsc_service_descriptor_DEFAULT_INIT;
const relative_publisher_tags relative_publisher_tags_default=relative_publisher_tags_DEFAULT_INIT;
const relative_server_tags relative_server_tags_default=relative_server_tags_DEFAULT_INIT;

typedef struct __decoding_target {
	sfsc_uint8* buffer;sfsc_size* buffer_index;
	relative_sfsc_service_descriptor* descriptor;
} _decoding_target;

static sfsc_uint8 copy_target_name = _copy_target_name,
		copy_target_custom_tags = _copy_target_custom_tags,
		copy_target_publisher_tags_topic = _copy_target_publisher_tags_topic,
		copy_target_publisher_tags_output_message_type =
		_copy_target_publisher_tags_output_message_type,
		copy_target_server_tags_topic = _copy_target_server_tags_topic,
		copy_target_server_tags_input_message_type =
		_copy_target_server_tags_input_message_type,
		copy_target_server_tags_output_message_type =
		_copy_target_server_tags_output_message_type;

static sfsc_bool b_store_deleted(_sfsc_queries* reg, sfsc_SfscId* id) {
	for (sfsc_size i = 0; i < MAX_DELETED_MEMORY; i++) {
		if (reg->deleted_services[i].id[0] == 0) {
			reg->deleted_services[i] = *id;
			return 1;
		}
	}
	return 0;
}

static sfsc_bool b_check_and_process_deleted(_sfsc_queries* reg,
		sfsc_SfscId* id) {
	const sfsc_SfscId default_id = sfsc_SfscId_init_default;
	for (sfsc_size i = 0; i < MAX_DELETED_MEMORY; i++) {
		if (reg->deleted_services[i].id[0] != 0
				&& b_bytes_equal((sfsc_uint8*) id->id,
						(sfsc_uint8*) reg->deleted_services[i].id,
						UUID_LEN)) {
			reg->deleted_services[i] = default_id;
			return 1;
		}
	}
	return 0;
}

static void clear_deleted(_sfsc_queries* reg) {
	for (sfsc_size i = 0; i < MAX_DELETED_MEMORY; i++) {
		reg->deleted_services[i].id[0] = 0;
	}
}

static sfsc_bool decode_service_tagscopy_callback(pb_istream_t *stream,
		const pb_field_t *pb_field, void **arg) {
	_decoding_target* target = _GET_DECODING_TARGET(stream);
	sfsc_uint16 bytes_to_write = stream->bytes_left;
	if ((*(target->buffer_index) + bytes_to_write) < REGISTRY_BUFFER_SIZE) {
		sfsc_size* field;
		sfsc_size* len_field;
		sfsc_uint8 copy_target_type = *((sfsc_uint8*) (*arg));
		switch (copy_target_type) {
		case _copy_target_name:
			field = &(target->descriptor->name_offset);
			len_field = &(target->descriptor->name_len);
			break;
		case _copy_target_custom_tags:
			field = &(target->descriptor->custom_tags_offset);
			len_field = &(target->descriptor->custom_tags_len);
			break;
		case _copy_target_publisher_tags_topic:
			field =
					&(target->descriptor->service_tags.publisher_tags.topic_offset);
			len_field =
					&(target->descriptor->service_tags.publisher_tags.topic_len);
			break;
		case _copy_target_publisher_tags_output_message_type:
			field =
					&(target->descriptor->service_tags.publisher_tags.output_message_type_offset);
			len_field =
					&(target->descriptor->service_tags.publisher_tags.output_message_type_len);
			break;
		case _copy_target_server_tags_topic:
			field =
					&(target->descriptor->service_tags.server_tags.topic_offset);
			len_field =
					&(target->descriptor->service_tags.server_tags.topic_len);
			break;
		case _copy_target_server_tags_input_message_type:
			field =
					&(target->descriptor->service_tags.server_tags.input_message_type_offset);
			len_field =
					&(target->descriptor->service_tags.server_tags.input_message_type_len);
			break;
		case _copy_target_server_tags_output_message_type:
			field =
					&(target->descriptor->service_tags.server_tags.output_message_type_offset);
			len_field =
					&(target->descriptor->service_tags.server_tags.output_message_type_len);
			break;
		default:
			return 0;
		}
		*field = *(target->buffer_index);
		*len_field = bytes_to_write;
		pb_read(stream, target->buffer + *field, bytes_to_write);
		(*target->buffer_index) += bytes_to_write;
		return 1;
	} else {
		return 0;
	}
}

static sfsc_bool decode_discard_callback(pb_istream_t *stream,
		const pb_field_t *field, void **arg) {
	pb_read(stream, NULL, stream->bytes_left);
	return 1;
}

static sfsc_bool prepare_service_descriptor_submsg(pb_istream_t *stream,
		const pb_field_t *field, void **arg) {
	_decoding_target* target = _GET_DECODING_TARGET(stream);
	if (field->tag == sfsc_SfscServiceDescriptor_ServiceTags_server_tags_tag) {
		sfsc_SfscServiceDescriptor_ServiceTags_ServerTags *msg =
				(sfsc_SfscServiceDescriptor_ServiceTags_ServerTags*) field->pData;
		target->descriptor->service_type = SERVICE_TYPE_SERVER;
		target->descriptor->service_tags.server_tags =
				relative_server_tags_default;
		msg->input_message_type.type.funcs.decode =
				decode_service_tagscopy_callback;
		msg->input_message_type.type.arg =
				&copy_target_server_tags_input_message_type;
		msg->output_message_type.type.funcs.decode =
				decode_service_tagscopy_callback;
		msg->output_message_type.type.arg =
				&copy_target_server_tags_output_message_type;
		msg->input_topic.topic.funcs.decode = decode_service_tagscopy_callback;
		msg->input_topic.topic.arg = &copy_target_server_tags_topic;
		return 1;
	} else if (field->tag
			== sfsc_SfscServiceDescriptor_ServiceTags_publisher_tags_tag) {
		sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags *msg =
				(sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags*) field->pData;
		target->descriptor->service_type = SERVICE_TYPE_PUBLISHER;
		target->descriptor->service_tags.publisher_tags =
				relative_publisher_tags_default;
		msg->output_message_type.type.funcs.decode =
				decode_service_tagscopy_callback;
		msg->output_message_type.type.arg =
				&copy_target_publisher_tags_output_message_type;
		msg->output_topic.topic.funcs.decode = decode_service_tagscopy_callback;
		msg->output_topic.topic.arg = &copy_target_publisher_tags_topic;
		return 1;
	} else {
		return 0;
	}
}

static void set_up_service_descirptor_submsg(sfsc_SfscServiceDescriptor* msg) {
	msg->service_tags.cb_serviceTags.funcs.decode =
			prepare_service_descriptor_submsg;
	msg->custom_tags.funcs.decode = decode_service_tagscopy_callback;
	msg->custom_tags.arg = &copy_target_custom_tags;
	msg->service_name.funcs.decode = decode_service_tagscopy_callback;
	msg->service_name.arg = &copy_target_name;
}

static sfsc_bool prepare_query_reply_submsg(pb_istream_t *stream,
		const pb_field_t *field, void **arg) {
	if (field->tag == sfsc_QueryReply_expired_tag) {
		return 1;
	} else if (field->tag == sfsc_QueryReply_future_tag) {
		return 1;
	} else if (field->tag == sfsc_QueryReply_deleted_tag) {
		sfsc_SfscServiceDescriptor* deleted =
				(sfsc_SfscServiceDescriptor*) field->pData;
		deleted->service_tags.cb_serviceTags.funcs.decode =
				decode_discard_callback;
		return 1;
	} else if (field->tag == sfsc_QueryReply_created_tag) {
		set_up_service_descirptor_submsg(
				(sfsc_SfscServiceDescriptor*) field->pData);
		return 1;
	} else {
		return 0;
	}
}

static sfsc_int8 send_query_request(sfsc_adapter_stats* stats,
		_sfsc_adapter_data* adapter, _sfsc_queries* registry,
		sfsc_int64 event_id) {
	sfsc_buffer response_topic_parts[2] = { { _query_topic, _QUERY_TOPIC_LEN },
			{ stats->adapter_id, UUID_LEN } };
	sfsc_composite_buffer response_topic = { response_topic_parts, 2 };

	sfsc_QueryRequest req = sfsc_QueryRequest_init_default;
	req.event_id = event_id;
	registry->expected_event_id = event_id;
	if (b_encode_and_publish_with_request_composite_reply_pattern(_query_topic,
	_QUERY_TOPIC_LEN, &response_topic,
	_REPLY_ID_UNINTERESTED, &(adapter->control_pub),
	sfsc_QueryRequest_fields, &req)) {
		return ZMTP_OK;
	} else {
		return E_PROTO_ENCODE;
	}
}

static void finalize_realtive_service_descriptor(
		relative_sfsc_service_descriptor* relative,
		sfsc_SfscServiceDescriptor* descriptor) {
	relative->service_id = descriptor->service_id;
	relative->adapter_id = descriptor->adapter_id;
	relative->core_id = descriptor->core_id;
	if (descriptor->service_tags.which_serviceTags
			== sfsc_SfscServiceDescriptor_ServiceTags_server_tags_tag) {
		relative->service_tags.server_tags.ack_settings =
				descriptor->service_tags.serviceTags.server_tags.ack_settings;
	} else {
		relative->service_tags.publisher_tags.unregistered =
				descriptor->service_tags.serviceTags.publisher_tags.unregistered;
	}
}

sfsc_int8 decode_channel_result(sfsc_uint8* msg, sfsc_size msg_len,
		relative_sfsc_service_descriptor* descriptor, sfsc_uint8* write_target,
		sfsc_size write_target_len, sfsc_size* write_target_used) {
	if (write_target_len < msg_len) {
		*write_target_used=msg_len;
		return E_BUFFER_INSUFFICIENT;
	} else {
		*write_target_used = 0;
		sfsc_ChannelFactoryReply reply = sfsc_ChannelFactoryReply_init_default;
		set_up_service_descirptor_submsg(&reply.service_descriptor);

		_decoding_target as_target = { write_target, write_target_used,
				descriptor };
		generic_decode_state decode_state = { &as_target, msg, msg_len };
		pb_istream_t input = GENERIC_ISTREAM(&decode_state);

		if (pb_decode(&input, sfsc_ChannelFactoryReply_fields, &reply)) {
			finalize_realtive_service_descriptor(descriptor,
					&reply.service_descriptor);
			return ZMTP_OK;
		} else {
			return E_PROTO_DECODE;
		}
	}
}

static sfsc_int8 decode_query_result(_sfsc_queries* registry, sfsc_uint8* msg,
sfsc_size msg_len, sfsc_int64 *query_next) {

	_decoding_target as_target = { registry->buffer, &registry->buffer_index,
			&registry->latest };
	generic_decode_state decode_state = { &as_target, msg, msg_len };
	sfsc_QueryReply reply = sfsc_QueryReply_init_default;
	reply.cb_created_or_deleted_or_expired_or_future.funcs.decode =
			prepare_query_reply_submsg;

	pb_istream_t input = GENERIC_ISTREAM(&decode_state);
	if (pb_decode(&input, sfsc_QueryReply_fields, &reply)) {
		if (reply.event_id == registry->expected_event_id) {
			switch (reply.which_created_or_deleted_or_expired_or_future) {
			case sfsc_QueryReply_expired_tag:
				registry->has_data = 0;
				registry->is_last = 1;
				return ZMTP_OK;
			case sfsc_QueryReply_deleted_tag:
				registry->has_data = 0;
				if (b_store_deleted(registry,
						&(reply.created_or_deleted_or_expired_or_future.deleted.service_id))) {
					*query_next = reply.event_id - 1;
					return _OK_QUERY_NEXT;
				} else {
					return E_NO_FREE_SLOT;
				}
			case sfsc_QueryReply_future_tag:
				*query_next =
						reply.created_or_deleted_or_expired_or_future.future.newest_valid_event_id;
				*query_next -= 1; //TODO report this bug to Matthias
				return _OK_QUERY_NEXT;
			case sfsc_QueryReply_created_tag:
				if (b_check_and_process_deleted(registry,
						&(reply.created_or_deleted_or_expired_or_future.created.service_id))) {
					*query_next = reply.event_id - 1;
					return _OK_QUERY_NEXT;
				} else {
					registry->has_data = 1;
					finalize_realtive_service_descriptor(&registry->latest,
							&reply.created_or_deleted_or_expired_or_future.created);
					return ZMTP_OK;
				}
			default:
				return E_UNEXPECTED;
			}
		} else {
			return E_UNEXPECTED;
		}
	} else {
		return E_PROTO_DECODE;
	}
}

sfsc_int8 system_task_query(sfsc_adapter_stats* stats,
		_sfsc_adapter_data* adapter, _sfsc_queries* registry) {
	sfsc_uint8* message = get_message(&(adapter->control_sub));
	if (message != NULL) {

		sfsc_buffer response_topic_parts[2] = {
				{ _query_topic, _QUERY_TOPIC_LEN }, { stats->adapter_id,
				UUID_LEN } };
		sfsc_composite_buffer response_topic = { response_topic_parts, 2 };

		if (stats->state == SFSC_STATE_OPERATIONAL) {
			if (adapter->control_sub.is_message == 1
					&& adapter->control_sub.last_message == 0
					&& b_composite_bytes_equal(&response_topic, message,
							adapter->control_sub.buffer_index)) {
				zmtp_pop(&(adapter->control_sub));
				stats->state = SFSC_STATE_EXPECT_QUERY_RESULT;
			}
		} else if (adapter->control_sub.is_message
				== 1&& adapter->control_sub.last_message == 1
				&& stats->state == SFSC_STATE_EXPECT_QUERY_RESULT) {
			sfsc_int64 reply_id;
			sfsc_size stripped_len;
			sfsc_uint8* stripped;
			if (b_strip_request_reply_pattern(message,
					adapter->control_sub.buffer_index, &stripped, &stripped_len,
					&reply_id)) {
				sfsc_int64 query_next;
				sfsc_int8 op_result = decode_query_result(registry, stripped,
						stripped_len, &query_next);
				if (op_result == _OK_QUERY_NEXT) {
					op_result = send_query_request(stats, adapter, registry,
							query_next);
				}
				zmtp_pop(&(adapter->control_sub));
				stats->state = SFSC_STATE_OPERATIONAL;
				return op_result;
			} else {
				return E_PROTO_DECODE;
			}
		}
	}
	return ZMTP_OK;
}

sfsc_int8 sfsc_internal_query_services(sfsc_adapter_stats* stats,
		_sfsc_adapter_data* adapter, _sfsc_queries* registry,
		sfsc_query_callback* callback) {
	if (stats->query_in_progress == 0) {

		registry->is_last = 0;

		registry->has_data = 0;
		registry->latest = relative_sfsc_service_descriptor_default;
		registry->buffer_index = 0;
		sfsc_int8 op_result = send_query_request(stats, adapter, registry,
		_MAX_EVENT_ID);
		if (op_result == ZMTP_OK) {
			stats->query_in_progress = 1;
			registry->callback = (void*) callback;
			clear_deleted(registry);
			return ZMTP_OK;
		} else {
			return op_result;
		}
	} else {
		return E_QUERY_IN_PROGRESS;
	}
}

void sfsc_internal_query_services_next(sfsc_adapter_stats* stats,
		_sfsc_adapter_data* adapter, _sfsc_queries* registry,
		sfsc_bool next) {
	if (next && !registry->is_last) {
		send_query_request(stats, adapter, registry,
				registry->expected_event_id - 1);
	} else {
		stats->query_in_progress = 0;
	}
}

sfsc_int8 user_task_query(sfsc_adapter* forward_pointer, _sfsc_queries* registry) {
	if (adapter_stats(forward_pointer)->query_in_progress) {
		if (registry->has_data == 1) {
			sfsc_query_callback* callback =
					(sfsc_query_callback*) registry->callback;
			callback(forward_pointer, registry->latest, registry->buffer,
					registry->buffer_index, registry->is_last);
		} else if (registry->is_last == 1) {
			sfsc_query_callback* callback =
					(sfsc_query_callback*) registry->callback;
			callback(forward_pointer, relative_sfsc_service_descriptor_default,
			NULL, 0, 1);
		}
		registry->has_data = 0;
		registry->latest = relative_sfsc_service_descriptor_default;
		registry->buffer_index = 0;
	}
	return ZMTP_OK;
}
