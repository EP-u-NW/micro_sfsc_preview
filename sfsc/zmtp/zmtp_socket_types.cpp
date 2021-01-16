/*
 * zmtp_socket_types.cpp
 *
 *  Created on: 28.10.2020
 *      Author: Eric
 */

#include "zmtp_socket_types.h"
#include "zmtp.h"

const sfsc_uint8 socket_type_meta_key[SOCKET_TYPE_META_KEY_LENGTH] = { 'S', 'o',
		'c', 'k', 'e', 't', '-', 'T', 'y', 'p', 'e' };

const sfsc_uint8 socket_type_names[] = { 'R', 'E', 'Q', 'R', 'E', 'P', 'P', 'U',
		'B', 'S', 'U', 'B', 'X', 'P', 'U', 'B', 'X', 'S', 'U', 'B', 'P', 'U',
		'S', 'H', 'P', 'U', 'L', 'L', 'P', 'A', 'I', 'R', 'R', 'O', 'U', 'T',
		'E', 'R', 'D', 'E', 'A', 'L', 'E', 'R' };

static sfsc_bool b_socket_types_compatible(sfsc_uint8 a, sfsc_uint8 b) {
	return ((a == ZMTP_SOCKET_TYPE_REQ
			&& (b == ZMTP_SOCKET_TYPE_REP || b == ZMTP_SOCKET_TYPE_ROUTER))
			|| (a == ZMTP_SOCKET_TYPE_REP
					&& (b == ZMTP_SOCKET_TYPE_REQ || b == ZMTP_SOCKET_TYPE_DEALER))
			|| (a == ZMTP_SOCKET_TYPE_DEALER
					&& (b == ZMTP_SOCKET_TYPE_REP || b == ZMTP_SOCKET_TYPE_DEALER
							|| b == ZMTP_SOCKET_TYPE_ROUTER))
			|| (a == ZMTP_SOCKET_TYPE_ROUTER
					&& (b == ZMTP_SOCKET_TYPE_REQ || b == ZMTP_SOCKET_TYPE_DEALER
							|| b == ZMTP_SOCKET_TYPE_ROUTER))
			|| ((a == ZMTP_SOCKET_TYPE_PUB || a == ZMTP_SOCKET_TYPE_XPUB)
					&& (b == ZMTP_SOCKET_TYPE_SUB || b == ZMTP_SOCKET_TYPE_XSUB))
			|| ((a == ZMTP_SOCKET_TYPE_SUB || a == ZMTP_SOCKET_TYPE_XSUB)
					&& (b == ZMTP_SOCKET_TYPE_PUB || b == ZMTP_SOCKET_TYPE_XPUB))
			|| (a == ZMTP_SOCKET_TYPE_PUSH && b == ZMTP_SOCKET_TYPE_PULL)
			|| (a == ZMTP_SOCKET_TYPE_PULL && b == ZMTP_SOCKET_TYPE_PUSH)
			|| (a == ZMTP_SOCKET_TYPE_PAIR && b == ZMTP_SOCKET_TYPE_PAIR));
}

static sfsc_int8 socket_type_by_name(sfsc_uint8* name, sfsc_uint8 len) {
	switch (len) {
	case 6:
		return (name[4] == 'E' && name[5] == 'R' ?
				(name[0] == 'D' && name[1] == 'E' && name[2] == 'A'
						&& name[3] == 'L' ?
						ZMTP_SOCKET_TYPE_DEALER :
						(name[0] == 'R' && name[1] == 'O' && name[2] == 'U'
								&& name[3] == 'T' ?
						ZMTP_SOCKET_TYPE_ROUTER :
													ZMTP_SOCKET_TYPE_ERROR)) :
				ZMTP_SOCKET_TYPE_ERROR);
	case 4:
		return (name[0] == 'P' ?
				(name[1] == 'U' ?
						(name[2] == 'S' && name[3] == 'H' ?
								ZMTP_SOCKET_TYPE_PUSH :
								(name[2] == 'L' && name[3] == 'L' ?
								ZMTP_SOCKET_TYPE_PULL :
																	ZMTP_SOCKET_TYPE_ERROR)) :
						(name[1] == 'A' && name[2] == 'I' && name[3] == 'R' ?
								ZMTP_SOCKET_TYPE_PAIR :
								ZMTP_SOCKET_TYPE_ERROR)) :
				(name[0] == 'X' && name[2] == 'U' && name[3] == 'B' ?
						(name[1] == 'S' ?
						ZMTP_SOCKET_TYPE_XSUB :
											(name[1] == 'P' ?
											ZMTP_SOCKET_TYPE_XPUB :
																ZMTP_SOCKET_TYPE_ERROR)) :
						ZMTP_SOCKET_TYPE_ERROR));
	case 3:
		return (name[0] == 'R' && name[1] == 'E' ?
				(name[2] == 'Q' ?
				ZMTP_SOCKET_TYPE_REQ :

									(name[3] == 'P' ?
									ZMTP_SOCKET_TYPE_REP :

														ZMTP_SOCKET_TYPE_ERROR)) :
				(name[1] == 'U' && name[2] == 'B' ?
						(name[0] == 'S' ?
						ZMTP_SOCKET_TYPE_SUB :

											(name[0] == 'P' ?
											ZMTP_SOCKET_TYPE_PUB :

																ZMTP_SOCKET_TYPE_ERROR)) :
						ZMTP_SOCKET_TYPE_ERROR));
	default:
		return ZMTP_SOCKET_TYPE_ERROR;
	}
}

sfsc_bool b_validate_socket_types(zmtp_socket* socket) {
	sfsc_uint16 type_len = 0;
	sfsc_uint8* type_name = get_meta(socket->peer_metadata_buffer,
			socket_type_meta_key, SOCKET_TYPE_META_KEY_LENGTH, &type_len);
	sfsc_uint8 peer_type = socket_type_by_name(type_name, type_len);
	type_name = get_meta(socket->metadata_buffer, socket_type_meta_key,
	SOCKET_TYPE_META_KEY_LENGTH, &type_len);
	sfsc_uint8 my_type = socket_type_by_name(type_name, type_len);
	return b_socket_types_compatible(my_type, peer_type);
}

sfsc_bool b_socket_type_name(sfsc_int8 type,const sfsc_uint8** name,
sfsc_uint8* len) {
	if (type <= 10) {
		sfsc_uint8 type_offset = (
				type <= 3 ?
						(3 * type) :
						(type <= 8 ? 12 + (type - 4) * 4 : 32 + (type - 9) * 6));
		*len = type <= 3 ? 3 : (type <= 8 ? 4 : (type <= 10 ? 6 : 0));
		*name = socket_type_names + type_offset;
		return 1;
	} else {
		return 0;
	}
}
