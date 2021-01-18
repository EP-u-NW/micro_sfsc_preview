/*
 * sfsc_adapter_struct.h
 *
 *  Created on: 01.11.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_ADAPTER_SFSC_ADAPTER_STRUCT_H_
#define SRC_SFSC_ADAPTER_SFSC_ADAPTER_STRUCT_H_

#include "sfsc_adapter.h"
#include "sfsc_adapter_data.h"
#include "sfsc_command.h"
#include "sfsc_connect.h"
#include "sfsc_heartbeats.h"
#include "sfsc_publisher.h"
#include "sfsc_query.h"
#include "sfsc_request.h"
#include "sfsc_server.h"
#include "sfsc_subscriber.h"

#ifdef __cplusplus
extern "C" {
#endif
#define sfsc_adapter_DEFAULT_INIT {sfsc_adapter_stats_DEFAULT_INIT,_sfsc_adapter_data_DEFAULT_INIT,{NULL},{NULL},{NULL} ,_sfsc_heartbeats_DEFAULT_INIT,_sfsc_queries_DEFAULT_INIT,_sfsc_commands_DEFAULT_INIT,_sfsc_requests_DEFAULT_INIT,_sfsc_acks_DEFAULT_INIT}
struct _sfsc_adapter {
	sfsc_adapter_stats stats;
	_sfsc_adapter_data data;
	sfsc_subscriber *subscribers[MAX_SUBSCRIBERS];
	sfsc_publisher *publishers[MAX_PUBLISHERS];
	sfsc_server *servers[MAX_SERVERS];
	_sfsc_heartbeats heartbeat_info;
	_sfsc_queries queries;
	_sfsc_commands commands;
	_sfsc_requests requests;
	_sfsc_acks pending_acks;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_ADAPTER_STRUCT_H_ */
