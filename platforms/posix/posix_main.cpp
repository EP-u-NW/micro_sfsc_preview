
#ifdef POSIX
#ifndef EVALUATION
#include "posix_config.h"
#include "../sfsc/sfsc_adapter/sfsc_adapter.h"
#include "../sfsc/platform/sfsc_platform.h"
#include "../debug/console.h"

const sfsc_uint8 ssid[] = "The Promised Lan2.4";
const sfsc_uint8 password[] = "ericistsuper";
const char epnw[] = "81.169.207.220";
const char stauli[] = "192.168.175.73";
const char local[] = "192.168.178.171";
const char pc[] = "192.168.178.92";
const char* address = epnw;
const sfsc_uint16 bootstrap_port = 1251;

#include "../sfsc/sfsc_adapter/sfsc_adapter.h"
#include "../sfsc/sfsc_adapter/sfsc_adapter_struct.h"
#include "../examples/pub_sub.h"
#include "../examples/req_rep.h"
#include "../examples/channel.h"

sfsc_adapter adapter;
sfsc_uint8 operating = 0;
sfsc_uint8 start_up = 0;
sfsc_uint8 last_state = SFSC_STATE_NONE;
sfsc_uint64 operation_start_time;
static void on_descriptor(sfsc_adapter* adapter,
		relative_sfsc_service_descriptor descriptor, sfsc_uint8* start,
		sfsc_size length, sfsc_uint8 is_last) {
	if (start == NULL) {
		console_println_char("NULL descriptor!");
	} else {
		print_relative_service_descriptor(&descriptor, start, length);
		example_on_descriptor_publisher(descriptor, start, length, is_last);
		example_on_descriptor_server(descriptor, start, length, is_last);
		example_on_descriptor_channel(descriptor, start, length, is_last);
	}
	sfsc_uint8 needs_query = example_needs_query_publisher()
			|| example_needs_query_server() || example_needs_query_channel();
	query_services_next(adapter, needs_query);
	if (is_last) {
		console_println_char("Last!");
		if (needs_query) {
			console_println_char("Service not found!");
			console_println_char("Trying again...");
			query_services(adapter, on_descriptor);
		}
	}
}

void service_created_callback(sfsc_adapter* adapter,
		sfsc_publisher_or_server service,
		sfsc_uint8 created) {
	if (service.is_server) {
		example_server_command_result(service.service.server, created);
		example_channel_command_result(service.service.server, created);
	} else {
		example_publisher_command_result(service.service.publisher, created);
	}
	if (!adapter_stats(adapter)->query_in_progress) {
		query_services(adapter, on_descriptor);
	}
}

static void main_system_task() {
	sfsc_int8 op_result = system_task(&adapter);
	if (op_result == SFSC_OK) {
		if (last_state
				!= adapter_stats(&adapter)->state&& adapter_stats(&adapter)->state != SFSC_STATE_EXPECT_HEARTBEAT) {
			last_state = adapter_stats(&adapter)->state;
			console_print_char("New state: ");
			console_println_uint8(adapter_stats(&adapter)->state);
		}
		if (adapter_stats(&adapter)->state == SFSC_STATE_OPERATIONAL
				&& start_up == 0) {
			start_up = 1;
			console_print_char("My id: ");
			console_write(adapter_stats(&adapter)->adapter_id, 36);
			console_println();
			console_print_char("Connected to core ");
			console_write(adapter_stats(&adapter)->core_id, 36);
			console_println();
			operation_start_time = time_ms();
			example_startup_publisher(&adapter, service_created_callback);
			example_startup_server(&adapter, service_created_callback);
			example_startup_channel(&adapter, service_created_callback);
		}
	} else if (op_result == W_MESSAGE_DROP) {
		console_println_char("Message drop!");
	} else {
		console_print_char("Error: ");
		console_println_int8(op_result);
		console_print_char("Total operational time: ");
		if (operation_start_time > 0) {
			console_println_uint32((sfsc_uint32) (time_ms() - operation_start_time));
		}
		operating = 0;
	}
}

static void main_user_task() {
	sfsc_uint64 start = time_ms();
	sfsc_int8 op_result = user_task(&adapter);
	sfsc_uint64 user_task_duration = time_ms() - start;
	if (user_task_duration > 200) {
		console_print_char("WARNING: user loop took ");
		console_print_uint32((sfsc_uint32) user_task_duration);
		console_println_char("ms");
	}
	if (op_result != ZMTP_OK) {
		console_print_char("User loop error ");
		console_println_int8(op_result);
		operating = 0;
	}
}

void test_task() {
	if (operating) {
		main_system_task();
	}
	if (operating) {
		example_task_publisher();
		example_task_server();
		example_task_channel();
	}
	if (operating) {
		main_system_task();
	}
	if (operating) {
		main_user_task();
	}
}

void test_setup() {
	console_print_char("Adapter size: ");
	console_println_uint32((sfsc_uint32)sizeof(sfsc_adapter));
	start_session_bootstraped(&adapter, address, bootstrap_port);
	operating = 1;
}



int main(){
    test_setup();
    while(1){
        test_task();
    }
}

#endif
#endif