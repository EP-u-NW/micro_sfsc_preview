/*
 * sfsc_adapter_sockets.h
 *
 *  Created on: 28.10.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_ADAPTER_SFSC_ADAPTER_DATA_H_
#define SRC_SFSC_ADAPTER_SFSC_ADAPTER_DATA_H_

#include "../zmtp/zmtp.h"
#include "sfsc_constants.h"
#include "user_ring.h"

#ifdef __cplusplus
extern "C" {
#endif
#define _sfsc_adapter_data_DEFAULT_INIT {zmtp_socket_DEFAULT_INIT,zmtp_socket_DEFAULT_INIT,zmtp_socket_DEFAULT_INIT,zmtp_socket_DEFAULT_INIT,user_ring_DEFAULT_INIT,0,0}
typedef struct __sfsc_adapter_data {
	zmtp_socket data_pub;
	zmtp_socket data_sub;
	zmtp_socket control_pub;
	zmtp_socket control_sub;
	user_ring user_buffer;
	sfsc_uint8 next_payload_type;
	sfsc_bool b_need_user_ring_advance;
} _sfsc_adapter_data;

sfsc_int8 process_io(_sfsc_adapter_data* adapter);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_ADAPTER_SOCKETS_H_ */
