/*
 * zmtp_handshake_plain.cpp
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#include "zmtp_plain.h"

#include "zmtp_constants.h"
#include "zmtp_io.h"
#include "zmtp_metadata.h"
#include "zmtp_null.h"
#include "zmtp_states.h"
#include "zmtp_utils.h"

void get_plain_peer_credentials(zmtp_socket* socket, const sfsc_uint8** name,
                                sfsc_uint8* name_len,
                                const sfsc_uint8** password,
                                sfsc_uint8* password_len) {
    sfsc_uint8* buffer = socket->buffer_space + 6;
    *password = buffer + 1 + buffer[0] + 1;
    *password_len = buffer[1 + buffer[0]];
    *name = buffer + 1;
    *name_len = buffer[0];
}

static sfsc_int8 handshake_plain_server(zmtp_socket* socket) {
    sfsc_uint8* buffer = effective_buffer(socket);
    if (socket->state == ZMTP_STATE_HANDSHAKE) {
        socket->state = ZMTP_STATE_HANDSHAKE_PLAIN_SERVER_EXPECT_HELLO;
    }
    if (socket->state == ZMTP_STATE_HANDSHAKE_PLAIN_SERVER_EXPECT_HELLO) {
        sfsc_int8 read_result = read_next(socket);
        if (read_result == ZMTP_OK) {
            if (b_zmtp_socket_buffer_full(socket)) {
                if (IS_HELLO_COMMAND(socket, buffer)) {
                    socket->state =
                        ZMTP_STATE_HANDSHAKE_PLAIN_SERVER_VALIDATE_CREDENTIALS;
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
    }
    if (socket->state ==
        ZMTP_STATE_HANDSHAKE_PLAIN_SERVER_VALIDATE_CREDENTIALS) {
        return ZMTP_OK;
    }
    if (socket->state == ZMTP_STATE_HANDSHAKE_PLAIN_SERVER_EXPECT_INITIATE) {
        sfsc_int8 read_result = read_next(socket);
        if (read_result == ZMTP_OK) {
            if (b_zmtp_socket_buffer_full(socket)) {
                if (IS_INITIATE_COMMAND(socket, buffer)) {
                    sfsc_int8 result = handle_peer_metadata(socket, 9);
                    if (result == ZMTP_OK) {
                        return write_ready(socket);
                    } else {
                        // revert state
                        socket->state =
                            ZMTP_STATE_HANDSHAKE_PLAIN_SERVER_EXPECT_INITIATE;
                        return result;
                    }
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
    }
    // We should not be here
    return E_UNREACHABLE;
}

static sfsc_uint16 plain_buffer_length(const sfsc_uint8* buffer) {
    return buffer[0] + buffer[1 + buffer[0]];
}

static sfsc_int8 handshake_plain_client(zmtp_socket* socket) {
    sfsc_int8 write_result;
    sfsc_uint8* buffer = effective_buffer(socket);
    if (socket->state == ZMTP_STATE_HANDSHAKE) {
        sfsc_int64 length =
            6 + plain_buffer_length((sfsc_uint8*)socket->mechanism_extra);
        write_result = write_length_header(socket, length, _FLAG_COMMAND);
        if (write_result == ZMTP_OK) {
            const sfsc_uint8 hello_signature[] = {5, 'H', 'E', 'L', 'L', 'O'};
            if (write_result == ZMTP_OK) {
                write_result = zmtp_write(socket, hello_signature, 6);
                if (write_result == ZMTP_OK) {
                    write_result =
                        zmtp_write(socket, (sfsc_uint8*)socket->mechanism_extra,
                                   length - 6);
                    if (write_result == ZMTP_OK) {
                        socket->state =
                            ZMTP_STATE_HANDSHAKE_PLAIN_CLIENT_EXPECT_WELCOME;
                        reset(socket);
                    } else {
                        return write_result;
                    }
                } else {
                    return write_result;
                }
            } else {
                return write_result;
            }
        } else {
            return write_result;
        }
    }
    if (socket->state == ZMTP_STATE_HANDSHAKE_PLAIN_CLIENT_EXPECT_WELCOME) {
        sfsc_int8 read_result = read_next(socket);
        if (read_result == ZMTP_OK) {
            if (b_zmtp_socket_buffer_full(socket)) {
                if (IS_WELCOME_COMMAND(socket, buffer)) {
                    sfsc_int64 length =
                        9 + meta_length(socket->metadata_buffer);
                    write_result =
                        write_length_header(socket, length, _FLAG_COMMAND);
                    if (write_result == ZMTP_OK) {
                        const sfsc_uint8 initiate_signature[] = {
                            8, 'I', 'N', 'I', 'T', 'I', 'A', 'T', 'E'};
                        write_result =
                            zmtp_write(socket, initiate_signature, 9);
                        if (write_result == ZMTP_OK) {
                            write_result = write_metadata(socket, length - 9);
                            if (write_result == ZMTP_OK) {
                                socket->state =
                                    ZMTP_STATE_HANDSHAKE_PLAIN_CLIENT_EXPECT_READY;
                                reset(socket);
                            } else {
                                return write_result;
                            }
                        } else {
                            return write_result;
                        }
                    } else {
                        return write_result;
                    }
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
    }
    if (socket->state == ZMTP_STATE_HANDSHAKE_PLAIN_CLIENT_EXPECT_READY) {
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
    }
    // We should not be here
    return E_UNREACHABLE;
}

sfsc_int8 handshake_plain(zmtp_socket* socket) {
    if (socket->as_server) {
        return handshake_plain_server(socket);
    } else {
        return handshake_plain_client(socket);
    }
}

sfsc_int8 plain_credentials_ok(zmtp_socket* socket) {
    if (socket->mechanism == MECHANISM_PLAIN && socket->as_server &&
        socket->state ==
            ZMTP_STATE_HANDSHAKE_PLAIN_SERVER_VALIDATE_CREDENTIALS) {
        write_length_header(socket, 8, _FLAG_COMMAND);
        const sfsc_uint8 welcome_signature[] = {7,   'W', 'E', 'L',
                                                'C', 'O', 'M', 'E'};
        zmtp_write(socket, welcome_signature, 8);
        socket->state = ZMTP_STATE_HANDSHAKE_PLAIN_SERVER_EXPECT_INITIATE;
        reset(socket);
        return ZMTP_OK;
    } else {
        return E_UNEXPECTED;
    }
}
