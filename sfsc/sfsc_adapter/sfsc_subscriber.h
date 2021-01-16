/*
 * sfsc_subscriber.h
 *
 *  Created on: 27.10.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_ADAPTER_SFSC_SUBSCRIBER_H_
#define SRC_SFSC_ADAPTER_SFSC_SUBSCRIBER_H_

#include "sfsc_adapter.h"
#include "sfsc_adapter_data.h"

#ifdef __cplusplus
extern "C" {
#endif

sfsc_int8 sfsc_internal_register_subscriber(_sfsc_adapter_data* adapter,
		sfsc_subscriber* subscribers[], sfsc_subscriber* subscriber);

sfsc_int8 sfsc_internal_unregister_subscriber(_sfsc_adapter_data* adapter, sfsc_subscriber* subscribers[],
		sfsc_subscriber* subscriber);

sfsc_int8 user_task_data_check_subscribers(sfsc_adapter* forward_pointer,sfsc_subscriber* subscribers[],
		sfsc_uint8* next_topic,
		sfsc_size topic_len, sfsc_uint8* next_payload,
		sfsc_size payload_len, sfsc_uint8* consumed,sfsc_bool* b_auto_advance);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_SUBSCRIBER_H_ */
