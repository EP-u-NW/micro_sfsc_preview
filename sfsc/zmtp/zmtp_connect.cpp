/*
 * zmtp_connect.cpp
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#include "zmtp_connect.h"

#include "zmtp_io.h"
#include "zmtp_metadata.h"
#include "zmtp_null.h"
#include "zmtp_plain.h"
#include "zmtp_socket_types.h"
#include "zmtp_states.h"
#include "zmtp_utils.h"
#ifndef NO_CURVE
#include "zmtp_curve.h"
#endif

#define _GREETING_LENGTH 10 + 2 + 20 + 1 + 31

sfsc_int8 zmtp_connect(zmtp_socket *socket, sfsc_int16 socket_handle,
                  sfsc_uint8 mechanism, sfsc_uint8 *mechanism_extra,
                  sfsc_uint8 as_server, sfsc_uint8 socket_type) {
    const sfsc_uint8 *name;
    sfsc_uint8 name_len;
    b_socket_type_name(socket_type, &name, &name_len);
    put_meta(socket->metadata_buffer, ZMTP_METADATA_BUFFER_SIZE,
             socket_type_meta_key, SOCKET_TYPE_META_KEY_LENGTH, name, name_len);
    socket->socket_handle = socket_handle;
    socket->state = ZMTP_STATE_GREETING;
    socket->as_server = as_server;
    socket->mechanism = mechanism;
    socket->mechanism_extra = mechanism_extra;
    sfsc_uint8 signature[_GREETING_LENGTH] = {0};
    signature[0] = 0xFF;
    signature[9] = 0x7F;
    signature[10] = ZMTP_VERSION_MAJOR;
    signature[11] = ZMTP_VERSION_MINOR;
    switch (mechanism) {
        case MECHANISM_NULL:
            signature[12] = 'N';
            signature[13] = 'U';
            signature[14] = 'L';
            signature[15] = 'L';
            break;
        case MECHANISM_PLAIN:
            signature[12] = 'P';
            signature[13] = 'L';
            signature[14] = 'A';
            signature[15] = 'I';
            signature[16] = 'N';
            break;
        case MECHANISM_CURVE:
            signature[12] = 'C';
            signature[13] = 'U';
            signature[14] = 'R';
            signature[15] = 'V';
            signature[16] = 'E';
            break;
    }
    signature[32] = as_server;
    sfsc_int8 op_result = zmtp_write(socket, signature, _GREETING_LENGTH);
    if (op_result == ZMTP_OK) {
        socket->expected_size = _GREETING_LENGTH;
		return ZMTP_OK;
    } else {
        return op_result;
    }
}

static sfsc_bool b_zmtp_compatible_versions(sfsc_uint8 peer_major,
                                            sfsc_uint8 peer_minor) {
    return peer_major == ZMTP_VERSION_MAJOR;
}

static sfsc_int8 validate_greeting(zmtp_socket *socket) {
    sfsc_uint8 *buffer = effective_buffer(socket);
    if (buffer[0] == 0xFF && buffer[9] == 0x7F &&
        b_zmtp_compatible_versions(buffer[10], buffer[11])) {
        sfsc_uint8 peer_mechanism = zmtp_mechanism_by_name(buffer + 12);
        if (socket->mechanism != peer_mechanism) {
            return E_DIFFERENT_MECHANISMS;
        } else if ((socket->mechanism == MECHANISM_PLAIN ||
                    socket->mechanism == MECHANISM_CURVE) &&
                   socket->as_server == buffer[32]) {
            return E_BAD_ROLES;
        } else {
            return ZMTP_OK;
        }
    } else {
        return E_BAD_SIGNATURE;
    }
}

sfsc_int8 zmtp_task_connect(zmtp_socket *socket) {
    if (socket->state == ZMTP_STATE_GREETING) {
        sfsc_int8 read_result = read_expected(socket);
        if (read_result == ZMTP_OK) {
            if (b_zmtp_socket_buffer_full(socket)) {
                sfsc_uint8 greeting = validate_greeting(socket);
                if (greeting == ZMTP_OK) {
                    socket->state = ZMTP_STATE_HANDSHAKE;
                    reset(socket);
                } else {
                    return greeting;
                }
            }
        } else {
            return read_result;
        }
    }
    if (socket->state != ZMTP_STATE_GREETING &&
        socket->state < ZMTP_STATE_OPERATING) {
        sfsc_uint8 handshake_result = E_UNEXPECTED;
        switch (socket->mechanism) {
            case MECHANISM_NULL:
                handshake_result = handshake_null(socket);
                break;
            case MECHANISM_PLAIN:
                handshake_result = handshake_plain(socket);
                break;
            case MECHANISM_CURVE:
#ifdef NO_CURVE
                handshake_result = E_CURVE_DISABLED;
#else
                handshake_result = handshake_curve(socket);
#endif
                break;
        }
        if (handshake_result != ZMTP_OK) {
            return handshake_result;
        }
    }
    return ZMTP_OK;
}
