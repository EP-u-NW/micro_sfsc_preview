/*
 * zmtp_utils.cpp
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#include "zmtp_utils.h"

#include "../platform/sfsc_strings.h"

sfsc_int8 zmtp_mechanism_by_name(const sfsc_uint8* name) {
    return (
        name[0] == 'P' && name[1] == 'L' && name[2] == 'A' && name[3] == 'I' &&
                name[4] == 'N' && name[5] == 0
            ? MECHANISM_PLAIN
            : (name[0] == 'N' && name[1] == 'U' && name[2] == 'L' &&
                       name[3] == 'L' && name[4] == 0
                   ? MECHANISM_NULL
                   : (name[0] == 'C' && name[1] == 'U' && name[2] == 'R' &&
                              name[3] == 'V' && name[4] == 'E' && name[5] == 0
                          ? MECHANISM_CURVE
                          : MECHANISM_ERROR)));
}

void zmtp_order_bytes(sfsc_uint8 buf[], sfsc_size len) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    sfsc_uint8 tmp = buf[0];
    for (sfsc_size i = 0; i < len / 2; i++) {
        buf[i] = buf[len - i - 1];
        buf[len - i - 1] = tmp;
        tmp = buf[i + 1];
    }
#endif
}

void zmtp_zero(sfsc_uint8* buffer, sfsc_size length) {
    memset(buffer, 0, length);
}

sfsc_uint8* effective_buffer(zmtp_socket* socket) {
    if ((socket)->mechanism == MECHANISM_CURVE) {
#ifdef NO_CURVE
        return NULL;
#else
        return (socket)->buffer_space + _CURVE_DECRYPTION_OFFSET;
#endif
    } else {
        return (socket)->buffer_space;
    }
}

sfsc_bool b_zmtp_socket_buffer_full(zmtp_socket* socket) {
    return socket->buffer_index == socket->expected_size;
}

void zmtp_copy(sfsc_uint8* dst, const sfsc_uint8* src, sfsc_size len) {
    memcpy(dst, src, len);
}
