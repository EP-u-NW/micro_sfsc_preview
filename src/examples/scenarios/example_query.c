/**
 * @brief Demonstrates how to query the service registry.
 *
 * To run this example, define EXAMPLE_QUERY and configure
 * the host and CONTROL_PUB_PORT to match your core.
 *
 * The following steps are taken in this example:
 * 1. Create an adapter and get it operational
 * 2. Register six publisher services
 * 3. After they are registered, pick a random number from 0 to 8 (exclusive)
 * 4. Try to find the corresponding publisher, if a number > 5 was picked
 * this should fail and reach the end of the registry
 * 5. If the publisher was found, copy it somewhere global (for demonstration
 * purposes)
 * 6. After the publisher was copied or the end of the registry was reached
 * wait 2 seconds and try again (goto point 3)
 *
 * Some interesting/important remarks on running this example multiple times
 * with the same core:
 * 1. The service registry is stored as a event log on a core, and every time
 * you run this example without restarting the core, the event log gets longer.
 * So as a consequence, it will take longer to reach the end of the log when we
 * query it (this wont be noticable all of the time, since we dont always reach
 * the end of the log because we stop querying after we found our target, this
 * "taking longer" will only occure when the service we are looking for is not
 * actually in the log, like in cases G and H (this will be explained in the
 * code)).
 * 2. The second thing to notice is, that this example won't unregister the
 * services. As long as the core does not identify the adapter of a previous run
 * of this example as timed out, there will be multiple valid (meaning not
 * deleted) services with the same name in the registry. Despite this, this
 * example will always find the most recent version of the service, since it
 * starts querying the event log starting at the most recent event.
 */

#ifdef EXAMPLE_QUERY
#include "../../sfsc/sfsc_adapter/sfsc_adapter.h"
#include "../../sfsc/sfsc_adapter/sfsc_adapter_struct.h"
#ifdef ENABLE_PRINTS
#include "../shared/console.h"
#endif

#define CONTROL_PUB_PORT 11251
const char host[] = "81.169.207.220";

// Declare an adapter
static sfsc_adapter adapter = sfsc_adapter_DEFAULT_INIT;
// Declare a space for the publishers
#define PUBLISHER_COUNT 6
static sfsc_publisher publishers[PUBLISHER_COUNT] = {
    sfsc_publisher_DEFAULT_INIT};
// We declare this varaibles globaly since we use it in different places
// but we don't declare them const, since we will change them according to
// the situation
#define PUBLISHER_NAME_LEN 23
static char publisher_name_space[] = "Hello_World_Publisher_X";
static sfsc_buffer publisher_name_as_buffer = {
    (const sfsc_uint8 *)publisher_name_space, PUBLISHER_NAME_LEN};
// Declare space to store the found service
static relative_sfsc_service_descriptor found_service_descriptor;
#define MAX_FOUND_SERVICE_SIZE 128
static sfsc_uint8 found_service_buffer[MAX_FOUND_SERVICE_SIZE];
static sfsc_size found_service_actuall_size;
// Other state variables
static bool first_time = 1;
static sfsc_uint8 registered_count = 0;
static sfsc_uint64 next_query_time = 0;
#define QUERY_INTERVALL_MS 2000
static sfsc_size delivered_services = 0;
#define ASCII_A 65

static void on_publisher_created(sfsc_adapter *a,
                                 sfsc_publisher_or_server service,
                                 sfsc_bool created) {
    // Wait until all publishers created
    if (created) {
        registered_count++;
#ifdef ENABLE_PRINTS
        console_print_uint8(registered_count);
        console_print_char(" of ");
        console_print_uint8(PUBLISHER_COUNT);
        console_println_char(" publishers registered...");
#endif
        if (registered_count == PUBLISHER_COUNT) {
#ifdef ENABLE_PRINTS
            console_println_char("All publishers registered!");
#endif
            next_query_time = time_ms();
        }
    }
}

static sfsc_int8 register_publisher_services() {
#ifdef ENABLE_PRINTS
    console_print_char("Attempting to create ");
    console_print_uint8(PUBLISHER_COUNT);
    console_println_char(" publishers...");
#endif
    // Creating publishers is discussed in deepth in the pubsub
    // example, so study this first to understand what we are doing here.
    // Lets register PUBLISHER_COUNT publisher servers simultaneously,
    // and name them Hello_World_Publisher_X where X is a letter from A to F.
    sfsc_buffer empty = sfsc_buffer_DEFAULT_INIT;
    for (sfsc_size i = 0; i < PUBLISHER_COUNT; i++) {
        publisher_name_space[PUBLISHER_NAME_LEN - 1] = (char)(i + ASCII_A);
        publishers[i].topic.content = NULL;
        publishers[i].topic.length = 0;
        sfsc_int8 op_result = register_publisher(
            &adapter, publishers + i, publisher_name_as_buffer, empty, empty,
            on_publisher_created);
        if (op_result != SFSC_OK) {
            return op_result;
        }
    }
#ifdef ENABLE_PRINTS
    console_println_char("Issued all create commands");
#endif
    return SFSC_OK;
}

static bool is_target_service(relative_sfsc_service_descriptor *descriptor,
                              sfsc_uint8 *offset) {
    // First, check if the lengths of the names are the same
    if (descriptor->name_len == PUBLISHER_NAME_LEN) {
        // Now we compare the actuall names.
        // To get the name of the service we found, we add the
        // name_offset from the descriptor to the offset
        char *found_name_pointer = (char *)(offset + descriptor->name_offset);
        for (sfsc_size i = 0; i < PUBLISHER_NAME_LEN; i++) {
            if (publisher_name_space[i] != *(found_name_pointer + i)) {
                return 0;
            }
        }
        return 1;
    } else {
        return 0;
    }
}

static void on_service(sfsc_adapter *a,
                       relative_sfsc_service_descriptor descriptor,
                       sfsc_uint8 *offset, sfsc_size length,
                       sfsc_bool is_last) {
    sfsc_bool found_service = false;
    // If the following check is true, we got a service,
    // if it is false we are most likely at the end of the log.
    // Keep in mind that even if is_last is true there still might
    // be a service in the call, so we need the following check.
    if (offset != NULL) {
        delivered_services++;
        // A service was found, lets check if it's the service we are looking
        // for. We are doing this based on the name, if you run this example
        // multiple times make sure to read the reamarks of this example above.
        if (is_target_service(&descriptor, offset)) {
            // We found the target service
            found_service = true;
            // Once we call query_services_next the parameters in this callback
            // will get invalid, so lets copy them somewhere global (for
            // demonstration purposes).
            if (length <= MAX_FOUND_SERVICE_SIZE) {
                // We have enough space in our preallocated buffer.
                // Frist, copy the discriptor.
                found_service_descriptor = descriptor;
                // And the length of the data.
                found_service_actuall_size = length;
                memcpy(found_service_buffer, offset,
                       found_service_actuall_size);
                // We can now access this service from outside the callback,
                // e.g. to subscribe to it.
            } else {
#ifdef ENABLE_PRINTS
                console_print_char("Can not store the found service of  ");
                console_print_uint32((sfsc_uint32)length);
                console_print_char(
                    " bytes size in the preallocated buffer of ");
                console_print_uint32(MAX_FOUND_SERVICE_SIZE);
                console_println_char(" bytes size");
#endif
            }
        }
    }
    // If this is either the end of the log, or if we found the service, we stop
    // the query process, otherwise we continue. We are doing this with some
    // if clauses here because of logging, usually you could simply use
    // next=!(is_last||found_service).
    sfsc_bool next = true;
    if (found_service) {
        next = false;
#ifdef ENABLE_PRINTS
        console_print_char("Found target service after ");
        console_print_uint32((sfsc_uint32)delivered_services);
        console_println_char(" services");
#endif
    }
    if (is_last) {
        next = false;
        if (!found_service) {
#ifdef ENABLE_PRINTS
            console_print_char("Reached the end of the event log after ");
            console_print_uint32((sfsc_uint32)delivered_services);
            console_println_char(" services, but did not find the target");
#endif
        }
    }
    if (!next) {
        // Prepare to start again
        next_query_time = time_ms() + QUERY_INTERVALL_MS;
    }
    query_services_next(&adapter, next);
}

static sfsc_int8 start_query_process() {
    next_query_time = 0;
    // We pick a random number from 0 to 8 (exclusive) and add 65 to get a
    // letter from A to H. To pick that number, we are using the random_bytes
    // function, but only take the lowest 3 bits
    sfsc_uint8 *random_letter =
        (sfsc_uint8 *)(publisher_name_space + PUBLISHER_NAME_LEN - 1);
    random_bytes(random_letter, 1);
    *random_letter = *random_letter & 0x7;
    *random_letter = *random_letter + ASCII_A;
#ifdef ENABLE_PRINTS
    console_print_char("Trying to find publisher ");
    console_println_char(publisher_name_space);
#endif
    // Reset this counter
    delivered_services = 0;
    // Now we start the query process, which will deliver each service that is
    // currently in the registry to the callback. Then we examin every found
    // service to see if it's the one we are interessted in. The 2 cases where
    // there is no publisher (G,H) should demonstrate what happens if there is no
    // matching service and we reach the end of the log.
    return query_services(&adapter, on_service);
}

static sfsc_bool example_step() {
    sfsc_int8 op_result = system_task(&adapter);
    if (op_result == SFSC_OK ||
        op_result ==
            W_MESSAGE_DROP)  // Message drop is just a warning, not an error
    {
        sfsc_adapter_stats *stats = adapter_stats(&adapter);
        if (stats->state >= SFSC_STATE_OPERATIONAL) {
            if (first_time) {
                first_time = 0;
#ifdef ENABLE_PRINTS
                console_println_char(
                    "SFSC Handshake done, adapter operational");
                console_print_char("Adapter ID: ");
                console_write(stats->adapter_id, UUID_LEN);
                console_println();
                console_print_char("Core ID: ");
                console_write(stats->core_id, UUID_LEN);
                console_println();
#endif
                // Since the adapter is now operational, start with registering
                // the publishers
                op_result = register_publisher_services();
                if (op_result != SFSC_OK) {
#ifdef ENABLE_PRINTS
                    console_print_char("Registering publishers failed: ");
                    console_println_int8(op_result);
#endif
                    return 0;
                }
            }
            op_result = user_task(&adapter);
            if (op_result != SFSC_OK) {
#ifdef ENABLE_PRINTS
                console_print_char("User task error: ");
                console_println_int8(op_result);
#endif
                return 0;
            }
            if (next_query_time != 0 && next_query_time < time_ms()) {
                op_result = start_query_process();
                if (op_result != SFSC_OK) {
#ifdef ENABLE_PRINTS
                    console_print_char("Query start error: ");
                    console_println_int8(op_result);
#endif
                    return 0;
                }
            }
        }
        return 1;
    } else {
#ifdef ENABLE_PRINTS
        console_print_char("System task error: ");
        console_println_int8(op_result);
#endif
        return 0;
    }
}

int example_main() {
    // Start a session
    sfsc_int8 op_result =
        start_session_bootstraped(&adapter, host, CONTROL_PUB_PORT);
    if (op_result == SFSC_OK) {
        while (1)  // global execution loop
        {
            if (!example_step()) {
                return -1;
            }
        }
    } else {
#ifdef ENABLE_PRINTS
        console_print_char("Failed to start bootstrapping: ");
        console_println_int8(op_result);
        return -1;
#endif
    }
}
#endif