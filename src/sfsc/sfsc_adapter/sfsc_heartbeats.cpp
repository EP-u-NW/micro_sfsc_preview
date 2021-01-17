/*
 * sfsc_heartbeats.c
 *
 *  Created on: 27.10.2020
 *      Author: Eric
 */
#include "sfsc_utils.h"
#include "sfsc_constants.h"
#include "sfsc_heartbeats.h"
#include "../proto/pb.h"
#include "../proto/pb_encode.h"
#include "../platform/sfsc_platform.h"

sfsc_int8 prepare_heartbeat_message(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,_sfsc_heartbeats* heartbeats) {
	pb_ostream_t output = pb_ostream_from_buffer(heartbeats->heartbeat_msg,
	sfsc_HeartbeatMessage_size);
	sfsc_HeartbeatMessage msg = sfsc_HeartbeatMessage_init_default;
	sfsc_copy((sfsc_uint8*)msg.adapter_id.id, stats->adapter_id, UUID_LEN);
	if (pb_encode(&output, sfsc_HeartbeatMessage_fields, &msg)) {
		return ZMTP_OK;
	} else {
		return E_PROTO_ENCODE;
	}
}
sfsc_int8 handle_incoming_heartbeats(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,_sfsc_heartbeats* heartbeats) {

	sfsc_buffer adapter_heartbeat_topic_parts[2] = { { _heartbeat_topic,
	_HEARTBEAT_TOPIC_LEN }, { stats->adapter_id, UUID_LEN } };
	sfsc_composite_buffer adapter_heartbeat_topic = {
			adapter_heartbeat_topic_parts, 2 };

	sfsc_uint64 now = time_ms();

	sfsc_uint8* message = get_message(&(adapter->control_sub));
	if (message != NULL) {
		if (stats->state == SFSC_STATE_OPERATIONAL) {
			if (adapter->control_sub.is_message == 1
					&& adapter->control_sub.last_message == 0
					&& b_composite_bytes_equal(&adapter_heartbeat_topic, message,
							adapter->control_sub.buffer_index)) {
				zmtp_pop(&(adapter->control_sub));
				stats->state = SFSC_STATE_EXPECT_HEARTBEAT;
			}
		} else if (adapter->control_sub.is_message
				== 1&& adapter->control_sub.last_message == 1
				&& stats->state == SFSC_STATE_EXPECT_HEARTBEAT) {
			zmtp_pop(&(adapter->control_sub));
			//We dont need to decode the heartbeat message to see if it is actually for
			//us since we encoded our adapter id in the topic
			stats->state = SFSC_STATE_OPERATIONAL;
			heartbeats->receive_time = now;
		}
	}
	if (now
			< heartbeats->receive_time
					+ HEARTBEAT_DEADLINE_INCOMING_MS
			|| heartbeats->receive_time == 0) {
		return ZMTP_OK;
	} else {
		return E_HEARTBEAT_MISSING;
	}
}

sfsc_int8 handle_outgoing_heartbeats(_sfsc_adapter_data* adapter, _sfsc_heartbeats* heartbeats) {
	sfsc_uint64 now = time_ms();
	sfsc_uint64 since_last_send = now - heartbeats->send_time;
	if (since_last_send < HEARTBEAT_DEADLINE_OUTGOING_MS
			|| heartbeats->send_time == 0) {

		if (since_last_send >= HEARTBEAT_SEND_RATE_MS
				|| heartbeats->send_time == 0) {
			write_message(&(adapter->control_pub), _heartbeat_topic,
			_HEARTBEAT_TOPIC_LEN, 0); //Seems as the core wants its heartbeats on the default topic rather the one from the hello topic
			write_message(&(adapter->control_pub), heartbeats->heartbeat_msg,
			sfsc_HeartbeatMessage_size, 1);
			heartbeats->send_time = now;
		}
		return ZMTP_OK;
	} else {
		return E_TOO_SLOW;
	}
}
