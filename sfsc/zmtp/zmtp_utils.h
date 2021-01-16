/*
 * zmtp_utils.h
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#ifndef SRC_ZMTP_ZMTP_UTILS_H_
#define SRC_ZMTP_ZMTP_UTILS_H_

#include "zmtp.h"

#ifdef __cplusplus
extern "C" {
#endif

sfsc_int8 zmtp_mechanism_by_name(const sfsc_uint8* name);

void zmtp_order_bytes(sfsc_uint8 buf[], sfsc_size len);

void zmtp_zero(sfsc_uint8* buffer, sfsc_size len);
void zmtp_copy(sfsc_uint8* dst,const sfsc_uint8* src, sfsc_size len);

sfsc_uint8* effective_buffer(zmtp_socket* socket);
sfsc_bool b_zmtp_socket_buffer_full(zmtp_socket* socket);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_ZMTP_ZMTP_UTILS_H_ */
