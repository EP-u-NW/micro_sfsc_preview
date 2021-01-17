/*
 * zmtp_plain.h
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#ifndef SRC_ZMTP_ZMTP_PLAIN_H_
#define SRC_ZMTP_ZMTP_PLAIN_H_

#include "zmtp.h"

#ifdef __cplusplus
extern "C" {
#endif

sfsc_int8 handshake_plain(zmtp_socket* socket);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_ZMTP_ZMTP_PLAIN_H_ */
