/*
 * zmtp_curve_constants.h
 *
 *  Created on: 30.10.2020
 *      Author: Eric
 */

#ifndef SRC_ZMTP_ZMTP_CURVE_CONSTANTS_H_
#define SRC_ZMTP_ZMTP_CURVE_CONSTANTS_H_

#include "tweetnacl_stripped.h"

#ifndef NO_CURVE
#ifdef __cplusplus
extern "C" {
#endif

#define CURVE_ZMQ_VERSION_MAJOR 1
#define CURVE_ZMQ_VERSION_MINOR 0


//To decrypt something we need 16 byte (crypto_box_BOXZEROBYTES) of zeros before we actually place the decrypted message.
//The thing we are decrypting is in most cases not the beginning of the message, e.g.
//we have a offset because of the command name and nonce. We can use this and set the bytes before the actuall box
//to zero. The HELLO, WELCOME, INITIATE and MESSAGE command have enough space for the zero bytes,the READY needs 2 more bytes.
#define _CURVE_DECRYPTION_OFFSET 2


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
#endif /* SRC_ZMTP_ZMTP_CURVE_CONSTANTS_H_ */
