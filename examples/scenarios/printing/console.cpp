/*
 * console.cpp
 *
 *  Created on: 31.10.2020
 *      Author: Eric
 */
#include "console.h"

void print_byte_array(const sfsc_uint8 array[], sfsc_int32 size) {
	console_print_char("{");
	for (sfsc_int32 i = 0; i < size - 1; i++) {
		console_print_uint8(array[i]);
		console_print_char(", ");
	}
	console_print_uint8(array[size - 1]);
	console_println_char("}");
}

void print_relative_service_descriptor(
		relative_sfsc_service_descriptor* descriptor, sfsc_uint8* offset,
		sfsc_size len) {
	console_print_char("Raw descriptor buffer (");
	console_print_uint16(len);
	console_print_char("): ");
	print_byte_array(offset, len);
	console_print_char("Service name: ");
	console_write(offset + descriptor->name_offset, descriptor->name_len);
	console_println();
	console_print_char("Service Id: ");
	console_println_char(descriptor->service_id.id);
	console_print_char("Adapter Id: ");
	console_println_char(descriptor->adapter_id.id);
	console_print_char("Core Id: ");
	console_println_char(descriptor->core_id.id);
	console_print_char("Custom tag length:");
	console_println_uint16(descriptor->custom_tags_len);
	console_print_char("Service type: ");
	if (descriptor->service_type == SERVICE_TYPE_PUBLISHER) {
		console_println_char("Publisher");
		console_print_char("\tOutput topic: ");
		console_write(
				offset + descriptor->service_tags.publisher_tags.topic_offset,
				descriptor->service_tags.publisher_tags.topic_len);
		console_println();
		console_print_char("\tMessage type: ");
		console_write(
				offset
						+ descriptor->service_tags.publisher_tags.output_message_type_offset,
				descriptor->service_tags.publisher_tags.output_message_type_len);
		console_println();
	} else {
		console_println_char("Server");
		console_print_char("\tInput topic: ");
		console_write(
				offset + descriptor->service_tags.server_tags.topic_offset,
				descriptor->service_tags.server_tags.topic_len);
		console_println();
		console_print_char("\tInput Message type: ");
		console_write(
				offset
						+ descriptor->service_tags.server_tags.input_message_type_offset,
				descriptor->service_tags.server_tags.input_message_type_len);
		console_println();
		console_print_char("\tOutput Message type: ");
		console_write(
				offset
						+ descriptor->service_tags.publisher_tags.output_message_type_offset,
				descriptor->service_tags.publisher_tags.output_message_type_len);
		console_println();
		console_print_char("\tAck max send tries: ");
		console_println_int32(
				descriptor->service_tags.server_tags.ack_settings.send_max_tries);
		console_print_char("\tAck timeout: ");
		console_println_int32(
				descriptor->service_tags.server_tags.ack_settings.timeout_ms);
	}
}
