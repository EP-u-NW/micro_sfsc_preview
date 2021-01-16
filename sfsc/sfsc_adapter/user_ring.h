/*
 * ring_buffer.h
 *
 *  Created on: 28.09.2020
 *      Author: Eric
 */

#ifndef SRC_USER_RING_H_
#define SRC_USER_RING_H_

#include "../platform/sfsc_types.h"
#include "sfsc_adapter_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _user_ring{
	sfsc_uint8 buffer[USER_RING_SIZE];
	sfsc_size read_index;
	sfsc_size write_index;
	sfsc_uint8 wrapped_writer;
	sfsc_bool next_is_payload:1;
	sfsc_size last_written_len;
	sfsc_size left_free;
} user_ring;

#define user_ring_DEFAULT_INIT {{0},0,0,0,0,0,0}

sfsc_uint8* read_topic(user_ring* ring, sfsc_size* out_len);
sfsc_uint8* read_payload(user_ring* ring, sfsc_size* out_len);
void advance_topic_and_payload(user_ring* ring);
sfsc_bool b_write_topic(user_ring* ring,sfsc_uint8* element,sfsc_size len);
sfsc_bool b_write_payload(user_ring* ring,sfsc_uint8* element,sfsc_size len);
void forget_last_written_if_topic(user_ring* ring);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_USER_RING_H_ */
