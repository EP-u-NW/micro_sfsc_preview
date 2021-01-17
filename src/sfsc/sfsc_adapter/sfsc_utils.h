/*
 * sfsc_utils.h
 *
 *  Created on: 06.10.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_UTILS_H_
#define SRC_SFSC_UTILS_H_

#include "../zmtp/zmtp.h"
#include "sfsc_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif

#define sfsc_composite_buffer_DEFAULT_INIT {NULL,0}
typedef struct _sfsc_composite_buffer {
	sfsc_buffer* buffers;
	sfsc_size count;
} sfsc_composite_buffer;

typedef struct _encode_submsg_to_bytes {
	const pb_msgdesc_t* fields;
	const void* src_struct;
} encode_submsg_to_bytes;


#define GENERIC_ISTREAM(decode_state) {generic_decoding_callback,(decode_state),(decode_state)->msg_left,0}
typedef struct _generic_decode_state {
	void* extra;
	sfsc_uint8* msg;
	sfsc_size msg_left;
} generic_decode_state;
sfsc_bool generic_decoding_callback(pb_istream_t *stream, uint8_t *buf, sfsc_size count);

sfsc_bool b_bytes_equal(const sfsc_uint8* a, const sfsc_uint8* b, sfsc_size len);
sfsc_bool b_composite_bytes_equal(sfsc_composite_buffer* a, const sfsc_uint8* b, sfsc_size len_of_b);

sfsc_bool b_encode_buffer_for_pb(pb_ostream_t *stream, const pb_field_iter_t *field, void * const *arg);
sfsc_bool b_encode_composite_buffer_for_pb(pb_ostream_t *stream,	const pb_field_iter_t *field, void * const *arg);
sfsc_bool b_encode_submsg_to_bytes_callback(pb_ostream_t *stream, const pb_field_iter_t *field, void * const *arg);

sfsc_bool b_encode_and_publish(const sfsc_uint8* topic, sfsc_size topic_len, zmtp_socket* socket, const pb_msgdesc_t *fields, const void *src_struct);
sfsc_bool b_encode_and_publish_with_request_reply_pattern(const sfsc_uint8* topic,sfsc_size topic_len,const sfsc_uint8* reply_topic,sfsc_uint16 reply_topic_len,sfsc_int64 expected_reply_id, zmtp_socket* socket,const pb_msgdesc_t *fields, const void *src_struct);
sfsc_bool b_encode_and_publish_with_request_composite_reply_pattern(const sfsc_uint8* topic,sfsc_size topic_len,sfsc_composite_buffer* reply_info,sfsc_int64 expected_reply_id, zmtp_socket* socket,const pb_msgdesc_t *fields, const void *src_struct);
sfsc_bool b_strip_request_reply_pattern(sfsc_uint8* msg,sfsc_size msg_len,sfsc_uint8** out_stripped,sfsc_size* out_stripped_length, sfsc_int64* out_reply_id);

sfsc_bool b_strip_reqrepack_pattern_reply(sfsc_uint8* msg,sfsc_size msg_len,sfsc_uint8** out_paylaod,sfsc_size* out_payload_len,sfsc_int32* out_reply_id,sfsc_buffer* out_ack,sfsc_int32* out_ack_id);
sfsc_bool b_strip_reqrepack_pattern_request(sfsc_uint8* msg,sfsc_size msg_len,sfsc_uint8** out_paylaod,sfsc_size* out_payload_len,sfsc_int32* out_reply_id,sfsc_buffer* out_reply_topic,sfsc_bool is_channel_request);

sfsc_int8 write_composite_subscription_message(zmtp_socket* socket, sfsc_composite_buffer* buf, sfsc_bool subscribe);

void sfsc_copy(sfsc_uint8* dst, const sfsc_uint8* src, sfsc_size len);
void random_uuid_if_needed(sfsc_uint8 target[36]);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_UTILS_H_ */
