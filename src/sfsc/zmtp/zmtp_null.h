/*
 * zmtp_null.h
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#ifndef SRC_ZMTP_ZMTP_NULL_H_
#define SRC_ZMTP_ZMTP_NULL_H_

#include "zmtp.h"

#ifdef __cplusplus
extern "C" {
#endif

sfsc_int8 handshake_null(zmtp_socket* socket);
sfsc_int8 write_ready(zmtp_socket* socket);
sfsc_int8 handle_peer_metadata(zmtp_socket* socket, sfsc_uint8 offset);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_ZMTP_ZMTP_NULL_H_ */
