/*
 * zmtp_io.cpp
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#include "zmtp_io.h"

#include "../platform/sfsc_sockets.h"
#include "zmtp_states.h"
#include "zmtp_utils.h"

#define _SHORT_LENGTH_FIELD 1
#define _LONG_LENGTH_FIELD 8
#define _FIND_EXPECTED_SIZE -(_LONG_LENGTH_FIELD + 1)

#define _LONG_COMMAND 0x06
#define _SHORT_COMMAND 0x04
#define _LONG_MESSAGE 0x03
#define _SHORT_MESSAGE 0x01
#define _LONG_LAST_MESSAGE 0x02
#define _SHORT_LAST_MESSAGE 0x00

#if defined(__BYTE_ORDER__)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _LSB_OFFSET 0
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _LSB_OFFSET 8
#else
#error Byte order not supported!
#endif
#endif

const sfsc_uint8 long_message = _LONG_MESSAGE, short_message = _SHORT_MESSAGE,
                 long_last_message = _LONG_LAST_MESSAGE,
                 short_last_message = _SHORT_LAST_MESSAGE,
                 long_command = _LONG_COMMAND, short_command = _SHORT_COMMAND;

static sfsc_int8 read_to_index(zmtp_socket* socket, sfsc_size index) {
    sfsc_size in_buffer_size;
    if ((socket)->mechanism == MECHANISM_CURVE) {
#ifdef NO_CURVE
        return E_CURVE_DISABLED;
#else
        in_buffer_size = (ZMTP_IN_BUFFER_SIZE - _CURVE_DECRYPTION_OFFSET);
#endif
    } else {
        in_buffer_size = ZMTP_IN_BUFFER_SIZE;
    }
    if (index < in_buffer_size) {
        if (index == socket->buffer_index) {
            return ZMTP_OK;  // We are already there
        } else if (socket->buffer_index < index) {
            sfsc_socket_size r =
                socket_read(socket->socket_handle,
                            effective_buffer(socket) + socket->buffer_index,
                            index - socket->buffer_index);
            if (r < 0) {
                return E_EOF;
            } else {
                socket->buffer_index += r;
                return ZMTP_OK;
            }
        } else {
            return E_INDEX_ERROR;  // We can not read a negative amount of bytes
        }
    } else {
        return E_BUFFER_INSUFFICIENT;  // We could not poses so much bytes
    }
}

sfsc_uint8 read_expected(zmtp_socket* socket) {
    return read_to_index(socket, socket->expected_size);
}

static sfsc_int8 read_size(zmtp_socket* socket) {
    sfsc_uint8* buffer = effective_buffer(socket);
    if (socket->expected_size == _FIND_EXPECTED_SIZE) {  // We know nothing
        // read_to_index one byte
        sfsc_int8 read_result = read_to_index(socket, 1);
        if (read_result == ZMTP_OK) {
            if (socket->buffer_index == 1) {
                socket->last_message = 0;
                socket->is_message = 0;
                //We want fallthroughs in this switch
                switch (buffer[0]) {
                    case _SHORT_LAST_MESSAGE:
                        socket->last_message =
                            1;  // @suppress("No break at end of case")
                    case _SHORT_MESSAGE:
                        socket->is_message =
                            1;  // @suppress("No break at end of case")
                    case _SHORT_COMMAND:
                        socket->expected_size = -_SHORT_LENGTH_FIELD;
                        break;
                    case _LONG_LAST_MESSAGE:
                        socket->last_message =
                            1;  // @suppress("No break at end of case")
                    case _LONG_MESSAGE:
                        socket->is_message =
                            1;  // @suppress("No break at end of case")
                    case _LONG_COMMAND:
                        socket->expected_size = -_LONG_LENGTH_FIELD;
                        break;
                    default:
                        return E_UNREACHABLE;
                }
                socket->buffer_index = 0;
                return ZMTP_OK;
            } else {
                // The read was successful, but to few bytes where ready, so
                // return
                return ZMTP_OK;
            }
        } else {
            // read_to_index failed
            return read_result;
        }
    }
    if (-socket->expected_size == _SHORT_LENGTH_FIELD ||
        -socket->expected_size ==
            _LONG_LENGTH_FIELD) {  // we know the length field size now
        sfsc_int8 read_result = read_to_index(socket, -socket->expected_size);
        if (read_result == ZMTP_OK) {
            if (socket->buffer_index == (sfsc_size)(-socket->expected_size)) {
                if (-socket->expected_size == _SHORT_LENGTH_FIELD) {
                    socket->expected_size = buffer[0];
                } else {
                    zmtp_order_bytes(buffer, 8);
                    socket->expected_size = *((sfsc_int64*)buffer);
                }
                socket->buffer_index = 0;
            }
            return ZMTP_OK;
        } else {
            // read_to_index failed
            return read_result;
        }
    } else {
        return E_UNREACHABLE;
    }
}

void reset(zmtp_socket* socket) {
    socket->buffer_index = 0;
    socket->expected_size = _FIND_EXPECTED_SIZE;
}

sfsc_int8 read_next(zmtp_socket* socket) {
    if (socket->expected_size < 0) {
        sfsc_int8 read_result = read_size(socket);
        if (read_result != ZMTP_OK) {
            return read_result;
        }
    }
    if (socket->expected_size >= 0) {
        sfsc_int8 read_result = read_to_index(socket, socket->expected_size);
        if (read_result != ZMTP_OK) {
            return read_result;
        }
    }
    return ZMTP_OK;
}

sfsc_int8 write_length_header(zmtp_socket* socket, sfsc_int64 length,
                              sfsc_uint8 type) {
    sfsc_uint8 is_short = length <= 255;
    const sfsc_uint8* header;
    switch (type) {
        case _FLAG_COMMAND:
            header = is_short ? &short_command : &long_command;
            break;
        case _FLAG_MESSAGE:
            header = is_short ? &short_message : &long_message;
            break;
        default:  // case _FLAG_LAST_MESSAGE:
            header = is_short ? &short_last_message : &long_last_message;
            break;
    }
    sfsc_int8 op_result = socket_write(socket->socket_handle, header, 1);
    if (op_result == SOCKET_OK) {
        if (is_short) {
            return socket_write(socket->socket_handle,
                                ((sfsc_uint8*)&length) + _LSB_OFFSET, 1);
        } else {
            sfsc_uint8* len_bytes = (sfsc_uint8*)(&length);
            zmtp_order_bytes(len_bytes, 8);
            return socket_write(socket->socket_handle, len_bytes, 8);
        }
    } else {
        return op_result;
    }
}

sfsc_int8 zmtp_write(zmtp_socket* socket, const sfsc_uint8* msg,
                     sfsc_size msg_len) {
    return socket_write(socket->socket_handle, msg, msg_len);
}

sfsc_int8 zmtp_flush(zmtp_socket* socket) {
    return socket_flush(socket->socket_handle);
}
