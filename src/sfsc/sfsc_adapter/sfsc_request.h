/*
 * sfsc_request.h
 *
 *  Created on: 27.10.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_ADAPTER_SFSC_REQUEST_H_
#define SRC_SFSC_ADAPTER_SFSC_REQUEST_H_

#include "sfsc_adapter.h"
#include "sfsc_adapter_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _sfsc_channel_request_extra_DEFAULT_INIT {NULL,NULL,0,NULL}
typedef struct __sfsc_channel_request_extra {
	relative_sfsc_service_descriptor* descriptor;
	sfsc_uint8* target_buffer;
	sfsc_size taget_buffer_size;
	sfsc_channel_request_callback* callback;
} _sfsc_channel_request_extra;

#define _sfsc_request_DEFAULT_INIT {_REPLY_ID_UNUSED,NULL,0,{_sfsc_channel_request_extra_DEFAULT_INIT},0}
typedef struct __sfsc_request {
	sfsc_int32 reply_id;
	void* arg;
	sfsc_bool is_channel_request;
	union {
		_sfsc_channel_request_extra channel;
		sfsc_request_callback* normal_callback;
	} extra;
	sfsc_uint64 valid_until;
} _sfsc_request;

#define _sfsc_requests_DEFAULT_INIT {0,{_sfsc_request_DEFAULT_INIT}}
typedef struct __sfsc_requests {
	sfsc_int32 reply_id_counter;
	_sfsc_request callbacks[MAX_SIMULTANIOUS_REQUESTS];
} _sfsc_requests;

sfsc_int8 sfsc_internal_request(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,
		_sfsc_requests* requests, sfsc_buffer topic, sfsc_buffer payload,
		sfsc_uint64 timeout, sfsc_request_callback* callback, void* arg);
sfsc_int8 sfsc_internal_channel_request(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,
		_sfsc_requests* requests, sfsc_buffer topic, sfsc_buffer payload,
		sfsc_uint64 timeout,sfsc_uint8* descriptor_space,sfsc_size descriptor_space_lenght, relative_sfsc_service_descriptor* descriptor,sfsc_channel_request_callback* callback,
		void* arg);

void user_task_request_timeout(sfsc_adapter* forward_pointer,
		_sfsc_requests* requests);
sfsc_int8 system_task_data_sub_receive_reply(_sfsc_adapter_data* adapter,
		_sfsc_requests* requests,
		sfsc_uint8* message);
sfsc_int8 user_task_data_handle_reply(sfsc_adapter* forward_pointer,
		_sfsc_requests* requests, sfsc_uint8* next_payload,
		sfsc_size payload_len,sfsc_bool* b_auto_advance);
		
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_REQUEST_H_ */
