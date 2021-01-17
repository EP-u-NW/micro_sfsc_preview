#ifndef TWEETNACL_STRIPPED_H
#define TWEETNACL_STRIPPED_H

#include "zmtp_config.h"

#ifndef NO_CURVE
#include "../platform/sfsc_types.h"
#include "../platform/sfsc_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define crypto_box_PUBLICKEYBYTES 32
#define crypto_box_SECRETKEYBYTES 32
#define crypto_box_MACBYTES 16
#define crypto_box_NONCEBYTES 24
#define crypto_box_ZEROBYTES 32
#define crypto_box_BOXZEROBYTES 16
#define crypto_box_CHIPER_LENGTH(message) (message+16)

int crypto_box_keypair(sfsc_uint8 *pk, sfsc_uint8 *sk);
int crypto_box(sfsc_uint8 *chiper, const sfsc_uint8 *message,
		sfsc_uint64 message_length, const sfsc_uint8 *nonce,
		const sfsc_uint8 *receiver_pk, const sfsc_uint8 *sender_sk);
int crypto_box_open(sfsc_uint8 *message, const sfsc_uint8 *chiper,
		sfsc_uint64 chiper_length, const sfsc_uint8 *nonce,
		const sfsc_uint8 *sender_pk, const sfsc_uint8 *receiver_sk);
		
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
#endif
