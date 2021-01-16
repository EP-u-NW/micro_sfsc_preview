/*
 * sfsc_adapter_connect.c
 *
 *  Created on: 12.10.2020
 *      Author: Eric
 */

#include "sfsc_connect.h"

#include "../platform/sfsc_sockets.h"
#include "../proto/pb.h"
#include "../proto/pb_decode.h"
#include "../zmtp/zmtp.h"
#include "../zmtp/zmtp_socket_types.h"
#include "sfsc_constants.h"
#include "sfsc_utils.h"

static sfsc_int8 start_session_after_sub(sfsc_adapter_stats* stats,
                                         _sfsc_adapter_data* adapter,
                                         _sfsc_heartbeats* heartbeats,
                                         int original_control_sub_port,
                                         int original_data_pub_port,
                                         int original_data_sub_port) {
    random_uuid_if_needed(stats->adapter_id);
    sfsc_int8 op_result = prepare_heartbeat_message(stats, adapter, heartbeats);
    if (op_result == ZMTP_OK) {
        stats->state = SFSC_STATE_SETUP_CONNECTING;
        sfsc_int16 socket_handle =
            socket_connect(stats->address, original_control_sub_port);
        if (socket_handle < SOCKET_OK) {
            return socket_handle;
        }
        op_result = zmtp_connect(&(adapter->control_pub), socket_handle,
                                 MECHANISM_NULL, NULL, 0, ZMTP_SOCKET_TYPE_PUB);
        if (op_result != ZMTP_OK) {
            return op_result;
        }

        socket_handle = socket_connect(stats->address, original_data_pub_port);
        if (socket_handle < SOCKET_OK) {
            return socket_handle;
        }
        op_result = zmtp_connect(&(adapter->data_sub), socket_handle,
                                 MECHANISM_NULL, NULL, 0, ZMTP_SOCKET_TYPE_SUB);
        if (op_result != ZMTP_OK) {
            return op_result;
        }

        socket_handle = socket_connect(stats->address, original_data_sub_port);
        if (socket_handle < SOCKET_OK) {
            return socket_handle;
        }
        op_result = zmtp_connect(&(adapter->data_pub), socket_handle,
                                 MECHANISM_NULL, NULL, 0, ZMTP_SOCKET_TYPE_PUB);
        if (op_result != ZMTP_OK) {
            return op_result;
        }

        return ZMTP_OK;
    } else {
        return op_result;
    }
}

sfsc_int8 sfsc_internal_start_session(
    sfsc_adapter_stats* stats, _sfsc_adapter_data* adapter,
    _sfsc_heartbeats* heartbeats, const char* address,
    int original_control_pub_port, int original_control_sub_port,
    int original_data_pub_port, int original_data_sub_port) {
    sfsc_int16 socket_handle =
        socket_connect(address, original_control_pub_port);
    if (socket_handle < SOCKET_OK) {
        return socket_handle;
    }
    stats->address = address;
    sfsc_int8 op_result =
        zmtp_connect(&(adapter->control_sub), socket_handle, MECHANISM_NULL,
                     NULL, 0, ZMTP_SOCKET_TYPE_SUB);
    if (op_result == ZMTP_OK) {
        return start_session_after_sub(
            stats, adapter, heartbeats, original_control_sub_port,
            original_data_pub_port, original_data_sub_port);
    } else {
        return op_result;
    }
}

sfsc_int8 sfsc_internal_start_session_bootstraped(
    sfsc_adapter_stats* stats, _sfsc_adapter_data* adapter, const char* address,
    int original_control_pub_port) {
    stats->address = address;
    sfsc_int16 socket_handle =
        socket_connect(address, original_control_pub_port);
    if (socket_handle < SOCKET_OK) {
        return socket_handle;
    }
    sfsc_int8 op_result =
        zmtp_connect(&(adapter->control_sub), socket_handle, MECHANISM_NULL,
                     NULL, 0, ZMTP_SOCKET_TYPE_SUB);
    if (op_result == ZMTP_OK) {
        stats->state = SFSC_STATE_BOOTSTRAP_CONNECTING;
        return ZMTP_OK;
    } else {
        return op_result;
    }
}

static sfsc_int8 system_task_setup(sfsc_adapter_stats* stats,
                                   _sfsc_adapter_data* adapter) {
    if (adapter->control_sub.state == ZMTP_STATE_OPERATING &&
        adapter->control_pub.state == ZMTP_STATE_OPERATING &&
        adapter->data_sub.state == ZMTP_STATE_OPERATING &&
        adapter->data_pub.state == ZMTP_STATE_OPERATING) {
        sfsc_buffer adapter_hello_reply_topic_parts[2] = {
            {_welcome_topic, _WELCOME_TOPIC_LEN},
            {stats->adapter_id, UUID_LEN}};
        sfsc_composite_buffer adapter_hello_reply_topic = {
            adapter_hello_reply_topic_parts, 2};
        sfsc_int8 write_result;
        if (stats->state == SFSC_STATE_SETUP_CONNECTING) {
            // subscribe welcome
            write_result = write_composite_subscription_message(
                &(adapter->control_sub), &adapter_hello_reply_topic, 1);
            if (write_result != ZMTP_OK) {
                return write_result;
            }

            write_result =
                zmtp_flush(&(adapter->control_sub));  // TODO experimental
            if (write_result != ZMTP_OK) {
                return write_result;
            }

            // Set up hello message
            sfsc_Hello hello = sfsc_Hello_init_default;
            sfsc_copy((sfsc_uint8*)hello.adapter_id.id, stats->adapter_id,
                      UUID_LEN);

            sfsc_buffer adapter_heartbeat_topic_parts[2] = {
                {_heartbeat_topic, _HEARTBEAT_TOPIC_LEN},
                {stats->adapter_id, UUID_LEN}};

            sfsc_composite_buffer adapter_heartbeat_topic = {
                adapter_heartbeat_topic_parts, 2};
            hello.heartbeat_topic.topic.funcs.encode =
                b_encode_composite_buffer_for_pb;
            hello.heartbeat_topic.topic.arg = &adapter_heartbeat_topic;

            if (b_encode_and_publish_with_request_composite_reply_pattern(
                    _hello_topic, _HELLO_TOPIC_LEN, &adapter_hello_reply_topic,
                    _REPLY_ID_UNINTERESTED, &(adapter->control_pub),
                    sfsc_Hello_fields, &hello)) {
                stats->state = SFSC_STATE_SETUP_AWAIT_WELCOME;
                return ZMTP_OK;
            } else {
                return E_PROTO_ENCODE;
            }

        } else if (stats->state == SFSC_STATE_SETUP_AWAIT_WELCOME) {
            sfsc_uint8* message = get_message(&(adapter->control_sub));
            if (message != NULL) {
                if (adapter->control_sub.is_message == 1 &&
                    adapter->control_sub.last_message == 0 &&
                    b_composite_bytes_equal(
                        &adapter_hello_reply_topic, message,
                        adapter->control_sub.buffer_index)) {
                    zmtp_pop(&(adapter->control_sub));
                    stats->state = SFSC_STATE_SETUP_RECEIVE_WELCOME;
                    return ZMTP_OK;
                } else {
                    return E_UNEXPECTED;  // TODO consistency with bootstrap
                }
            } else {
                return ZMTP_OK;
            }
        } else if (stats->state == SFSC_STATE_SETUP_RECEIVE_WELCOME) {
            sfsc_uint8* message = get_message(&(adapter->control_sub));
            if (message != NULL) {
                if (adapter->control_sub.is_message == 1 &&
                    adapter->control_sub.last_message == 1) {
                    write_result = write_composite_subscription_message(
                        &(adapter->control_sub), &adapter_hello_reply_topic, 0);
                    if (write_result != ZMTP_OK) {
                        return write_result;
                    }

                    sfsc_buffer adapter_heartbeat_topic_parts[2] = {
                        {_heartbeat_topic, _HEARTBEAT_TOPIC_LEN},
                        {stats->adapter_id, UUID_LEN}};

                    sfsc_composite_buffer adapter_heartbeat_topic = {
                        adapter_heartbeat_topic_parts, 2};
                    write_result = write_composite_subscription_message(
                        &(adapter->control_sub), &adapter_heartbeat_topic, 1);
                    if (write_result != ZMTP_OK) {
                        return write_result;
                    }

                    sfsc_Welcome welcome = sfsc_Welcome_init_default;
                    sfsc_int64 reply_id;
                    sfsc_size stripped_length;
                    sfsc_uint8* stripped;
                    if (b_strip_request_reply_pattern(
                            message, adapter->control_sub.buffer_index,
                            &stripped, &stripped_length, &reply_id)) {
                        pb_istream_t input =
                            pb_istream_from_buffer(stripped, stripped_length);
                        if (pb_decode(&input, sfsc_Welcome_fields, &welcome)) {
                            zmtp_pop(&(adapter->control_sub));
                            sfsc_copy(stats->core_id,
                                      (sfsc_uint8*)welcome.core_id.id,
                                      UUID_LEN);

                            sfsc_buffer sub_topics_parts[2] = {
                                {_data_request_reply_topic,
                                 _DATA_REQUEST_REPLY_TOPIC_LEN},
                                {stats->adapter_id, UUID_LEN}};
                            sfsc_composite_buffer sub_topic = {sub_topics_parts,
                                                               2};
                            write_result = write_composite_subscription_message(
                                &(adapter->data_sub), &sub_topic, 1);
                            if (write_result != ZMTP_OK) {
                                return write_result;
                            }
                            sub_topics_parts[0].content = _data_ack_topic;
                            sub_topics_parts[0].length = _DATA_ACK_TOPIC_LEN;
                            write_result = write_composite_subscription_message(
                                &(adapter->data_sub), &sub_topic, 1);
                            if (write_result != ZMTP_OK) {
                                return write_result;
                            }
                            sub_topics_parts[0].content = _command_topic;
                            sub_topics_parts[0].length = _COMMAND_TOPIC_LEN;
                            write_result = write_composite_subscription_message(
                                &(adapter->control_sub), &sub_topic, 1);
                            if (write_result != ZMTP_OK) {
                                return write_result;
                            }
                            sub_topics_parts[0].content = _query_topic;
                            sub_topics_parts[0].length = _QUERY_TOPIC_LEN;
                            write_result = write_composite_subscription_message(
                                &(adapter->control_sub), &sub_topic, 1);
                            if (write_result != ZMTP_OK) {
                                return write_result;
                            }
                            stats->state = SFSC_STATE_OPERATIONAL;
                            return ZMTP_OK;
                        } else {
                            return E_PROTO_DECODE;
                        }
                    } else {
                        return E_PROTO_DECODE;
                    }
                } else {
                    return E_UNEXPECTED;
                }
            } else {
                return ZMTP_OK;
            }
        } else {
            return E_UNEXPECTED;
        }
    } else {
        return ZMTP_OK;
    }
}

static sfsc_int8 system_task_bootstraping(sfsc_adapter_stats* stats,
                                          _sfsc_adapter_data* adapter,
                                          _sfsc_heartbeats* heartbeats) {
    sfsc_int8 op_result = zmtp_task(&(adapter->control_sub));
    sfsc_int8 write_result;
    if (op_result == ZMTP_OK) {
        if (adapter->control_sub.state == ZMTP_STATE_OPERATING) {
            if (stats->state < SFSC_STATE_BOOTSTRAP_SUBSCRIBED) {
                write_result = write_subscription_message(
                    &(adapter->control_sub), _bootstrap_topic,
                    _BOOTSTRAP_TOPIC_LEN, 1);
                if (write_result != ZMTP_OK) {
                    return write_result;
                }
                
                stats->state = SFSC_STATE_BOOTSTRAP_SUBSCRIBED;
                return ZMTP_OK;
            } else {
                sfsc_uint8* message = get_message(&(adapter->control_sub));
                if (message != NULL) {
                    if (stats->state ==
                        SFSC_STATE_BOOTSTRAP_SUBSCRIBED_DISCARD_NEXT) {  // TODO
                                                                         // consistency
                                                                         // with
                                                                         // E_UNEXPECTED
                                                                         // when
                                                                         // receiving
                                                                         // welcome
                        stats->state = SFSC_STATE_BOOTSTRAP_SUBSCRIBED;
                        zmtp_pop(&(adapter->control_sub));
                        return ZMTP_OK;
                    } else if (stats->state ==
                               SFSC_STATE_BOOTSTRAP_SUBSCRIBED) {
                        // Expect something on the bootstrap topic only
                        if (adapter->control_sub.is_message == 1 &&
                            adapter->control_sub.last_message == 0 &&
                            _BOOTSTRAP_TOPIC_LEN ==
                                adapter->control_sub.buffer_index &&
                            b_bytes_equal(_bootstrap_topic, message,
                                          adapter->control_sub.buffer_index)) {
                            stats->state = SFSC_STATE_BOOTSTRAP_RECEIVEING;
                        } else {
                            stats->state =
                                SFSC_STATE_BOOTSTRAP_SUBSCRIBED_DISCARD_NEXT;
                        }
                        zmtp_pop(&(adapter->control_sub));
                        return ZMTP_OK;
                    } else if (stats->state ==
                               SFSC_STATE_BOOTSTRAP_RECEIVEING) {
                        if (adapter->control_sub.is_message == 1 &&
                            adapter->control_sub.last_message == 1) {
                            sfsc_BootstrapMessage msg =
                                sfsc_BootstrapMessage_init_default;
                            pb_istream_t input = pb_istream_from_buffer(
                                message, adapter->control_sub.buffer_index);
                            if (pb_decode(&input, sfsc_BootstrapMessage_fields,
                                          &msg)) {
                                write_result = write_subscription_message(
                                    &(adapter->control_sub), _bootstrap_topic,
                                    _BOOTSTRAP_TOPIC_LEN, 0);
                                if (write_result != ZMTP_OK) {
                                    return write_result;
                                }
                                zmtp_pop(&(adapter->control_sub));
                                return start_session_after_sub(
                                    stats, adapter, heartbeats,
                                    msg.core_control_sub_tcp_port,
                                    msg.core_data_pub_tcp_port,
                                    msg.core_data_sub_tcp_port);
                            } else {
                                return E_PROTO_DECODE;
                            }
                        } else {
                            return E_UNEXPECTED;
                        }
                    } else {
                        return E_UNEXPECTED;
                    }
                } else {
                    return ZMTP_OK;
                }
            }
        } else {
            return ZMTP_OK;
        }
    } else {
        return op_result;
    }
}

sfsc_int8 system_task_connect(sfsc_adapter_stats* stats,
                              _sfsc_adapter_data* adapter,
                              _sfsc_heartbeats* heartbeats) {
    if (stats->state < SFSC_STATE_SETUP) {
        return system_task_bootstraping(stats, adapter, heartbeats);
    } else if (stats->state == SFSC_STATE_SETUP) {
        return E_UNREACHABLE;
    } else if (stats->state < SFSC_STATE_OPERATIONAL) {
        sfsc_int8 op_result = process_io(adapter);
        if (op_result == ZMTP_OK) {
            if (adapter->control_pub.state == ZMTP_STATE_OPERATING) {
                // op_result = handle_outgoing_heartbeats(adapter); //TODO
                // experimental, do I even need this befor welcome? Wait for
                // Chris answer
                if (op_result == ZMTP_OK) {
                    return system_task_setup(stats, adapter);
                } else {
                    return op_result;
                }
            } else {
                return system_task_setup(stats, adapter);
            }
        } else {
            return op_result;
        }
    } else {
        return E_UNREACHABLE;
    }
}
