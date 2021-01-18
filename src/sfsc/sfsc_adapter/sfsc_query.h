/*
 * sfsc_registry.h
 *
 *  Created on: 03.10.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_REGISTRY_H_
#define SRC_SFSC_REGISTRY_H_

#include "sfsc_adapter.h"
#include "sfsc_adapter_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _sfsc_queries_DEFAULT_INIT {relative_sfsc_service_descriptor_DEFAULT_INIT,{0},0,{sfsc_SfscId_init_default},NULL,0,0,0}
typedef struct __sfsc_queries {
	relative_sfsc_service_descriptor latest;
	sfsc_uint8 buffer[REGISTRY_BUFFER_SIZE];
	sfsc_size buffer_index;
	sfsc_SfscId deleted_services[MAX_DELETED_MEMORY];
	void* callback;
	sfsc_bool is_last;
	sfsc_bool has_data;
	sfsc_int64 expected_event_id;
} _sfsc_queries;

sfsc_int8 sfsc_internal_query_services(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter, _sfsc_queries* registry,
		sfsc_query_callback* callback);
void sfsc_internal_query_services_next(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter, _sfsc_queries* registry,
sfsc_bool next);
sfsc_int8 system_task_query(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,
		_sfsc_queries* registry);
sfsc_int8 user_task_query(sfsc_adapter* forward_pointer,_sfsc_queries* registry);

sfsc_int8 decode_channel_result(sfsc_uint8* msg, sfsc_size msg_len,
		relative_sfsc_service_descriptor* descriptor, sfsc_uint8* write_target,
		sfsc_size write_target_len, sfsc_size* write_target_used);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_REGISTRY_H_ */
