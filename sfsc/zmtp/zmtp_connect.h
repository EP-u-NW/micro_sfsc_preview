/*
 * zmtp_connect.h
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#ifndef SRC_ZMTP_ZMTP_CONNECT_H_
#define SRC_ZMTP_ZMTP_CONNECT_H_

#include "zmtp.h"

#ifdef __cplusplus
extern "C" {
#endif

sfsc_int8 zmtp_task_connect(zmtp_socket* socket);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_ZMTP_ZMTP_CONNECT_H_ */
