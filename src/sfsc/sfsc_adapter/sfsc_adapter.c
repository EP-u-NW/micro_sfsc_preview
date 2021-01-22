/*
 * sfsc_adapter.c
 *
 *  Created on: 02.09.2020
 *      Author: Eric
 */
#include "sfsc_adapter.h"

#include "../platform/sfsc_platform.h"
#include "sfsc_adapter_struct.h"
#include "sfsc_utils.h"

#define _NEXT_PAYLOAD_TYPE_IGNORE 0
#define _NEXT_PAYLOAD_TYPE_REQ_OR_SUB 1
#define _NEXT_PAYLOAD_TYPE_REPLY 2
#define _NEXT_PAYLOAD_TYPE_ACK 3

const sfsc_buffer sfsc_buffer_default = sfsc_buffer_DEFAULT_INIT;

sfsc_adapter_stats* adapter_stats(sfsc_adapter* adapter) {
    return &adapter->stats;
}

sfsc_int8 release_session(sfsc_adapter* adapter){
    sfsc_int8 op_result;
    op_result=zmtp_release(&(adapter->data.control_pub));
    if(op_result!=ZMTP_OK){
        return op_result;
    }
    op_result=zmtp_release(&(adapter->data.control_sub));
        if(op_result!=ZMTP_OK){
        return op_result;
    }
    op_result=zmtp_release(&(adapter->data.data_pub));
        if(op_result!=ZMTP_OK){
        return op_result;
    }
    op_result=zmtp_release(&(adapter->data.data_sub));
        if(op_result!=ZMTP_OK){
        return op_result;
    }
    adapter->stats.state=SFSC_STATE_NONE;
    return ZMTP_OK;
}

static sfsc_int8 system_task_read_data_sub(sfsc_adapter* adapter) {
    sfsc_uint8* message = get_message(&(adapter->data.data_sub));
    if (message != NULL) {
        sfsc_buffer reply_topic_parts[2] = {
            {_data_request_reply_topic, _DATA_REQUEST_REPLY_TOPIC_LEN},
            {adapter->stats.adapter_id, UUID_LEN}};
        sfsc_composite_buffer reply_topic = {reply_topic_parts, 2};

        sfsc_buffer ack_topic_parts[2] = {
            {_data_ack_topic, _DATA_ACK_TOPIC_LEN},
            {adapter->stats.adapter_id, UUID_LEN}};
        sfsc_composite_buffer ack_topic = {ack_topic_parts, 2};

        sfsc_int8 op_result;
        if (adapter->data.data_sub.is_message == 1 &&
            adapter->data.data_sub.last_message == 0 &&
            adapter->data.user_buffer.next_is_payload == 0) {
            adapter->data.user_buffer.next_is_payload = 1;
            if (b_composite_bytes_equal(&ack_topic, message,
                                        adapter->data.data_sub.buffer_index)) {
                adapter->data.next_payload_type = _NEXT_PAYLOAD_TYPE_ACK;
                op_result = ZMTP_OK;
            } else if (b_write_topic(&(adapter->data.user_buffer), message,
                                     adapter->data.data_sub.buffer_index)) {
                adapter->data.next_payload_type = _NEXT_PAYLOAD_TYPE_REQ_OR_SUB;
                if (b_composite_bytes_equal(
                        &reply_topic, message,
                        adapter->data.data_sub.buffer_index)) {
                    adapter->data.next_payload_type = _NEXT_PAYLOAD_TYPE_REPLY;
                }
                op_result = ZMTP_OK;
            } else {
                adapter->data.next_payload_type = _NEXT_PAYLOAD_TYPE_IGNORE;
                op_result = W_MESSAGE_DROP;
            }
        } else if (adapter->data.data_sub.is_message == 1 &&
                   adapter->data.data_sub.last_message == 1 &&
                   adapter->data.user_buffer.next_is_payload == 1) {
            adapter->data.user_buffer.next_is_payload = 0;

            switch (adapter->data.next_payload_type) {
                case _NEXT_PAYLOAD_TYPE_IGNORE:
                    op_result = ZMTP_OK;
                    break;
                case _NEXT_PAYLOAD_TYPE_ACK:
                    op_result = system_task_data_sub_acks(
                        &adapter->data, &adapter->pending_acks, message);
                    if (op_result == E_UNEXPECTED) {
                        // TODO maybe just ignore it since its
                        // an answer to a request we  dont care about?
                        op_result = ZMTP_OK;
                    }
                    break;
                case _NEXT_PAYLOAD_TYPE_REPLY:
                    op_result = system_task_data_sub_receive_reply(
                        &adapter->data, &adapter->requests, message);
                    if (op_result == E_UNEXPECTED) {
                        // TODO maybe just ignore it since its
                        // an answer to a request we  dont care about?
                        op_result = ZMTP_OK;
                    }
                    break;
                case _NEXT_PAYLOAD_TYPE_REQ_OR_SUB:
                    if (b_write_payload(&(adapter->data.user_buffer), message,
                                        adapter->data.data_sub.buffer_index)) {
                        op_result = ZMTP_OK;
                    } else {
                        forget_last_written_if_topic(
                            &(adapter->data.user_buffer));
                        op_result = W_MESSAGE_DROP;
                    }
                    break;
                default:
                    op_result = E_UNREACHABLE;
                    break;
            }
        } else {
            return E_UNEXPECTED;
        }
        return op_result;
    } else {
        return ZMTP_OK;
    }
}

sfsc_int8 system_task(sfsc_adapter* adapter) {
    if (adapter->stats.state == SFSC_STATE_NONE) {
        // DO NOTHING
        return ZMTP_OK;
    } else if (adapter->stats.state < SFSC_STATE_OPERATIONAL) {
        return system_task_connect(adapter_stats(adapter), &adapter->data,
                                   &adapter->heartbeat_info);
    } else if (adapter->stats.state >= SFSC_STATE_OPERATIONAL) {
        sfsc_int8 op_result = process_io(&adapter->data);
        if (op_result == ZMTP_OK) {
            op_result = handle_incoming_heartbeats(adapter_stats(adapter),
                                                   &adapter->data,
                                                   &adapter->heartbeat_info);
            if (op_result == ZMTP_OK) {
                op_result = handle_outgoing_heartbeats(
                    &adapter->data, &adapter->heartbeat_info);
                if (op_result == ZMTP_OK) {
                    op_result =
                        system_task_query(adapter_stats(adapter),
                                          &adapter->data, &adapter->queries);
                    if (op_result == ZMTP_OK) {
                        op_result = system_task_commands(adapter_stats(adapter),
                                                         &adapter->data,
                                                         &adapter->commands);
                        if (op_result == ZMTP_OK) {
                            op_result = system_task_publisher(
                                &adapter->data, adapter->publishers);
                            if (op_result == ZMTP_OK) {
                                op_result = system_task_read_data_sub(adapter);
                                if (op_result == W_MESSAGE_DROP) {
                                    adapter->stats.discarded_message_count++;
                                }
                                if (HAS_MESSAGE(&(adapter->data.control_sub))) {
                                    zmtp_pop(
                                        &(adapter->data.control_sub));  // POP
                                }
                                if (HAS_MESSAGE(&(adapter->data.control_pub))) {
                                    zmtp_pop(
                                        &(adapter->data.control_pub));  // POP
                                }
                                if (HAS_MESSAGE(&(adapter->data.data_pub))) {
                                    zmtp_pop(&(adapter->data.data_pub));  // POP
                                }
                                if (HAS_MESSAGE(&(adapter->data.data_sub))) {
                                    zmtp_pop(&(adapter->data.data_sub));  // POP
                                }
                                return op_result;
                            } else {
                                return op_result;
                            }
                        } else {
                            return op_result;
                        }
                    } else {
                        return op_result;
                    }
                } else {
                    return op_result;
                }
            } else {
                return op_result;
            }
        } else {
            return op_result;
        }
    } else {
        return E_UNREACHABLE;
    }
}

void advance_user_ring(sfsc_adapter* adapter) {
    if (adapter->data.b_need_user_ring_advance == 1) {
        advance_topic_and_payload(&(adapter->data.user_buffer));
        adapter->data.b_need_user_ring_advance = 0;
    }
}

#define _MIN_DATA_REPREQ_USER_RING_PAYLOAD_LEN \
    1  // 1 byte header+ min 0 byte actual payload
static sfsc_int8 user_task_data(sfsc_adapter* adapter) {
    sfsc_bool b_auto_advance = 1;
    sfsc_size topic_len;
    sfsc_size payload_len;
    sfsc_uint8* next_topic;
    sfsc_uint8* next_payload;
    sfsc_int8 op_result = ZMTP_OK;
    sfsc_buffer reply_topic_parts[2] = {
        {_data_request_reply_topic, _DATA_REQUEST_REPLY_TOPIC_LEN},
        {adapter->stats.adapter_id, UUID_LEN}};
    sfsc_composite_buffer reply_topic = {reply_topic_parts, 2};
#if REPLAYS_PER_TASK > 0
    for (sfsc_size i = 0;
         i < REPLAYS_PER_TASK && adapter->data.b_need_user_ring_advance == 0;
         i++) {
#else
    while (adapter->data.b_need_user_ring_advance == 0) {
#endif
        b_auto_advance = 1;
        next_topic = read_topic(&(adapter->data.user_buffer), &topic_len);
        if (next_topic != NULL) {
            next_payload =
                read_payload(&(adapter->data.user_buffer), &payload_len);
            if (next_payload != NULL) {
                if (payload_len >= _MIN_DATA_REPREQ_USER_RING_PAYLOAD_LEN &&
                    b_composite_bytes_equal(&reply_topic, next_topic,
                                            topic_len)) {
                    op_result = user_task_data_handle_reply(
                        adapter, &adapter->requests,
                        next_payload, payload_len, &b_auto_advance);
                } else {
                    sfsc_uint8 consumed = 0;
                    op_result = user_task_data_check_subscribers(
                        adapter, adapter->subscribers, next_topic, topic_len,
                        next_payload, payload_len, &consumed, &b_auto_advance);
                    if (!consumed) {
                        op_result = user_task_data_check_servers(
                            adapter, adapter->servers,
                            next_topic, topic_len, next_payload, payload_len,
                            &consumed, &b_auto_advance);
                    }
                    // Maybe check consumed and print something if not?
                }
                if (b_auto_advance) {
                    advance_topic_and_payload(&(adapter->data.user_buffer));
                    adapter->data.b_need_user_ring_advance = 0;
                } else {
                    adapter->data.b_need_user_ring_advance = 1;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }
    return op_result;
}

sfsc_int8 user_task(sfsc_adapter* adapter) {
    sfsc_int8 op_result =
        user_task_query(adapter, &adapter->queries);
    if (op_result == ZMTP_OK) {
        op_result = user_task_commands(adapter, &adapter->commands);
        if (op_result == ZMTP_OK) {
            op_result = user_task_publisher(adapter, adapter->publishers);
            if (op_result == ZMTP_OK) {
                op_result = user_task_ack(adapter, &adapter->data,
                                          &adapter->pending_acks);
                if (op_result == ZMTP_OK) {
                    op_result = user_task_data(adapter);
                    if (op_result == ZMTP_OK) {
                        user_task_request_timeout(adapter, &adapter->requests);
                        return ZMTP_OK;
                    } else {
                        return op_result;
                    }
                } else {
                    return op_result;
                }
            } else {
                return op_result;
            }
        } else {
            return op_result;
        }
    } else {
        return op_result;
    }
}

const sfsc_uint8 _hex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

#define _HEX_MASK 0x0F
void random_uuid(sfsc_uint8 target[36]) {
    sfsc_uint8 bytes[16];
    random_bytes(bytes, 16);
    target[0] = _hex[bytes[0] & _HEX_MASK];
    target[1] = _hex[bytes[0] >> 4];
    target[2] = _hex[bytes[1] & _HEX_MASK];
    target[3] = _hex[bytes[1] >> 4];
    target[4] = _hex[bytes[2] & _HEX_MASK];
    target[5] = _hex[bytes[2] >> 4];
    target[6] = _hex[bytes[3] & _HEX_MASK];
    target[7] = _hex[bytes[3] >> 4];
    target[8] = 45;
    target[9] = _hex[bytes[4] & _HEX_MASK];
    target[10] = _hex[bytes[4] >> 4];
    target[11] = _hex[bytes[5] & _HEX_MASK];
    target[12] = _hex[bytes[5] >> 4];
    target[13] = 45;
    target[14] = _hex[bytes[6] & _HEX_MASK];
    target[15] = _hex[bytes[6] >> 4];
    target[16] = _hex[bytes[7] & _HEX_MASK];
    target[17] = _hex[bytes[7] >> 4];
    target[18] = 45;
    target[19] = _hex[bytes[8] & _HEX_MASK];
    target[20] = _hex[bytes[8] >> 4];
    target[21] = _hex[bytes[9] & _HEX_MASK];
    target[22] = _hex[bytes[9] >> 4];
    target[23] = 45;
    target[24] = _hex[bytes[10] & _HEX_MASK];
    target[25] = _hex[bytes[10] >> 4];
    target[26] = _hex[bytes[11] & _HEX_MASK];
    target[27] = _hex[bytes[11] >> 4];
    target[28] = _hex[bytes[12] & _HEX_MASK];
    target[29] = _hex[bytes[12] >> 4];
    target[30] = _hex[bytes[13] & _HEX_MASK];
    target[31] = _hex[bytes[13] >> 4];
    target[32] = _hex[bytes[14] & _HEX_MASK];
    target[33] = _hex[bytes[14] >> 4];
    target[34] = _hex[bytes[15] & _HEX_MASK];
    target[35] = _hex[bytes[15] >> 4];
}
#undef _HEX_MASK

sfsc_int8 start_session_bootstraped(sfsc_adapter* adapter, const char* address,
                                    int original_control_pub_port) {
    return sfsc_internal_start_session_bootstraped(adapter_stats(adapter),
                                                   &adapter->data, address,
                                                   original_control_pub_port);
}

sfsc_int8 start_session(sfsc_adapter* adapter, const char* address,
                        int original_control_pub_port,
                        int original_control_sub_port,
                        int original_data_pub_port,
                        int original_data_sub_port) {
    return sfsc_internal_start_session(
        adapter_stats(adapter), &adapter->data, &adapter->heartbeat_info,
        address, original_control_pub_port, original_control_sub_port,
        original_data_pub_port, original_data_sub_port);
}

sfsc_int8 register_subscriber(sfsc_adapter* adapter,
                              sfsc_subscriber* subscriber) {
    return sfsc_internal_register_subscriber(&adapter->data,
                                             adapter->subscribers, subscriber);
}

sfsc_int8 unregister_subscriber(sfsc_adapter* adapter,
                                sfsc_subscriber* subscriber) {
    return sfsc_internal_unregister_subscriber(
        &adapter->data, adapter->subscribers, subscriber);
}

sfsc_int8 query_services(sfsc_adapter* adapter, sfsc_query_callback* callback) {
    return sfsc_internal_query_services(adapter_stats(adapter), &adapter->data,
                                        &adapter->queries, callback);
}

void query_services_next(sfsc_adapter* adapter, sfsc_bool next) {
    sfsc_internal_query_services_next(adapter_stats(adapter), &adapter->data,
                                      &adapter->queries, next);
}

sfsc_int8 register_publisher_unregistered(sfsc_adapter* adapter,
                                          sfsc_publisher* publisher) {
    sfsc_size i = 0;
    return sfsc_internal_register_publisher_unregistered(adapter->publishers, publisher, &i);
}

sfsc_int8 register_publisher(sfsc_adapter* adapter, sfsc_publisher* publisher,
                             sfsc_buffer name, sfsc_buffer custom_tags,
                             sfsc_buffer output_message_type,
                             sfsc_command_callback* callback) {
    return sfsc_internal_register_publisher(
        adapter_stats(adapter), &adapter->data, &adapter->commands,
        adapter->publishers, publisher, name, custom_tags, output_message_type,
        callback);
}

sfsc_int8 unregister_publisher(sfsc_adapter* adapter, sfsc_publisher* publisher,
                               sfsc_command_callback* callback) {
    return sfsc_internal_unregister_publisher(
        adapter_stats(adapter), &adapter->data, &adapter->commands,
        adapter->publishers, publisher, callback);
}

sfsc_int8 publish(sfsc_adapter* adapter, sfsc_publisher* publisher,
                  sfsc_buffer payload) {
    return sfsc_internal_publish(&adapter->data, publisher, payload);
}

sfsc_int8 request(sfsc_adapter* adapter, sfsc_buffer topic, sfsc_buffer payload,
                  sfsc_uint64 timeout, sfsc_request_callback* callback,
                  void* arg) {
    return sfsc_internal_request(adapter_stats(adapter), &adapter->data,
                                 &adapter->requests, topic, payload, timeout,
                                 callback, arg);
}

sfsc_int8 channel_request(sfsc_adapter* adapter, sfsc_buffer topic,
                          sfsc_buffer payload, sfsc_uint64 timeout,
                          relative_sfsc_service_descriptor* descriptor,
                          sfsc_uint8* descriptor_space,
                          sfsc_size descriptor_space_lenght,
                          sfsc_channel_request_callback* callback, void* arg) {
    return sfsc_internal_channel_request(
        adapter_stats(adapter), &adapter->data, &adapter->requests, topic,
        payload, timeout, descriptor_space, descriptor_space_lenght, descriptor,
        callback, arg);
}

sfsc_int8 register_server(sfsc_adapter* adapter, sfsc_server* server,
                          sfsc_buffer name, sfsc_buffer custom_tags,
                          sfsc_buffer output_message_type,
                          sfsc_buffer input_message_type,
                          sfsc_command_callback* callback) {
    return sfsc_internal_register_server(
        adapter_stats(adapter), &adapter->data, &adapter->commands,
        adapter->servers, server, name, custom_tags, output_message_type,
        input_message_type, callback);
}

sfsc_int8 unregister_server(sfsc_adapter* adapter, sfsc_server* server,
                            sfsc_command_callback* callback) {
    return sfsc_internal_unregister_server(adapter_stats(adapter),
                                           &adapter->data, &adapter->commands,
                                           adapter->servers, server, callback);
}

sfsc_int8 answer_request(sfsc_adapter* adapter, sfsc_server* server,
                         sfsc_int32 expected_reply_id, sfsc_buffer reply_topic,
                         sfsc_buffer* payload, void* arg,
                         sfsc_answer_ack_callback* on_ack) {
    return sfsc_internal_answer_request(
        adapter_stats(adapter), &adapter->data, &adapter->pending_acks, server,
        expected_reply_id, reply_topic, payload, arg, on_ack);
}

sfsc_int8 answer_channel_request(sfsc_adapter* adapter, sfsc_server* server,
                                 sfsc_int32 expected_reply_id,
                                 sfsc_buffer reply_topic,
                                 sfsc_channel_answer* channel_answer, void* arg,
                                 sfsc_answer_ack_callback* on_ack) {
    return sfsc_internal_answer_channel_request(
        adapter_stats(adapter), &adapter->data, &adapter->pending_acks, server,
        expected_reply_id, reply_topic, channel_answer, arg, on_ack);
}
