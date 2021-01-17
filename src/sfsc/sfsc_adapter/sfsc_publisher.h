/*
 * sfsc_publisher.h
 *
 *  Created on: 28.10.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_ADAPTER_SFSC_PUBLISHER_H_
#define SRC_SFSC_ADAPTER_SFSC_PUBLISHER_H_

#include "sfsc_adapter.h"
#include "sfsc_adapter_data.h"
#include "sfsc_command.h"

#ifdef __cplusplus
extern "C" {
#endif

sfsc_int8 user_task_publisher(sfsc_adapter* forward_pointer,sfsc_publisher* publishers[]);
sfsc_int8 system_task_publisher(_sfsc_adapter_data* adapter,sfsc_publisher* publishers[]);

sfsc_int8 sfsc_internal_register_publisher(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,
		_sfsc_commands* commands, sfsc_publisher* publishers[],
		sfsc_publisher* publisher, sfsc_buffer name, sfsc_buffer custom_tags,
		sfsc_buffer output_message_type, sfsc_command_callback* callback);
sfsc_int8 sfsc_internal_register_publisher_unregistered(sfsc_publisher* publishers[], sfsc_publisher* publisher,sfsc_size* i);
sfsc_int8 sfsc_internal_unregister_publisher(sfsc_adapter_stats* stats,_sfsc_adapter_data* adapter,
		_sfsc_commands* commands, sfsc_publisher* publishers[],
		sfsc_publisher* publisher, sfsc_command_callback* callback);
sfsc_int8 sfsc_internal_publish(_sfsc_adapter_data* adapter, sfsc_publisher* publisher,
		sfsc_buffer payload);

void configure_descriptor_for_channel_answer(
		sfsc_SfscServiceDescriptor* descriptor,
		sfsc_channel_answer* channel_answer, sfsc_buffer* topic_buffer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_PUBLISHER_H_ */
