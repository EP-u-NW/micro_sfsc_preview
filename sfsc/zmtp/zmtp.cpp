/*
 * zmtp.c
 *
 *  Created on: 13.07.2020
 *      Author: Eric
 */

#ifndef ZMTP_C_
#define ZMTP_C_
#include "zmtp.h"
#ifndef NO_CURVE
#include "zmtp_curve.h"
#endif
#include "../platform/sfsc_platform.h"
#include "zmtp_connect.h"
#include "zmtp_io.h"
#include "zmtp_metadata.h"
#include "zmtp_states.h"
#include "zmtp_utils.h"

void zmtp_error_msg(zmtp_socket* socket, const sfsc_uint8** error_msg,
                    sfsc_uint8* error_msg_len) {
    sfsc_uint8* buffer = effective_buffer(socket);
    *error_msg = buffer + 6;
    *error_msg_len = socket->buffer_index - 6;
}

sfsc_int8 write_message(zmtp_socket* socket, const sfsc_uint8 message[],
                        sfsc_size len, sfsc_bool last) {
    sfsc_int8 op_result = begin_message_multipart(socket, len, last);
    if (op_result == ZMTP_OK) {
        return continue_message_multipart(socket, message, len, 1);
    } else {
        return op_result;
    }
}

sfsc_int8 begin_message_multipart(zmtp_socket* socket, sfsc_size len,
                                  sfsc_bool as_last_message) {
    lock_resource(&socket->lock_mem);  // TODO
    socket->writing_last = as_last_message;
    if (socket->mechanism == MECHANISM_CURVE) {
#ifdef NO_CURVE
        return E_CURVE_DISABLED;
#else
        return begin_message_curve(socket, len, as_last_message);
#endif
    } else {
        return write_length_header(
            socket, len, as_last_message ? _FLAG_LAST_MESSAGE : _FLAG_MESSAGE);
    }
}

sfsc_int8 continue_message_multipart(zmtp_socket* socket,
                                     const sfsc_uint8 message[], sfsc_size len,
                                     sfsc_bool end) {
    sfsc_int8 op_result;
    if (socket->mechanism == MECHANISM_CURVE) {
#ifdef NO_CURVE
        op_result = E_CURVE_DISABLED;
#else
        op_result = continue_message_curve(socket, message, len, end);
#endif
    } else {
        op_result = zmtp_write(socket, message, len);
    }
    if (end && socket->writing_last) {
        unlock_resource(&socket->lock_mem);  // TODO
    }
    if (op_result == ZMTP_OK) {
        return ZMTP_OK;
    } else {
        return op_result;
    }
}

sfsc_int8 write_subscription_message(zmtp_socket* socket,
                                     const sfsc_uint8 topic[], sfsc_size len,
                                     sfsc_bool subscribe) {
    sfsc_int8 op_result = begin_message_multipart(socket, len + 1, 1);
    if (op_result == ZMTP_OK) {
        const sfsc_uint8 sub[1] = {1};
        const sfsc_uint8 unsub[1] = {0};
        op_result =
            continue_message_multipart(socket, subscribe ? sub : unsub, 1, 0);
        if (op_result == ZMTP_OK) {
            return continue_message_multipart(socket, topic, len, 1);
        } else {
            return op_result;
        }
    } else {
        return op_result;
    }
}

static sfsc_int8 zmtp_task_internal(zmtp_socket* socket) {
    sfsc_int8 op_result;
    if (socket->state < ZMTP_STATE_OPERATING) {
        op_result = zmtp_task_connect(socket);
        if (op_result != ZMTP_OK) {
            return op_result;
        }
    }
    if (socket->state == ZMTP_STATE_OPERATING &&
        !b_zmtp_socket_buffer_full(socket)) {
        op_result = read_next(socket);
        if (op_result != ZMTP_OK) {
            return op_result;
        } else if (socket->mechanism == MECHANISM_CURVE &&
                   b_zmtp_socket_buffer_full(socket)) {
#ifdef NO_CURVE
            return E_CURVE_DISABLED;
#else
            return curve_open_message(socket);
#endif
        }
    }
    return ZMTP_OK;
}

sfsc_int8 zmtp_task(zmtp_socket* socket) {
    sfsc_int8 result = zmtp_task_internal(socket);
    if (result == ZMTP_OK) {
        return ZMTP_OK;
    } else {
        socket->state = ZMTP_STATE_ERROR;
        return result;
    }
}

void zmtp_pop(zmtp_socket* socket) {
    if (socket->state == ZMTP_STATE_OPERATING) {
        reset(socket);
    }
}

sfsc_uint8* get_message(zmtp_socket* socket) {
    if (socket->is_message && b_zmtp_socket_buffer_full(socket)) {
        if (socket->mechanism == MECHANISM_CURVE) {
#ifdef NO_CURVE
            return NULL;
#else
            return socket->buffer_space + _CURVE_DECRYPTION_OFFSET +
                   _CURVE_MESSAGE_OFFSET;
#endif
        } else {
            return socket->buffer_space;
        }
    } else {
        return NULL;
    }
}

#endif /* ZMTP_C_ */
