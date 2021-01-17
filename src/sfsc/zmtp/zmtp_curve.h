/*
 * zmtp_curve.h
 *
 *  Created on: 29.07.2020
 *      Author: Eric
 */

#ifndef ZMTP_CURVE_H_
#define ZMTP_CURVE_H_
#include "zmtp.h"

#ifndef NO_CURVE
#ifdef __cplusplus
extern "C" {
#endif


#define _CURVE_MESSAGE_OFFSET (crypto_box_ZEROBYTES + 1)

sfsc_int8 handshake_curve(zmtp_socket* socket);

sfsc_int8 curve_open_message(zmtp_socket* socket);
sfsc_int8 continue_message_curve(zmtp_socket* socket,
		const sfsc_uint8 message[],
		sfsc_size len, sfsc_int8 done);
sfsc_int8 begin_message_curve(zmtp_socket* socket,
sfsc_size len, sfsc_int8 as_last_message);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif

#endif /* ZMTP_CURVE_H_ */
