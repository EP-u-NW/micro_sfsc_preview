/*
 * zmtp_curve.c
 *
 *  Created on: 17.08.2020
 *      Author: Eric
 */

#include "zmtp_curve.h"

#ifndef NO_CURVE
#include "zmtp_constants.h"
#include "zmtp_states.h"
#include "zmtp_io.h"
#include "zmtp_utils.h"
#include "zmtp_socket_types.h"
#include "zmtp_metadata.h"

#define IS_READY_COMMAND_CURVE(socket,buffer) ((socket)->buffer_index >= 30 && IS_READY_COMMAND((socket),(buffer)))
#define IS_INITIATE_COMMAND_CURVE(socket,buffer) ((socket)->buffer_index >= 257 && IS_INITIATE_COMMAND((socket),(buffer)))
#define IS_HELLO_COMMAND_CURVE(socket,buffer) ((socket)->buffer_index == 200 && IS_HELLO_COMMAND((socket),(buffer)))
#define IS_WELCOME_COMMAND_CURVE(socket,buffer)  ((socket)->buffer_index == 168 && IS_WELCOME_COMMAND((socket),(buffer)))
#define IS_MESSAGE_COMMAND_CURVE(socket,buffer) ((socket)->buffer_index >= 33 && (socket)->is_message == 1&& (socket)->last_message==1 && \
		(buffer)[0] == 7&& (buffer)[1] == 'M' && (buffer)[2] == 'E' && (buffer)[3] == 'S' && (buffer)[4] == 'S' && (buffer)[5] == 'A'  && (buffer)[6] == 'G' && (buffer)[7] == 'E')

sfsc_uint8 MESSAGE_NONCE[] = { 'C', 'u', 'r', 'v', 'e', 'Z', 'M', 'Q', 'M', 'E',
		'S', 'S', 'A', 'G', 'E', '?', 0, 0, 0, 0, 0, 0, 0, 0 };
sfsc_uint8 READY_NONCE[] = { 'C', 'u', 'r', 'v', 'e', 'Z', 'M', 'Q', 'R', 'E',
		'A', 'D', 'Y', '-', '-', '-', 0, 0, 0, 0, 0, 0, 0, 0 };
sfsc_uint8 INITIATE_NONCE[] = { 'C', 'u', 'r', 'v', 'e', 'Z', 'M', 'Q', 'I',
		'N', 'I', 'T', 'I', 'A', 'T', 'E', 0, 0, 0, 0, 0, 0, 0, 0 };
sfsc_uint8 HELLO_NONCE[] = { 'C', 'u', 'r', 'v', 'e', 'Z', 'M', 'Q', 'H', 'E',
		'L', 'L', 'O', '-', '-', '-', 0, 0, 0, 0, 0, 0, 0, 0 };
sfsc_uint8 WELCOME_NONCE[] = { 'W', 'E', 'L', 'C', 'O', 'M', 'E', '-', 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
sfsc_uint8 COOKIE_NONCE[] = { 'C', 'O', 'O', 'K', 'I', 'E', '-', '-', 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
sfsc_uint8 VOUCH_NONCE[] = { 'V', 'O', 'U', 'C', 'H', '-', '-', '-', 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static sfsc_bool b_curve_zmq_versions_compatible(sfsc_uint8 peer_major,
sfsc_uint8 peer_minor) {
	return peer_major == CURVE_ZMQ_VERSION_MAJOR
			&& peer_minor == CURVE_ZMQ_VERSION_MINOR;
}

static void delete_transient_keys(zmtp_curve_state* state) {
	for (sfsc_uint8 i = 0; i < crypto_box_SECRETKEYBYTES; i++) {
		state->peer_public_transient_key[i] = 0; //Discard keys according to zmtp spec
		state->secret_transient_key[i] = 0; // Do this just to be sure...
	}
}

void create_curve(zmtp_curve_state* state, const sfsc_uint8* persistent_keys,
		const sfsc_uint8* server_public_key) {
	state->persistent_keys = persistent_keys;
	state->server_public_key = server_public_key;
}

static void next_nonce(zmtp_curve_state* state, sfsc_uint8* target) {
	//Let the first nonce be 1, (to match jeromq)
	state->nonce_counter++;
	sfsc_uint8* counter_bytes = (sfsc_uint8*) (&(state->nonce_counter));
	zmtp_copy((target + 16), counter_bytes, 8);
	zmtp_order_bytes((target + 16), 8);
}

sfsc_bool b_fill_and_validate_nonce(zmtp_curve_state* state,
sfsc_uint8* nonce_bytes, sfsc_uint8* target, sfsc_uint8 first) {
	zmtp_copy(target + 16, nonce_bytes, 8);
	zmtp_order_bytes(nonce_bytes, 8);
	sfsc_uint64 counter = *((sfsc_uint64*) nonce_bytes);
	if (first || state->nonce_guard < counter) {
		state->nonce_guard = counter;
		return 1;
	} else {
		return 0;
	}
}

static void note_cookie_time(zmtp_curve_state* state) {
	sfsc_uint64 ttl = time_ms() + 60 * 1000;
	zmtp_copy(state->encryption_buffer, (sfsc_uint8*) (&ttl), 8);
}

static sfsc_int8 b_cookie_timed_out(zmtp_curve_state* state) {
	return *((sfsc_uint64*) (state->encryption_buffer)) < time_ms();
}

static sfsc_int8 b_validate_vouch(zmtp_curve_state* state, sfsc_uint8* vouch) {
	for (sfsc_uint8 i = 0; i < crypto_box_SECRETKEYBYTES; i++) {
		if (state->peer_public_transient_key[i] != vouch[i]
				|| (state->persistent_keys + CURVE_PUBLIC_KEY_OFFSET)[i]
						!= (vouch + crypto_box_SECRETKEYBYTES)[i]) {
			return 0;
		}
	}
	return 1;
}

static sfsc_uint8* client_persistent_key_from_initiate(
		zmtp_socket* socket) {
	//9 bytes header, 96 decrypted cookie, -8 because we needed 8 more bytes to start decoding the initiate box, finally crypto_box_ZEROBYTES
	return socket->buffer_space
			+ (_CURVE_DECRYPTION_OFFSET + 9 + 96 - 8 + crypto_box_ZEROBYTES);
}

static sfsc_int8 handshake_curve_decrypt_initiate(zmtp_socket* socket,
		zmtp_curve_state* state) {
	sfsc_uint8* buffer = effective_buffer(socket);
	zmtp_copy(COOKIE_NONCE + 8, buffer + 9, 16);
	zmtp_zero(buffer + 9, crypto_box_BOXZEROBYTES);
	//First open cookie box
	if (crypto_box_open(buffer + 9, buffer + 9,
	crypto_box_BOXZEROBYTES + 80, COOKIE_NONCE,
			state->peer_public_transient_key, state->secret_transient_key)
			== 0) { //These are the cookie keys
		//Get real transient keys
		zmtp_copy(state->peer_public_transient_key,
				buffer + 9 + crypto_box_ZEROBYTES, crypto_box_PUBLICKEYBYTES);
		zmtp_copy(state->secret_transient_key,
				buffer + 9 + crypto_box_ZEROBYTES + crypto_box_PUBLICKEYBYTES,
				crypto_box_SECRETKEYBYTES);
		if (b_fill_and_validate_nonce(state, buffer + 9 + 96, INITIATE_NONCE,
				0)) {
			sfsc_uint8* inititate_offset = buffer + 9 + 96 - 8;
			sfsc_uint8 initiate_box_len = (socket->buffer_index - (9 + 96 + 8));
			zmtp_zero(inititate_offset, crypto_box_BOXZEROBYTES);
			if (crypto_box_open(inititate_offset, inititate_offset,
			crypto_box_BOXZEROBYTES + initiate_box_len, INITIATE_NONCE,
					state->peer_public_transient_key,
					state->secret_transient_key) == 0) {
				sfsc_uint8* vouch_offset = buffer + 9 + 96
						- 8+ crypto_box_ZEROBYTES + crypto_box_PUBLICKEYBYTES;
				zmtp_copy(VOUCH_NONCE + 8, vouch_offset, 16);
				zmtp_zero(vouch_offset, 16);
				if (crypto_box_open(vouch_offset, vouch_offset,
				crypto_box_BOXZEROBYTES + 80, //80 is vouch
				VOUCH_NONCE, client_persistent_key_from_initiate(socket),
						state->secret_transient_key) == 0) {
					if (b_validate_vouch(state,
							vouch_offset + crypto_box_ZEROBYTES)) {
						sfsc_uint16 meta_offset = 9 + 96 - 8 +
						crypto_box_ZEROBYTES + crypto_box_PUBLICKEYBYTES + 96; //second 96 is total vouch size
						transkript_metadata(socket, meta_offset);
						socket->has_peer_metadata = 1;
						if (b_validate_socket_types(socket)) {
							socket->state =
									ZMTP_STATE_HANDSHAKE_CURVE_SERVER_VALIDATE_CLIENT_PERSISTENT_KEY;
							return ZMTP_OK;
						} else {
							return E_INCOMPATIBLE_SOCKET_TYPES;
						}
					} else {
						return E_BAD_CURVE_VOUCH;
					}
				} else {
					return E_CURVE_DECRYPTION_ERROR;
				}
			} else {
				return E_CURVE_DECRYPTION_ERROR;
			}
		} else {
			return E_CURVE_BAD_NONCE;
		}
	} else {
		return E_CURVE_DECRYPTION_ERROR;
	}
}

static sfsc_int8 handshake_curve_read_initiate(zmtp_socket* socket) {
	sfsc_uint8* buffer = effective_buffer(socket);
	sfsc_int8 read_result = read_next(socket);
	if (read_result == ZMTP_OK) {
		if (b_zmtp_socket_buffer_full(socket)) {
			if (IS_INITIATE_COMMAND_CURVE(socket, buffer)) {
				return handshake_curve_decrypt_initiate(socket,
						(zmtp_curve_state*) socket->mechanism_extra);
			} else if (IS_ERROR_COMMAND(socket, buffer)) {
				return E_ERROR_COMMAND;
			} else {
				return E_UNEXPECTED;
			}
		} else {
			zmtp_curve_state* state =
					(zmtp_curve_state*) socket->mechanism_extra;
			if (b_cookie_timed_out(state)) {
				delete_transient_keys(state);
				return E_CRUVE_INITIATE_TIMEOUT;
			} else {
				return ZMTP_OK;
			}
		}
	} else {
		return read_result;
	}
}

static sfsc_int8 handshake_curve_read_hello(zmtp_socket* socket,
sfsc_uint8* cookie_offset) {
	sfsc_uint8* buffer = effective_buffer(socket);
	if (b_curve_zmq_versions_compatible(buffer[6], buffer[7])) {
		for (sfsc_uint8 i = 8; i < 80; i++) { // 8 is the beginning of the 0 padding, and its 72 bytes long
			if (buffer[i] != 0) {
				return E_BAD_CURVE_HELLO;
			}
		}
		zmtp_curve_state* state = (zmtp_curve_state*) socket->mechanism_extra;
		if (b_fill_and_validate_nonce(state, buffer + 1 + 5 + 2 + 72 + 32,
				HELLO_NONCE, 1)) {
//Shift the client transient key
			zmtp_copy(buffer + 1 + 5 + 2, buffer + 1 + 5 + 2 + 72,
			crypto_box_PUBLICKEYBYTES);
//Also copy the client transient key to where we do encryption later on
//Dont save the clients transient key yet, according to the zmtp spec we should wait until initiate
			zmtp_copy(cookie_offset, buffer + 1 + 5 + 2 + 72,
			crypto_box_PUBLICKEYBYTES);
// Ensure that there are crypto_box_BOXZEROBYTES bytes so we can decrypt directly here
			sfsc_uint8* decryption_offset = buffer + 1 + 5 + 2 + 72 + 32 - 8;
			zmtp_zero(decryption_offset, crypto_box_BOXZEROBYTES);
			if (crypto_box_open(decryption_offset, decryption_offset,
					80 + crypto_box_BOXZEROBYTES, HELLO_NONCE, cookie_offset, //80 is the hello box size
					(state->persistent_keys + CURVE_SECRET_KEY_OFFSET)) == 0) {
				for (sfsc_uint8 i = 0; i < 64 + crypto_box_ZEROBYTES; i++) {
					if (decryption_offset[i] != 0) {
						return E_BAD_CURVE_HELLO;
					}
				}
				return ZMTP_OK;
			} else {
				return E_CURVE_DECRYPTION_ERROR;
			}
		} else {
			return E_CURVE_BAD_NONCE;
		}
	} else {
		return E_BAD_CURVE_VERSION;
	}
}

static sfsc_int8 handshake_curve_write_ready(zmtp_socket* socket) {
	zmtp_curve_state* state = (zmtp_curve_state*) socket->mechanism_extra;
	zmtp_zero(state->encryption_buffer, crypto_box_ZEROBYTES);
	sfsc_uint16 meta_len = meta_length(socket->metadata_buffer);
	sfsc_int64 length = 6 + 8 + crypto_box_CHIPER_LENGTH(meta_len);
	write_length_header(socket, length, _FLAG_COMMAND);
	const sfsc_uint8 ready_signature[] = { 5, 'R', 'E', 'A', 'D', 'Y' };
	zmtp_write(socket, ready_signature, 6);
	next_nonce(state, READY_NONCE);
	zmtp_write(socket, READY_NONCE + 16, 8);
	write_metadata_to_buffer(socket, meta_len,
			state->encryption_buffer + crypto_box_ZEROBYTES);
	if (crypto_box(state->encryption_buffer, state->encryption_buffer,
			meta_len + crypto_box_ZEROBYTES, READY_NONCE,
			state->peer_public_transient_key, state->secret_transient_key)
			== 0) {
		zmtp_write(socket, state->encryption_buffer + crypto_box_BOXZEROBYTES,
				crypto_box_CHIPER_LENGTH(meta_len));
		return ZMTP_OK;
	} else {
		return E_CURVE_ENCRYPTION_ERROR;
	}
}

static sfsc_int8 handshake_curve_write_welcome(zmtp_socket* socket,
sfsc_uint8* cookie_offset) {
	sfsc_uint8* buffer = effective_buffer(socket);
	zmtp_curve_state* state = (zmtp_curve_state*) socket->mechanism_extra;
	write_length_header(socket, 168, _FLAG_COMMAND);
	const sfsc_uint8 welcome_signature[] = { 7, 'W', 'E', 'L', 'C', 'O', 'M',
			'E' };
	random_bytes(WELCOME_NONCE + 8, 16);
	zmtp_write(socket, welcome_signature, 8);
	zmtp_write(socket, WELCOME_NONCE + 8, 16);
	crypto_box_keypair(state->encryption_buffer + crypto_box_ZEROBYTES,
			cookie_offset + crypto_box_PUBLICKEYBYTES);
	crypto_box_keypair(state->peer_public_transient_key,
			state->secret_transient_key); //borrow this buffers
	random_bytes(COOKIE_NONCE + 8, 16);
	if (crypto_box(cookie_offset - crypto_box_ZEROBYTES,
			cookie_offset - crypto_box_ZEROBYTES,
			crypto_box_ZEROBYTES + crypto_box_PUBLICKEYBYTES
					+ crypto_box_SECRETKEYBYTES, COOKIE_NONCE,
			state->peer_public_transient_key, state->secret_transient_key) //These are actually the cookie keys
	== 0) {
		zmtp_copy(
				state->encryption_buffer + crypto_box_ZEROBYTES
						+ crypto_box_PUBLICKEYBYTES, COOKIE_NONCE + 8, 16);
		if (crypto_box(state->encryption_buffer, state->encryption_buffer,
		crypto_box_ZEROBYTES + crypto_box_PUBLICKEYBYTES + 96, WELCOME_NONCE,
				buffer + 1 + 5 + 2,
				(state->persistent_keys + CURVE_SECRET_KEY_OFFSET)) == 0) //96 is cookie size
				{
			zmtp_write(socket,
					state->encryption_buffer + crypto_box_BOXZEROBYTES, 144);
			note_cookie_time(state);
			return ZMTP_OK;
		} else {
			return E_CURVE_ENCRYPTION_ERROR;
		}
	} else {
		return E_CURVE_ENCRYPTION_ERROR;
	}
}

sfsc_int8 curve_client_persistent_key_ok(zmtp_socket* socket) {
	if (socket->mechanism == MECHANISM_CURVE && socket->as_server
			&& socket->state
					== ZMTP_STATE_HANDSHAKE_CURVE_SERVER_VALIDATE_CLIENT_PERSISTENT_KEY) {
		sfsc_int8 result = handshake_curve_write_ready(socket);
		if (result == ZMTP_OK) {
			socket->state = ZMTP_STATE_OPERATING;
			reset(socket);
			return ZMTP_OK;
		} else {
			return result;
		}
	} else {
		return E_UNEXPECTED;
	}
}

sfsc_int8 handshake_curve(zmtp_socket* socket) {
	if (socket->as_server) {
		if (socket->state == ZMTP_STATE_HANDSHAKE) {
			socket->state = ZMTP_STATE_HANDSHAKE_CURVE_SERVER_EXPECT_HELLO;
		}
		if (socket->state == ZMTP_STATE_HANDSHAKE_CURVE_SERVER_EXPECT_HELLO) {
			sfsc_uint8* buffer = effective_buffer(socket);
			sfsc_int8 read_result = read_next(socket);
			if (read_result == ZMTP_OK) {
				if (b_zmtp_socket_buffer_full(socket)) {
					if (IS_HELLO_COMMAND_CURVE(socket, buffer)) {
						zmtp_curve_state* state =
								(zmtp_curve_state*) socket->mechanism_extra;
						sfsc_uint8* cookie_offset = state->encryption_buffer
								+ crypto_box_ZEROBYTES
								+ crypto_box_PUBLICKEYBYTES
								+ crypto_box_ZEROBYTES;
						sfsc_int8 op_result = handshake_curve_read_hello(socket,
								cookie_offset);
						if (op_result == ZMTP_OK) {
							socket->state =
							ZMTP_STATE_HANDSHAKE_CURVE_SERVER_EXPECT_INITIATE;
							op_result = handshake_curve_write_welcome(socket,
									cookie_offset);
							if (op_result == ZMTP_OK) {
								reset(socket);
								return ZMTP_OK;
							} else {
								return op_result;
							}
						} else {
							return op_result;
						}
					} else if (IS_ERROR_COMMAND(socket, buffer)) {
						return E_ERROR_COMMAND;
					} else {
						return E_UNEXPECTED;
					}
				} else {
					return ZMTP_OK;
				}
			} else {
				return read_result;
			}

		}
		if (socket->state == ZMTP_STATE_HANDSHAKE_CURVE_SERVER_EXPECT_INITIATE) {
			return handshake_curve_read_initiate(socket);
		}
		if (socket->state
				== ZMTP_STATE_HANDSHAKE_PLAIN_SERVER_VALIDATE_CREDENTIALS) {
			return ZMTP_OK;
		}
		return E_UNREACHABLE;
	} else {
		return E_UNREACHABLE; //FIXME Client Mode not implemented yet
	}
}

sfsc_int8 curve_open_message(zmtp_socket* socket) {
	sfsc_uint8* buffer = effective_buffer(socket);
	if (IS_MESSAGE_COMMAND_CURVE(socket, buffer)) {
		zmtp_curve_state* state = (zmtp_curve_state*) socket->mechanism_extra;
		if (b_fill_and_validate_nonce(state, buffer + 8, MESSAGE_NONCE, 0)) {
			MESSAGE_NONCE[15] = socket->as_server ? 'C' : 'S';
			zmtp_zero(buffer, crypto_box_BOXZEROBYTES);
			if (crypto_box_open(buffer, buffer, socket->buffer_index,
					MESSAGE_NONCE, state->peer_public_transient_key,
					state->secret_transient_key) == 0) {
				sfsc_uint8 flags = *(buffer + crypto_box_ZEROBYTES);
				socket->last_message = (flags & 0x01) == 0;
				socket->buffer_index = socket->buffer_index
						- _CURVE_MESSAGE_OFFSET;
				socket->expected_size = socket->expected_size
						- _CURVE_MESSAGE_OFFSET;
				return ZMTP_OK;
			} else {
				return E_CURVE_DECRYPTION_ERROR;
			}
		} else {
			return E_CURVE_BAD_NONCE;
		}
	} else {
		return E_UNEXPECTED;
	}
}

sfsc_int8 begin_message_curve(zmtp_socket* socket,
sfsc_size len, sfsc_int8 as_last_message) {
	if (CURVE_ENCRYPTION_BUFFER_SIZE < (len + 1 + crypto_box_ZEROBYTES)) {
		return E_BUFFER_INSUFFICIENT;
	} else {
		zmtp_curve_state* state = (zmtp_curve_state*) socket->mechanism_extra;
		zmtp_zero(state->encryption_buffer, crypto_box_ZEROBYTES);
		sfsc_size payload_len = 1 + len;
		write_length_header(socket,
				8 + 8 + crypto_box_CHIPER_LENGTH(payload_len),
				_FLAG_LAST_MESSAGE);
		const sfsc_uint8 message_signature[] = { 7, 'M', 'E', 'S', 'S', 'A',
				'G', 'E' };
		zmtp_write(socket, message_signature, 8);
		next_nonce(state, MESSAGE_NONCE);
		MESSAGE_NONCE[15] = socket->as_server ? 'S' : 'C';
		zmtp_write(socket, MESSAGE_NONCE + 16, 8);
		state->encryption_buffer[crypto_box_ZEROBYTES] =
				as_last_message ? 0 : 1;
		state->encryption_buffer_multipart_index = crypto_box_ZEROBYTES + 1;
		return ZMTP_OK;
	}
}
sfsc_int8 continue_message_curve(zmtp_socket* socket,
		const sfsc_uint8 message[],
		sfsc_size len, sfsc_int8 done) {
	zmtp_curve_state* state = (zmtp_curve_state*) socket->mechanism_extra;
	if (len > 0) {
		zmtp_copy(
				(state->encryption_buffer
						+ state->encryption_buffer_multipart_index), message,
				len);
		state->encryption_buffer_multipart_index += len;
	}
	if (done) {
		sfsc_size payload_len = state->encryption_buffer_multipart_index
				- crypto_box_ZEROBYTES;
		if (crypto_box(state->encryption_buffer, state->encryption_buffer,
				payload_len + crypto_box_ZEROBYTES, MESSAGE_NONCE,
				state->peer_public_transient_key, state->secret_transient_key)
				== 0) {
			zmtp_write(socket,
					state->encryption_buffer + crypto_box_BOXZEROBYTES,
					crypto_box_CHIPER_LENGTH(payload_len));
			return ZMTP_OK;
		} else {
			return E_CURVE_ENCRYPTION_ERROR;
		}
	} else {
		return ZMTP_OK;
	}
}

sfsc_uint8* curve_client_persistent_key_during_handshake(zmtp_socket* socket) {
	if (socket->state == ZMTP_STATE_HANDSHAKE_PLAIN_SERVER_VALIDATE_CREDENTIALS) {
		return client_persistent_key_from_initiate(socket);
	} else {
		return NULL;
	}
}
#endif