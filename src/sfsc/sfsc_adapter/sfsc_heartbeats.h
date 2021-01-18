/*
 * sfsc_heartbeats.h
 *
 *  Created on: 27.10.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_ADAPTER_SFSC_HEARTBEATS_H_
#define SRC_SFSC_ADAPTER_SFSC_HEARTBEATS_H_

#include "sfsc_adapter.h"
#include "sfsc_adapter_data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sfsc_heartbeats {
	sfsc_uint8 heartbeat_msg[sfsc_HeartbeatMessage_size];
	sfsc_uint64 send_time;
	sfsc_uint64 receive_time;
} _sfsc_heartbeats;

#define _sfsc_heartbeats_DEFAULT_INIT {{0},0,0}


sfsc_int8 handle_outgoing_heartbeats(_sfsc_adapter_data* adapter,_sfsc_heartbeats* heartbeats);
sfsc_int8 handle_incoming_heartbeats(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,_sfsc_heartbeats* heartbeats);
sfsc_int8 prepare_heartbeat_message(sfsc_adapter_stats* stats,_sfsc_heartbeats* heartbeats);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_HEARTBEATS_H_ */
