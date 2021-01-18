/*
 * user_ring.c
 *
 *  Created on: 02.10.2020
 *      Author: Eric
 */

#include "user_ring.h"
#include "sfsc_utils.h"


#define _ELEMENT_LEN_BYTES sizeof(sfsc_size)

static sfsc_uint8* get(user_ring* ring, sfsc_size read_index,
sfsc_size len, sfsc_uint8* wrapped_writer) {
	if (read_index < ring->write_index) {
		if (read_index + len <= ring->write_index) {
			return ring->buffer + read_index;
		} else {
			return NULL;
		}
	} else if (ring->write_index < read_index || (*wrapped_writer)) { // ... or (equal and wrapped_writer)
		if (read_index + len < USER_RING_SIZE) {
			return ring->buffer + read_index;
		} else if (len <= ring->write_index) {
			*wrapped_writer = 0;
			return ring->buffer;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

static sfsc_uint8* getget(user_ring* ring, sfsc_size read_index,
sfsc_size* out_len, sfsc_uint8* wrapped_writer) {
	sfsc_uint8* marker = get(ring, read_index, _ELEMENT_LEN_BYTES,
			wrapped_writer);
	if (marker != NULL) {
		*out_len = *((sfsc_size*) marker);
		sfsc_size offset = read_index + _ELEMENT_LEN_BYTES;
		return get(ring, offset < USER_RING_SIZE ? offset : 0, *out_len,
				wrapped_writer);
	} else {
		return NULL;
	}
}

static sfsc_uint8* read_topic_internal(user_ring* ring,
sfsc_size* out_len, sfsc_uint8* wrapped_writer) {
	return getget(ring, ring->read_index, out_len, wrapped_writer);
}

sfsc_uint8* read_topic(user_ring* ring, sfsc_size* out_len) {
	sfsc_uint8 wrapped_writer = ring->wrapped_writer;
	return read_topic_internal(ring, out_len, &wrapped_writer);
}
static sfsc_uint8* read_payload_internal(user_ring* ring,
sfsc_size* out_len, sfsc_uint8 *wrapped_writer) {
	sfsc_size topic_skip;
	sfsc_uint8* marker = read_topic_internal(ring, &topic_skip, wrapped_writer);
	if (marker != NULL) {
		sfsc_size offset = (marker - ring->buffer) + topic_skip;
		offset = offset < USER_RING_SIZE ? offset : 0;
		return getget(ring, offset, out_len, wrapped_writer);
	} else {
		return NULL;
	}
}
sfsc_uint8* read_payload(user_ring* ring, sfsc_size* out_len) {
	sfsc_uint8 wrapped_writer = ring->wrapped_writer;
	return read_payload_internal(ring, out_len, &wrapped_writer);
}
void advance_topic_and_payload(user_ring* ring) {
	sfsc_size skip;
	sfsc_uint8* marker = read_payload_internal(ring, &skip,
			&(ring->wrapped_writer));
	if (marker != NULL) {
		sfsc_size offset = (marker - ring->buffer) + skip;
		ring->read_index = offset < USER_RING_SIZE ? offset : 0;
	}
}

static sfsc_size write(user_ring* ring, sfsc_size write_index,
sfsc_uint8* element, sfsc_size len, sfsc_bool* b_success,
sfsc_uint8* wrapped_writer) {
	if (ring->read_index < write_index
			|| (ring->read_index == write_index && (*wrapped_writer == 0))) {
		if (write_index + len <= USER_RING_SIZE) {
			sfsc_copy(ring->buffer + write_index, element, len);
			ring->last_written_len = len;
			*b_success = 1;
			ring->left_free = 0;
			if (write_index + len == USER_RING_SIZE) {
				*wrapped_writer = 1;
				return 0;
			} else {
				return write_index + len;
			}
		} else if (len < ring->read_index) {
			sfsc_copy(ring->buffer, element, len);
			ring->last_written_len = len;
			*b_success = 1;
			ring->left_free = USER_RING_SIZE - write_index;
			*wrapped_writer = 1;
			return len;
		} else {
			*b_success = 0;
			return 0;
		}
	} else if (write_index + len < ring->read_index) {
		sfsc_copy(ring->buffer + write_index, element, len);
		ring->last_written_len = len;
		*b_success = 1;
		ring->left_free = 0;
		return write_index + len;
	} else {
		*b_success = 0;
		return 0;
	}

}

static sfsc_bool b_writewrite(user_ring* ring, sfsc_uint8* element,
sfsc_size len) {
	sfsc_bool success;
	sfsc_uint8 wrapped_writer = ring->wrapped_writer;
	sfsc_size write_index = write(ring, ring->write_index,
			(sfsc_uint8*) (&len), _ELEMENT_LEN_BYTES, &success, &wrapped_writer);
	if (success) {
		write_index = write(ring, write_index, element, len, &success,
				&wrapped_writer);
		if (success) {
			ring->write_index = write_index;
			ring->wrapped_writer = wrapped_writer;
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

sfsc_bool b_write_topic(user_ring* ring, sfsc_uint8* element,
sfsc_size len) {
	if (b_writewrite(ring, element, len)) {
		ring->next_is_payload = 1;
		return 1;
	} else {
		return 0;
	}
}

sfsc_bool b_write_payload(user_ring* ring, sfsc_uint8* element,
sfsc_size len) {
	if (b_writewrite(ring, element, len)) {
		ring->next_is_payload = 0;
		return 1;
	} else {
		return 0;
	}
}

void forget_last_written_if_topic(user_ring* ring) {
	if (ring->next_is_payload == 1) {
		if (ring->read_index < ring->write_index) {
			ring->write_index -= (_ELEMENT_LEN_BYTES + ring->last_written_len);
		} else if (ring->write_index < ring->read_index
				|| (ring->wrapped_writer)) {
			ring->write_index -= ring->last_written_len;
			if (ring->write_index == 0) {
				ring->write_index = USER_RING_SIZE - _ELEMENT_LEN_BYTES
						- ring->left_free;
				ring->wrapped_writer = 0;
			} else {
				ring->write_index -= _ELEMENT_LEN_BYTES;
			}
		}
		ring->next_is_payload = 0;
	}
}
