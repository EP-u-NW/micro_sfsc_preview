/*
 * zmtp_public.h
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#ifndef SRC_ZMTP_ZMTP_H_
#define SRC_ZMTP_ZMTP_H_

#include "../platform/sfsc_platform.h"
#include "zmtp_config.h"
#include "zmtp_curve_constants.h"
#include "zmtp_states.h"

#ifdef __cplusplus
extern "C" {
#endif


#define ZMTP_SOCKET_TYPE_REQ 0
#define ZMTP_SOCKET_TYPE_REP 1
#define ZMTP_SOCKET_TYPE_PUB 2
#define ZMTP_SOCKET_TYPE_SUB 3
#define ZMTP_SOCKET_TYPE_XPUB 4
#define ZMTP_SOCKET_TYPE_XSUB 5
#define ZMTP_SOCKET_TYPE_PUSH 6
#define ZMTP_SOCKET_TYPE_PULL 7
#define ZMTP_SOCKET_TYPE_PAIR 8
#define ZMTP_SOCKET_TYPE_ROUTER 9
#define ZMTP_SOCKET_TYPE_DEALER 10
#define ZMTP_SOCKET_TYPE_ERROR -1

#define MECHANISM_NULL 0
#define MECHANISM_PLAIN 1
#define MECHANISM_CURVE 2
#define MECHANISM_ERROR -1

#define HAS_MESSAGE(socket) (get_message(socket)!=NULL)

#ifndef NO_CURVE
#define CURVE_ENCRYPTION_BUFFER_SIZE (ZMTP_IN_BUFFER_SIZE-_CURVE_DECRYPTION_OFFSET+crypto_box_ZEROBYTES)//TODO think about this again
#define CURVE_PUBLIC_KEY_OFFSET 0
#define CURVE_SECRET_KEY_OFFSET crypto_box_PUBLICKEYBYTES

#define zmtp_curve_state_DEFAULT_INIT {NULL,{0},NULL,{0},0,0,{0},0}
typedef struct _zmtp_curve_state{
    const sfsc_uint8* persistent_keys;
	sfsc_uint8  peer_public_transient_key[crypto_box_PUBLICKEYBYTES];
	const sfsc_uint8* server_public_key; //Only Client
	sfsc_uint8 secret_transient_key[crypto_box_SECRETKEYBYTES];
	sfsc_uint64 nonce_counter;
	sfsc_uint64 nonce_guard;
	sfsc_uint8 encryption_buffer[CURVE_ENCRYPTION_BUFFER_SIZE];
	sfsc_size encryption_buffer_multipart_index;
} zmtp_curve_state;
#endif

#define zmtp_socket_DEFAULT_INIT {0,{0},0,ZMTP_STATE_NONE,0,0,0,{0},0,{0},0,0,0,0,NULL}
typedef struct _zmtp_socket {
	sfsc_int16 socket_handle;
	sfsc_uint8 buffer_space[ZMTP_IN_BUFFER_SIZE]; //First 16 bytes need to be 0 for curve
	sfsc_size buffer_index;
	sfsc_uint8 state;
	sfsc_uint8 mechanism;
	sfsc_bool as_server;
	// The protocol specifies a range from 0 to 2^63-1, but this
	// can actually not be greater than ZMTP_IN_BUFFER_SIZE
	//expected_size will be set to negative values internally, so it must be signed!
	sfsc_int64 expected_size;
	sfsc_uint8 metadata_buffer[ZMTP_METADATA_BUFFER_SIZE];
	sfsc_bool has_peer_metadata;
	sfsc_uint8 peer_metadata_buffer[ZMTP_METADATA_BUFFER_SIZE];
	sfsc_bool last_message;
	sfsc_bool is_message;
	sfsc_bool writing_last;
	sfsc_uint8 lock_mem;
	void* mechanism_extra;
} zmtp_socket;

/** Basic **/
sfsc_int8 zmtp_connect(zmtp_socket* socket,sfsc_int16 socket_handle, sfsc_uint8 mechanism,
		sfsc_uint8* mechanism_extra, sfsc_bool as_server,
		sfsc_uint8 socket_type);
sfsc_int8 zmtp_task(zmtp_socket* socket);
void zmtp_error_msg(zmtp_socket* socket, const sfsc_uint8** error_msg,sfsc_uint8* error_msg_len);
sfsc_int8 zmtp_flush(zmtp_socket* socket);

/** Writing **/
sfsc_int8 write_message(zmtp_socket* socket, const sfsc_uint8 message[],
sfsc_size len, sfsc_bool last);
sfsc_int8 begin_message_multipart(zmtp_socket* socket,sfsc_size len, sfsc_bool as_last_message);
sfsc_int8 continue_message_multipart(zmtp_socket* socket,const sfsc_uint8 message[],sfsc_size len, sfsc_bool end);
sfsc_int8 write_subscription_message(zmtp_socket* socket, const sfsc_uint8 topic[],
sfsc_size len, sfsc_bool subscribe);

/** Reading **/
sfsc_uint8* get_message(zmtp_socket* socket);
void zmtp_pop(zmtp_socket* socket);

/** Plain **/
sfsc_int8 plain_credentials_ok(zmtp_socket* socket);
//Don't use the sfsc_size type for name and password lengths, since their size is defined in the zmtp-plain protocol
void get_plain_peer_credentials(zmtp_socket* socket,
		const sfsc_uint8** name, sfsc_uint8* name_len,const sfsc_uint8** password,
		sfsc_uint8* password_len);


#ifndef NO_CURVE
/** Curve **/
sfsc_int8 curve_client_persistent_key_ok(zmtp_socket* socket);
void create_curve(zmtp_curve_state* state,const sfsc_uint8* persistent_keys,const sfsc_uint8* server_public_key);
sfsc_uint8* curve_client_persistent_key_during_handshake(zmtp_socket* socket);
#endif

/** Metadata handeling **/
//Don't use the sfsc_size type for metadata key and value, since their size is defined in the zmtp protocol
sfsc_uint8* get_meta(sfsc_uint8 metadata[], const sfsc_uint8 key[],
sfsc_uint8 key_length, sfsc_uint32* value_length_out);
sfsc_uint8 put_meta(sfsc_uint8 metadata[], sfsc_size metadata_buffer_length,
		const sfsc_uint8 key[],
		sfsc_uint8 key_length, const sfsc_uint8 value[],
		sfsc_uint32 value_length);
sfsc_size meta_length(sfsc_uint8 metadata[]);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* SRC_ZMTP_ZMTP_H_ */
