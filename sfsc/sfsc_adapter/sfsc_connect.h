/*
 * sfsc_connect.h
 *
 *  Created on: 28.10.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_ADAPTER_SFSC_CONNECT_H_
#define SRC_SFSC_ADAPTER_SFSC_CONNECT_H_

#include "sfsc_adapter.h"
#include "sfsc_adapter_data.h"
#include "sfsc_heartbeats.h"

#ifdef __cplusplus
extern "C" {
#endif

sfsc_int8 sfsc_internal_start_session_bootstraped(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,
		const char *address, int original_control_pub_port);

sfsc_int8 sfsc_internal_start_session(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,_sfsc_heartbeats* heartbeats, const char *address,
		int original_control_pub_port, int original_control_sub_port,
		int original_data_pub_port, int original_data_sub_port);

sfsc_int8 system_task_connect(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,_sfsc_heartbeats* heartbeats);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_CONNECT_H_ */
