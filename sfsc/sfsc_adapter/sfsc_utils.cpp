/*
 * sfsc_utils.c
 *
 *  Created on: 06.10.2020
 *      Author: Eric
 */
#include "sfsc_utils.h"

#include "../platform/sfsc_strings.h"
#include "../proto/pb.h"
#include "../proto/pb_decode.h"
#include "../proto/pb_encode.h"

const sfsc_uint8 _zmtp_subscribe[] = {1};
const sfsc_uint8 _zmtp_unsubscribe[] = {0};

sfsc_bool b_bytes_equal(const sfsc_uint8* a, const sfsc_uint8* b,
                        sfsc_size len) {
    for (sfsc_size i = 0; i < len; i++) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static sfsc_uint16 total_composite_length(sfsc_composite_buffer* buffer) {
    sfsc_uint16 total_length = 0;
    for (sfsc_size i = 0; i < (buffer->count); i++) {
        total_length += (buffer->buffers + i)->length;
    }
    return total_length;
}

sfsc_bool b_composite_bytes_equal(sfsc_composite_buffer* a, const sfsc_uint8* b,
                                  sfsc_uint16 b_len) {
    sfsc_uint16 a_len = total_composite_length(a);
    if (a_len == b_len) {
        sfsc_uint16 b_offset = 0;
        for (sfsc_size i = 0; i < a->count; i++) {
            if (b_bytes_equal((a->buffers + i)->content, (b + b_offset),
                              (a->buffers + i)->length)) {
                b_offset += (a->buffers + i)->length;
            } else {
                return 0;
            }
        }
        return 1;
    } else {
        return 0;
    }
}

static sfsc_bool b_out_callback(pb_ostream_t* stream, const sfsc_uint8* buf,
                                sfsc_size len) {
    zmtp_socket* socket = (zmtp_socket*)stream->state;
    return continue_message_multipart(
               socket, buf, len,
               (len + stream->bytes_written == stream->max_size)) == ZMTP_OK;
}

sfsc_bool b_encode_and_publish(const sfsc_uint8* topic, sfsc_size topic_len,
                               zmtp_socket* socket, const pb_msgdesc_t* fields,
                               const void* src_struct) {
    sfsc_size out_len;
    if (pb_get_encoded_size(&out_len, fields, src_struct)) {
        sfsc_int8 op_result = write_message(socket, topic, topic_len, 0);
        if (op_result != ZMTP_OK) {
            return 0;
        }
        op_result = begin_message_multipart(socket, (sfsc_size)out_len, 1);
        if (op_result != ZMTP_OK) {
            return 0;
        }
        pb_ostream_t ostream = {b_out_callback, socket, (sfsc_uint16)out_len, 0,
                                NULL};
        return pb_encode(&ostream, fields, src_struct);
    } else {
        return 0;
    }
}

sfsc_bool b_encode_buffer_for_pb(pb_ostream_t* stream,
                                 const pb_field_iter_t* field,
                                 void* const* arg) {
    sfsc_buffer* buffer = (sfsc_buffer*)(*arg);
    if (buffer->length > 0) {  // TODO check
        if (pb_encode_tag_for_field(stream, field)) {
            return pb_encode_string(stream, buffer->content, buffer->length);
        } else {
            return 0;
        }
    } else {
        return 1;
    }
}

sfsc_bool b_encode_composite_buffer_for_pb(pb_ostream_t* stream,
                                           const pb_field_iter_t* field,
                                           void* const* arg) {
    sfsc_composite_buffer* buffer = (sfsc_composite_buffer*)(*arg);
    if (buffer->count > 0) {  // TODO check
        if (pb_encode_tag_for_field(stream, field)) {
            sfsc_size size = total_composite_length(buffer);
            if (pb_encode_varint(stream, size)) {
                for (sfsc_size i = 0; i < buffer->count; i++) {
                    if (pb_write(stream, (buffer->buffers + i)->content,
                                 (buffer->buffers + i)->length) == 0) {
                        return 0;
                    }
                }
                return 1;
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    } else {
        return 1;
    }
}

sfsc_bool b_encode_submsg_to_bytes_callback(pb_ostream_t* stream,
                                            const pb_field_iter_t* field,
                                            void* const* arg) {
    encode_submsg_to_bytes* encode = (encode_submsg_to_bytes*)(*arg);
    if (pb_encode_tag_for_field(stream, field)) {
        return pb_encode_submessage(stream, encode->fields, encode->src_struct);
    } else {
        return 0;
    }
}

static sfsc_bool b_encode_and_publish_with_request_reply_pattern_internal(
    sfsc_RequestPattern* request, const sfsc_uint8* topic,
    sfsc_uint16 topic_len, sfsc_int64 expected_reply_id, zmtp_socket* socket,
    const pb_msgdesc_t* fields, const void* src_struct) {
    encode_submsg_to_bytes arg = {fields, src_struct};

    request->expected_reply_id = expected_reply_id;
    request->request_payload.funcs.encode = b_encode_submsg_to_bytes_callback;
    request->request_payload.arg = &arg;

    return b_encode_and_publish(topic, topic_len, socket,
                                sfsc_RequestPattern_fields, request);
}

sfsc_bool b_encode_and_publish_with_request_reply_pattern(
    const sfsc_uint8* topic, sfsc_size topic_len, const sfsc_uint8* reply_topic,
    sfsc_size reply_topic_len, sfsc_int64 expected_reply_id,
    zmtp_socket* socket, const pb_msgdesc_t* fields, const void* src_struct) {
    sfsc_buffer reply = {reply_topic, reply_topic_len};

    sfsc_RequestPattern request = sfsc_RequestPattern_init_default;
    request.reply_topic.funcs.encode = b_encode_buffer_for_pb;
    request.reply_topic.arg = &reply;

    return b_encode_and_publish_with_request_reply_pattern_internal(
        &request, topic, topic_len, expected_reply_id, socket, fields,
        src_struct);
}

sfsc_bool b_encode_and_publish_with_request_composite_reply_pattern(
    const sfsc_uint8* topic, sfsc_size topic_len,
    sfsc_composite_buffer* reply_info, sfsc_int64 expected_reply_id,
    zmtp_socket* socket, const pb_msgdesc_t* fields, const void* src_struct) {
    sfsc_RequestPattern request = sfsc_RequestPattern_init_default;
    request.reply_topic.funcs.encode = b_encode_composite_buffer_for_pb;
    request.reply_topic.arg = reply_info;

    return b_encode_and_publish_with_request_reply_pattern_internal(
        &request, topic, topic_len, expected_reply_id, socket, fields,
        src_struct);
}

typedef struct _strip_helper {
    sfsc_uint8* msg;
    sfsc_size len;
} strip_helper;

static sfsc_bool b_strip_callback(pb_istream_t* stream,
                                  const pb_field_t* pb_field, void** arg) {
    strip_helper* stripped = (strip_helper*)(*arg);
    generic_decode_state* state = (generic_decode_state*)stream->state;
    stripped->len = stream->bytes_left;
    stripped->msg = state->msg;
    pb_read(stream, NULL, stream->bytes_left);
    return 1;
}

sfsc_bool b_strip_request_reply_pattern(sfsc_uint8* msg, sfsc_size msg_len,
                                        sfsc_uint8** out_stripped,
                                        sfsc_uint16* out_stripped_length,
                                        sfsc_int64* out_reply_id) {
    sfsc_ReplyPattern reply = sfsc_ReplyPattern_init_default;
    strip_helper stripped;
    reply.reply_payload.funcs.decode = b_strip_callback;
    reply.reply_payload.arg = &stripped;
    generic_decode_state state = {NULL, msg, msg_len};
    pb_istream_t istream = GENERIC_ISTREAM(&state);
    if (pb_decode(&istream, sfsc_ReplyPattern_fields, &reply)) {
        *out_reply_id = reply.reply_id;
        *out_stripped_length = stripped.len;
        *out_stripped = stripped.msg;
        return 1;
    } else {
        return 0;
    }
}

sfsc_bool b_strip_reqrepack_pattern_reply(sfsc_uint8* msg, sfsc_size msg_len,
                                          sfsc_uint8** out_paylaod,
                                          sfsc_uint16* out_payload_len,
                                          sfsc_int32* out_reply_id,
                                          sfsc_buffer* out_ack,
                                          sfsc_int32* out_ack_id) {
    sfsc_Reply reply = sfsc_Reply_init_default;
    strip_helper ack_topic_stripper;
    reply.acknowledge_topic.topic.funcs.decode = b_strip_callback;
    reply.acknowledge_topic.topic.arg = &ack_topic_stripper;
    strip_helper payload_stripper = {NULL, 0};
    reply.reply_payload.funcs.decode = b_strip_callback;
    reply.reply_payload.arg = &payload_stripper;
    generic_decode_state state = {NULL, msg, msg_len};
    pb_istream_t istream = GENERIC_ISTREAM(&state);
    if (pb_decode(&istream, sfsc_Reply_fields, &reply)) {
        *out_payload_len = payload_stripper.len;
        *out_paylaod = payload_stripper.msg;
        *out_reply_id = reply.reply_id;
        out_ack->content = ack_topic_stripper.msg;
        out_ack->length = ack_topic_stripper.len;
        *out_ack_id = reply.expected_acknowledge_id;
        return 1;
    } else {
        return 0;
    }
}

#define _PAYLOAD_STRIPPER 0
#define _REPLY_TOPIC_STRIPPER 1
static sfsc_bool b_prepare_request_submsg(pb_istream_t* stream,
                                          const pb_field_t* field, void** arg) {
    generic_decode_state* state = (generic_decode_state*)(stream->state);
    strip_helper* stripper = (strip_helper*)state->extra;
    if (field->tag == sfsc_RequestOrAcknowledge_request_tag) {
        sfsc_RequestOrAcknowledge_Request* request =
            (sfsc_RequestOrAcknowledge_Request*)field->pData;

        request->request_payload.funcs.decode = b_strip_callback;
        request->request_payload.arg = stripper + _PAYLOAD_STRIPPER;
        request->reply_topic.topic.funcs.decode = b_strip_callback;
        request->reply_topic.topic.arg = stripper + _REPLY_TOPIC_STRIPPER;

        return 1;
    } else {
        return 0;
    }
}

sfsc_bool b_strip_reqrepack_pattern_request(sfsc_uint8* msg, sfsc_size msg_len,
                                            sfsc_uint8** out_paylaod,
                                            sfsc_uint16* out_payload_len,
                                            sfsc_int32* out_reply_id,
                                            sfsc_buffer* out_reply_topic,
                                            sfsc_uint8 is_channel_request) {
    strip_helper strippers[2];

    sfsc_RequestOrAcknowledge request_struct =
        sfsc_RequestOrAcknowledge_init_default;
    request_struct.cb_request_or_acknowledge.funcs.decode =
        b_prepare_request_submsg;

    generic_decode_state state = {strippers, msg, msg_len};
    pb_istream_t istream = GENERIC_ISTREAM(&state);
    if (pb_decode(&istream, sfsc_RequestOrAcknowledge_fields,
                  &request_struct)) {
        *out_reply_id =
            request_struct.request_or_acknowledge.request.expected_reply_id;
        out_reply_topic->content = strippers[_REPLY_TOPIC_STRIPPER].msg;
        out_reply_topic->length = strippers[_REPLY_TOPIC_STRIPPER].len;

        if (is_channel_request) {
            sfsc_ChannelFactoryRequest channel_request =
                sfsc_ChannelFactoryRequest_init_default;
            state.extra = NULL;
            state.msg = strippers[_PAYLOAD_STRIPPER].msg;
            state.msg_left = strippers[_PAYLOAD_STRIPPER].len;
            channel_request.payload.funcs.decode = b_strip_callback;
            channel_request.payload.arg = strippers + _PAYLOAD_STRIPPER;
            pb_istream_t substream = GENERIC_ISTREAM(&state);
            if (!pb_decode(&substream, sfsc_ChannelFactoryRequest_fields,
                           &channel_request)) {
                return 0;
            }
        }

        *out_paylaod = strippers[_PAYLOAD_STRIPPER].msg;
        *out_payload_len = strippers[_PAYLOAD_STRIPPER].len;
        return 1;
    } else {
        return 0;
    }
}

sfsc_bool generic_decoding_callback(pb_istream_t* stream, uint8_t* buf,
                                    sfsc_size count) {
    generic_decode_state* state = (generic_decode_state*)stream->state;
    if (buf != NULL) {
        sfsc_copy(buf, state->msg, count);
    }
    state->msg += count;
    state->msg_left -= count;
    return 1;
}

sfsc_int8 write_composite_message(zmtp_socket* socket,
                                  sfsc_composite_buffer* buf,
                                  sfsc_uint8 as_last_message) {
    sfsc_uint16 total_length = total_composite_length(buf);
    sfsc_int8 op_result =
        begin_message_multipart(socket, total_length, as_last_message);
    if (op_result == ZMTP_OK) {
        for (sfsc_size i = 0; i < buf->count - 1; i++) {
            op_result =
                continue_message_multipart(socket, (buf->buffers + i)->content,
                                           (buf->buffers + i)->length, 0);
            if (op_result != ZMTP_OK) {
                return op_result;
            }
        }
        return continue_message_multipart(
            socket, (buf->buffers + buf->count - 1)->content,
            (buf->buffers + buf->count - 1)->length, 1);
    } else {
        return op_result;
    }
}

sfsc_int8 write_composite_subscription_message(zmtp_socket* socket,
                                               sfsc_composite_buffer* buf,
                                               sfsc_uint8 subscribe) {
    sfsc_uint16 total_length = total_composite_length(buf);
    sfsc_int8 op_result = begin_message_multipart(socket, total_length + 1, 1);
    if (op_result == ZMTP_OK) {
        const sfsc_uint8 sub[1] = {1};
        const sfsc_uint8 unsub[1] = {0};
        op_result =
            continue_message_multipart(socket, subscribe ? sub : unsub, 1, 0);
        if (op_result == ZMTP_OK) {
            for (sfsc_size i = 0; i < buf->count - 1; i++) {
                op_result = continue_message_multipart(
                    socket, (buf->buffers + i)->content,
                    (buf->buffers + i)->length, 0);
                if (op_result != ZMTP_OK) {
                    return op_result;
                }
            }
            return continue_message_multipart(
                socket, (buf->buffers + buf->count - 1)->content,
                (buf->buffers + buf->count - 1)->length, 1);
        } else {
            return op_result;
        }
    } else {
        return op_result;
    }
}

void sfsc_copy(sfsc_uint8* dst, const sfsc_uint8* src, sfsc_size len) {
    memcpy(dst, src, len);
}

void random_uuid_if_needed(sfsc_uint8 target[36]) {
    if (target[0] == 0) {
        random_uuid(target);
    }
}
