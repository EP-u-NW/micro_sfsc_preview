/*
 * sfsc_server.c
 *
 *  Created on: 27.10.2020
 *      Author: Eric
 */

#include "sfsc_server.h"

#include "../platform/sfsc_platform.h"
#include "../proto/pb.h"
#include "../proto/pb_decode.h"
#include "sfsc_publisher.h"
#include "sfsc_utils.h"

#define _ACK_TRIGGERED 0

static sfsc_buffer topic_or_autogen(sfsc_server* service) {
    if (service->topic.length == 0) {
        sfsc_buffer buf = {(const sfsc_uint8*)service->service_id.id, UUID_LEN};
        return buf;
    } else {
        return service->topic;
    }
}

static sfsc_int8 send_server_reply(sfsc_adapter_stats* stats,
                                   _sfsc_adapter_data* adapter,
                                   _sfsc_pending_ack* settings) {
    sfsc_buffer ack_topic_parts[2] = {{_data_ack_topic, _DATA_ACK_TOPIC_LEN},
                                      {stats->adapter_id, UUID_LEN}};
    sfsc_composite_buffer ack_topic = {ack_topic_parts, 2};

    settings->tries++;
    settings->valid_until = time_ms() + settings->ack_settings.timeout_ms;

    sfsc_Reply reply = sfsc_Reply_init_default;
    reply.acknowledge_topic.topic.funcs.encode =
        b_encode_composite_buffer_for_pb;
    reply.acknowledge_topic.topic.arg = &ack_topic;
    reply.expected_acknowledge_id = settings->ack_id;

    reply.reply_id = settings->expected_reply_id;

    sfsc_ChannelFactoryReply channel_factory_reply =
        sfsc_ChannelFactoryReply_init_default;
    sfsc_buffer topic_buffer;
    encode_submsg_to_bytes submsg = {sfsc_ChannelFactoryReply_fields,
                                     &channel_factory_reply};
    if (settings->is_channel) {
        configure_descriptor_for_channel_answer(
            &channel_factory_reply.service_descriptor,
            settings->content.channel_answer, &topic_buffer);
        reply.reply_payload.funcs.encode = b_encode_submsg_to_bytes_callback;
        reply.reply_payload.arg = &submsg;
    } else {
        reply.reply_payload.funcs.encode = b_encode_buffer_for_pb;
        reply.reply_payload.arg = settings->content.payload;
    }

    if (b_encode_and_publish(settings->reply_topic.content,
                             settings->reply_topic.length, &(adapter->data_pub),
                             sfsc_Reply_fields, &reply)) {
        return ZMTP_OK;
    } else {
        return E_PROTO_ENCODE;
    }
}

sfsc_int8 sfsc_internal_register_server(
    sfsc_adapter_stats* stats, _sfsc_adapter_data* adapter,
    _sfsc_commands* commands, sfsc_server* servers[], sfsc_server* server,
    sfsc_buffer name, sfsc_buffer custom_tags, sfsc_buffer output_message_type,
    sfsc_buffer input_message_type, sfsc_command_callback* callback) {
    sfsc_size i = 0;
    for (; i < MAX_SERVERS; i++) {
        if (servers[i] == NULL) {
            break;
        }
    }
    if (i < MAX_SERVERS) {
        servers[i] = server;

        random_uuid_if_needed((sfsc_uint8*)server->service_id.id);

        sfsc_CommandRequest request = sfsc_CommandRequest_init_default;
        sfsc_SfscServiceDescriptor* create =
            &request.create_or_delete.create_request;

        create->service_name.funcs.encode = b_encode_buffer_for_pb;
        create->service_name.arg = &name;
        create->custom_tags.funcs.encode = b_encode_buffer_for_pb;
        create->custom_tags.arg = &custom_tags;

        create->service_tags.which_serviceTags =
            sfsc_SfscServiceDescriptor_ServiceTags_server_tags_tag;

        sfsc_publisher_or_server as_service =
            sfsc_publisher_or_server_INIT_DEFAULT;
        as_service.is_server = 1;
        as_service.service.server = server;
        sfsc_SfscServiceDescriptor_ServiceTags_ServerTags* server_tags =
            &create->service_tags.serviceTags.server_tags;

        server_tags->ack_settings.send_max_tries =
            server->ack_settings.send_max_tries;
        server_tags->ack_settings.timeout_ms = server->ack_settings.timeout_ms;

        server_tags->input_message_type.type.funcs.encode =
            b_encode_buffer_for_pb;
        server_tags->input_message_type.type.arg = &input_message_type;

        server_tags->output_message_type.type.funcs.encode =
            b_encode_buffer_for_pb;
        server_tags->output_message_type.type.arg = &output_message_type;

        server_tags->input_topic.topic.funcs.encode = b_encode_buffer_for_pb;
        sfsc_buffer input_topic = topic_or_autogen(server);
        server_tags->input_topic.topic.arg = &input_topic;

        sfsc_int8 op_result = write_subscription_message(
            &(adapter->data_sub), input_topic.content, input_topic.length, 1);
        if (op_result == ZMTP_OK) {
            op_result = execute_command(stats, adapter, commands, as_service,
                                        &request, 1, callback);
            if (op_result == ZMTP_OK) {
                return ZMTP_OK;
            } else {
                servers[i] = NULL;
                return op_result;
            }
        } else {
            return op_result;
        }
    } else {
        return E_NO_FREE_SLOT;
    }
}
sfsc_int8 sfsc_internal_unregister_server(sfsc_adapter_stats* stats,
                                          _sfsc_adapter_data* adapter,
                                          _sfsc_commands* commands,
                                          sfsc_server* servers[],
                                          sfsc_server* server,
                                          sfsc_command_callback* callback) {
    sfsc_size i = 0;
    for (; i < MAX_SERVERS; i++) {
        if (servers[i] == server) {
            break;
        }
    }
    if (i < MAX_SERVERS) {
        servers[i] = NULL;

        sfsc_CommandRequest request = sfsc_CommandRequest_init_default;
        sfsc_publisher_or_server as_service =
            sfsc_publisher_or_server_INIT_DEFAULT;
        as_service.is_server = 1;
        as_service.service.server = server;
        sfsc_buffer topic = topic_or_autogen(server);
        sfsc_int8 op_result = write_subscription_message(
            &(adapter->data_sub), topic.content, topic.length, 0);
        if (op_result == ZMTP_OK) {
            return execute_command(stats, adapter, commands, as_service,
                                   &request, 0, callback);
        } else {
            return op_result;
        }
    } else {
        return ZMTP_OK;
    }
}

static sfsc_int8 generic_answer_request(
    sfsc_adapter_stats* stats, _sfsc_adapter_data* adapter,
    _sfsc_acks* pending_acks, sfsc_server* server, sfsc_int32 expected_reply_id,
    sfsc_buffer reply_topic, sfsc_uint8 is_channel_answer,
    void* payload_or_channel_answer, void* arg,
    sfsc_answer_ack_callback* on_ack) {
    if (server->ack_settings.send_max_tries > 0) {
        for (sfsc_size i = 0; i < MAX_PENDING_ACKS; i++) {
            if (pending_acks->callbacks[i].ack_id == _ACK_ID_UNUSED) {
                _sfsc_pending_ack* ack = &(pending_acks->callbacks[i]);
                ack->ack_id = ++pending_acks->ack_id_counter;  // preincrement
                ack->ack_settings = server->ack_settings;
                ack->arg = arg;
                ack->server = server;
                ack->on_ack = on_ack;
                ack->expected_reply_id = expected_reply_id;
                if (is_channel_answer) {
                    ack->content.channel_answer =
                        (sfsc_channel_answer*)payload_or_channel_answer;
                    ack->is_channel = 1;
                } else {
                    ack->content.payload =
                        (sfsc_buffer*)payload_or_channel_answer;
                    ack->is_channel = 0;
                }
                ack->reply_topic = reply_topic;
                ack->tries = 0;
                ack->valid_until = time_ms() + server->ack_settings.timeout_ms;
                return send_server_reply(stats, adapter, ack);
            }
        }
        return E_NO_FREE_SLOT;
    } else {
        _sfsc_pending_ack tmp = _sfsc_pending_ack_DEFAULT_INIT;
        if (is_channel_answer) {
            tmp.content.channel_answer =
                (sfsc_channel_answer*)payload_or_channel_answer;
            tmp.is_channel = 1;
        } else {
            tmp.content.payload = (sfsc_buffer*)payload_or_channel_answer;
            tmp.is_channel = 0;
        }
        return send_server_reply(stats, adapter, &tmp);
    }
}

sfsc_int8 user_task_ack(sfsc_adapter* forward_pointer,
                        _sfsc_adapter_data* adapter, _sfsc_acks* pending_acks) {
    sfsc_uint64 now = time_ms();
    sfsc_bool b_trigger = 0;
    for (sfsc_size i = 0; i < MAX_PENDING_ACKS; i++) {
        _sfsc_pending_ack* ack = &(pending_acks->callbacks[i]);
        if (ack->ack_id != _ACK_ID_UNUSED) {
            if (ack->valid_until == _ACK_TRIGGERED) {
                b_trigger = 1;
            } else if (ack->valid_until < now) {
                if (ack->tries < ack->ack_settings.send_max_tries) {
                    b_trigger = 0;
                    sfsc_int8 op_result = send_server_reply(
                        adapter_stats(forward_pointer), adapter, ack);
                    if (op_result != ZMTP_OK) {
                        return op_result;
                    }
                } else {
                    b_trigger = 1;
                }
            } else {
                b_trigger = 0;
            }
            if (b_trigger) {
                if (ack->on_ack != NULL) {
                    ack->on_ack(forward_pointer, ack->server,
                                ack->valid_until != _ACK_TRIGGERED, ack->arg);
                }
                ack->ack_id = _ACK_ID_UNUSED;
            }
        }
    }
    return ZMTP_OK;
}

static sfsc_int8 user_task_invoke_server(sfsc_adapter* forward_pointer,
                                         sfsc_server* server,
                                         sfsc_uint8* request,
                                         sfsc_size request_len,
                                         sfsc_bool* b_auto_advance) {
    if (server->on_request != NULL) {
        sfsc_size payload_len;
        sfsc_buffer reply_topic = sfsc_buffer_DEFAULT_INIT;
        sfsc_int32 reply_id;
        sfsc_uint8* payload;
        if (b_strip_reqrepack_pattern_request(
                request, request_len, &payload, &payload_len, &reply_id,
                &reply_topic, server->is_channel)) {
            sfsc_buffer buf = {payload, payload_len};
            server->on_request(forward_pointer, server, buf, reply_id,
                               reply_topic, b_auto_advance);
            return ZMTP_OK;
        } else {
            return E_PROTO_DECODE;
        }
    } else {
        return ZMTP_OK;
    }
}
sfsc_int8 system_task_data_sub_acks(_sfsc_adapter_data* adapter,
                                    _sfsc_acks* pending_acks,
                                    sfsc_uint8* message) {
    pb_istream_t input =
        pb_istream_from_buffer(message, adapter->data_sub.buffer_index);
    sfsc_RequestOrAcknowledge msg = sfsc_RequestOrAcknowledge_init_default;
    msg.which_request_or_acknowledge =
        sfsc_RequestOrAcknowledge_acknowledge_tag;
    if (pb_decode(&input, sfsc_RequestOrAcknowledge_fields, &msg)) {
        for (sfsc_size i = 0; i < MAX_PENDING_ACKS; i++) {
            _sfsc_pending_ack* ack = &(pending_acks->callbacks[i]);
            if (ack->ack_id ==
                msg.request_or_acknowledge.acknowledge.acknowledge_id) {
                ack->valid_until = _ACK_TRIGGERED;
                return ZMTP_OK;
            }
        }
        return E_UNEXPECTED;
    } else {
        return E_PROTO_DECODE;
    }
}

sfsc_int8 user_task_data_check_servers(
    sfsc_adapter* forward_pointer, sfsc_server* servers[], sfsc_uint8* next_topic, sfsc_size topic_len,
    sfsc_uint8* next_payload, sfsc_size payload_len, sfsc_uint8* consumed,
    sfsc_bool* b_auto_advance) {
    for (sfsc_size i = 0; i < MAX_SERVERS; i++) {
        if (servers[i] != NULL) {
            sfsc_server* server = servers[i];
            sfsc_buffer server_topic = topic_or_autogen(server);
            if (server_topic.length == topic_len &&
                b_bytes_equal(server_topic.content, next_topic, topic_len)) {
                *consumed = 1;
                return user_task_invoke_server(forward_pointer, server,
                                               next_payload, payload_len,
                                               b_auto_advance);
            }
        }
    }
    return ZMTP_OK;
}

sfsc_int8 sfsc_internal_answer_channel_request(
    sfsc_adapter_stats* stats, _sfsc_adapter_data* adapter,
    _sfsc_acks* pending_acks, sfsc_server* server, sfsc_int32 expected_reply_id,
    sfsc_buffer reply_topic, sfsc_channel_answer* answer_data, void* arg,
    sfsc_answer_ack_callback* on_ack) {
    return generic_answer_request(stats, adapter, pending_acks, server,
                                  expected_reply_id, reply_topic, 1,
                                  answer_data, arg, on_ack);
}

sfsc_int8 sfsc_internal_answer_request(
    sfsc_adapter_stats* stats, _sfsc_adapter_data* adapter,
    _sfsc_acks* pending_acks, sfsc_server* server, sfsc_int32 expected_reply_id,
    sfsc_buffer reply_topic, sfsc_buffer* payload, void* arg,
    sfsc_answer_ack_callback* on_ack) {
    return generic_answer_request(stats, adapter, pending_acks, server,
                                  expected_reply_id, reply_topic, 0, payload,
                                  arg, on_ack);
}
