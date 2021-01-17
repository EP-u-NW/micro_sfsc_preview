/*
 * zmtp_handshake_null.cpp
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#include "zmtp_null.h"

#include "zmtp_constants.h"
#include "zmtp_io.h"
#include "zmtp_metadata.h"
#include "zmtp_socket_types.h"
#include "zmtp_states.h"
#include "zmtp_utils.h"

sfsc_int8 handle_peer_metadata(zmtp_socket* socket, sfsc_uint8 offset) {
    transkript_metadata(socket, offset);
    socket->has_peer_metadata = 1;
    if (b_validate_socket_types(socket)) {
        reset(socket);
        socket->state = ZMTP_STATE_OPERATING;
        return ZMTP_OK;
    } else {
        return E_INCOMPATIBLE_SOCKET_TYPES;
    }
}

sfsc_int8 write_ready(zmtp_socket* socket) {
    sfsc_int64 length = 6 + meta_length(socket->metadata_buffer);
    sfsc_int8 op_result = write_length_header(socket, length, _FLAG_COMMAND);
    if (op_result == ZMTP_OK) {
        const sfsc_uint8 ready_signature[] = {5, 'R', 'E', 'A', 'D', 'Y'};
        op_result = zmtp_write(socket, ready_signature, 6);
        if (op_result == ZMTP_OK) {
            return write_metadata(socket, length - 6);
        } else {
            return op_result;
        }
    } else {
        return op_result;
    }
}

sfsc_int8 handshake_null(zmtp_socket* socket) {
    // write ready
    if (socket->state == ZMTP_STATE_HANDSHAKE) {
        sfsc_int8 op_result = write_ready(socket);
        if (op_result == ZMTP_OK) {
            socket->state = ZMTP_STATE_HANDSHAKE_NULL_EXPECT_READY;
        } else {
            return op_result;
        }
    }
    // try to read_to_index ready
    if (socket->state == ZMTP_STATE_HANDSHAKE_NULL_EXPECT_READY) {
        sfsc_uint8* buffer = effective_buffer(socket);

        sfsc_int8 read_result = read_next(socket);
        if (read_result == ZMTP_OK) {
            if (b_zmtp_socket_buffer_full(socket)) {
                if (IS_READY_COMMAND(socket, buffer)) {
                    return handle_peer_metadata(socket, 6);
                } else if (IS_ERROR_COMMAND(socket, buffer)) {
                    return E_ERROR_COMMAND;
                } else {
                    return E_UNEXPECTED;
                }
            } else {
                return ZMTP_OK;
            }
        } else {
            return read_result;
        }
    } else {
        // We should not be here
        return E_UNREACHABLE;
    }
}
