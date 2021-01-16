/*
 * zmtp_socket_types.h
 *
 *  Created on: 23.07.2020
 *      Author: Eric
 */

#ifndef ZMTP_SOCKET_TYPES_H_
#define ZMTP_SOCKET_TYPES_H_

#include "zmtp.h"
#include "../platform/sfsc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SOCKET_TYPE_META_KEY_LENGTH 11
extern const sfsc_uint8 socket_type_meta_key[SOCKET_TYPE_META_KEY_LENGTH];

sfsc_bool b_socket_type_name(sfsc_int8 type,const sfsc_uint8** name, sfsc_uint8* len);
sfsc_bool b_validate_socket_types(zmtp_socket* socket);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ZMTP_SOCKET_TYPES_H_ */
