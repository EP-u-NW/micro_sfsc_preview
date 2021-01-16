/*
 * zmtp_metadata.c
 *
 *  Created on: 15.07.2020
 *      Author: Eric
 */

#include "zmtp_metadata.h"

#include "zmtp.h"
#include "zmtp_io.h"
#include "zmtp_utils.h"

static sfsc_uint8 elements_equal(const sfsc_uint8 a[], const sfsc_uint8 b[],
                                 sfsc_size len) {
    for (sfsc_size i = 0; i < len; i++) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}
sfsc_uint8* get_meta(sfsc_uint8 metadata[], const sfsc_uint8 key[],
                     sfsc_uint8 key_length, sfsc_uint16* value_length_out) {
    sfsc_uint16 index = 0;
    sfsc_uint8 match = 0;
    while (metadata[index] != 0) {
        match = metadata[index] == key_length &&
                elements_equal(key, metadata + index + 1, key_length);
        index = index + metadata[index] + 1;
        sfsc_uint16 val_len =
            (sfsc_uint16)(*((sfsc_uint32*)(metadata + index)));
        if (match) {
            *value_length_out = val_len;
            return metadata + index + 4;
        } else {
            index = index + 4 + val_len;
        }
    }
    return 0;
}

sfsc_uint8 put_meta(sfsc_uint8 metadata[], sfsc_size metadata_buffer_length,
                    const sfsc_uint8 key[], sfsc_uint8 key_length,
                    const sfsc_uint8 value[], sfsc_uint32 value_length) {
    sfsc_size offset = meta_length(metadata);
    if (offset + 1 + key_length + 4 + value_length < metadata_buffer_length) {
        metadata[offset] = key_length;
        zmtp_copy(metadata + offset + 1, key, key_length);
        zmtp_copy(metadata + offset + 1 + key_length,
                  (sfsc_uint8*)&value_length, 4);
        zmtp_copy(metadata + offset + 1 + key_length + 4, value, value_length);
        return 1;
    } else {
        return 0;
    }
}

sfsc_size meta_length(sfsc_uint8 metadata[]) {
    sfsc_size index = 0;
    while (metadata[index] != 0) {
        index = index + metadata[index] + 1;
        sfsc_size val_len = (sfsc_size)(*((sfsc_uint32*)(metadata + index)));
        index = index + 4 + val_len;
    }
    return index;
}

void write_metadata_to_buffer(zmtp_socket* socket, sfsc_size len,
                              sfsc_uint8* target) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    _COPY(target, socket->metadata_buffer, len);
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    sfsc_size index = 0;
    while (index < len) {
        zmtp_copy(target + index, socket->metadata_buffer + index,
                  socket->metadata_buffer[index] + 1);
        index = index + socket->metadata_buffer[index] + 1;
        (target + index)[3] = socket->metadata_buffer[index];
        (target + index)[2] = socket->metadata_buffer[index + 1];
        (target + index)[1] = socket->metadata_buffer[index + 2];
        (target + index)[0] = socket->metadata_buffer[index + 3];
        sfsc_size value_length =
            (sfsc_size)(*((sfsc_uint32*)(socket->metadata_buffer + index)));
        index = index + 4;
        zmtp_copy(target + index, socket->metadata_buffer + index,
                  value_length);
        index = index + value_length;
    }
#endif
}

sfsc_int8 write_metadata(zmtp_socket* socket, sfsc_size len) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return zmtp_write(socket->socket_handle, socket->metadata_buffer, len);
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    sfsc_size index = 0;
    sfsc_uint8 value_length_buf[4];
    sfsc_int8 op_result;
    while (index < len) {
        op_result = zmtp_write(socket, socket->metadata_buffer + index,
                               socket->metadata_buffer[index] + 1);
        if (op_result == ZMTP_OK) {
            index = index + socket->metadata_buffer[index] + 1;
            value_length_buf[3] = socket->metadata_buffer[index];
            value_length_buf[2] = socket->metadata_buffer[index + 1];
            value_length_buf[1] = socket->metadata_buffer[index + 2];
            value_length_buf[0] = socket->metadata_buffer[index + 3];
            op_result = zmtp_write(socket, value_length_buf, 4);
            if (op_result == ZMTP_OK) {
                sfsc_size value_length = (sfsc_size)(
                    *((sfsc_uint32*)(socket->metadata_buffer + index)));
                index = index + 4;
                op_result = zmtp_write(socket, socket->metadata_buffer + index,
                                       value_length);
                if (op_result == ZMTP_OK) {
                    index = index + value_length;
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
    return ZMTP_OK;
#endif
}

void transkript_metadata(zmtp_socket* socket, sfsc_uint16 offset) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    sfsc_uint8* buffer = _BUFFER(socket) + offset;
    _COPY((socket->peer_metadata_buffer), buffer,
          socket->buffer_index - offset);
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    sfsc_uint8* buffer = effective_buffer(socket);
    sfsc_uint16 index = 0;
    while (index + offset < socket->buffer_index) {
        zmtp_copy(socket->peer_metadata_buffer + index, buffer + offset + index,
                  buffer[offset + index] + 1);
        index = buffer[offset + index] + 1;
        socket->peer_metadata_buffer[index] = buffer[index + offset + 3];
        socket->peer_metadata_buffer[index + 1] = buffer[index + offset + 2];
        socket->peer_metadata_buffer[index + 2] = buffer[index + offset + 1];
        socket->peer_metadata_buffer[index + 3] = buffer[index + offset];
        sfsc_uint16 value_length = (sfsc_uint16)(
            *((sfsc_uint32*)(socket->peer_metadata_buffer + index)));
        index = index + 4;
        zmtp_copy((socket->peer_metadata_buffer + index),
                  (buffer + offset + index), value_length);
        index = index + value_length;
    }
#endif
}
