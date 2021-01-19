/**
 * @brief Demonstrates the usage of a publisher service and a subscriber.
 *
 * To run this example, define EXAMPLE_PUBSUB and configure
 * the host and CONTROL_PUB_PORT to match your core.
 *
 * The following steps are taken in this example:
 * 1. Create an adapter and get it operational
 * 2. Register a publisher
 * 3. Register a subscriber and subscribe the publisher with it
 * 4. While the publisher has at least one subscriber, publish messages every
 * PUBLISHING_INTERVAL_MS
 * 5. Print the messages the subscriber receives (only if ENABLE_PRINTS is
 * defined)
 */

#ifdef EXAMPLE_PUBSUB
#include "../../sfsc/sfsc_adapter/sfsc_adapter.h"
#include "../../sfsc/sfsc_adapter/sfsc_adapter_struct.h"
#ifdef ENABLE_PRINTS
#include "../shared/console.h"
#endif

#define CONTROL_PUB_PORT 11251
const char host[] = "192.168.178.92";

// Declare an adapter
static sfsc_adapter adapter = sfsc_adapter_DEFAULT_INIT;
// Declare a publisher
static sfsc_publisher publisher = sfsc_publisher_DEFAULT_INIT;
// Declare a subscriber
static sfsc_subscriber subscriber = sfsc_subscriber_DEFAULT_INIT;
// Other state variables
static bool first_time = 1;
#define PUBLISHING_INTERVAL_MS 1000
static sfsc_uint32 published_message_number = 0;
static sfsc_uint64 next_publish_time = 0;

static void on_subscription_change(sfsc_adapter *a, sfsc_publisher *p,
                                   sfsc_uint8 last_subscribers,
                                   sfsc_uint8 current_subscribers) {
#ifdef ENABLE_PRINTS
    console_print_char("Subscriber count changed from ");
    console_print_uint8(last_subscribers);
    console_print_char(" to ");
    console_println_uint8(current_subscribers);
#endif
    if (current_subscribers > 0) {
        next_publish_time =
            time_ms();  // Indicates that we want to publish messages
    } else {
        next_publish_time =
            0;  // Indicates that we don't want to publish messages
    }
}

static void on_subscriber_data(sfsc_adapter *a, sfsc_subscriber *s,
                               sfsc_buffer payload, sfsc_bool *b_auto_advance) {
    // We only subscribed one publisher, so we know that the payload is a
    // sfsc_uint32
    sfsc_uint32 message_number = *(const sfsc_uint32 *)payload.content;
#ifdef ENABLE_PRINTS
    console_print_char("Received message number ");
    console_println_uint32(message_number);
#endif
    *b_auto_advance =
        1;  // We don't need the payload outside of this callback, so there is
            // no need in entering the user task pause state
}

static void on_publisher_created(sfsc_adapter *a,
                                 sfsc_publisher_or_server service,
                                 sfsc_bool created) {
    // We could optain a pointer to our publisher using the service parameter,
    // but since we know that we only use one publisher, we access it directly.
    if (created) {
// We got here from the register_publisher call
#ifdef ENABLE_PRINTS
        console_println_char("Publisher created!");
#endif
        // Since the publisher is now part of the service registry, we can
        // create a subscriber and listen to it. Usually, we would query the
        // service registry, but we are using a shortcut here: We already know
        // the topic of our publisher, since we used SERVICE_TOPIC_AUTOGEN, the
        // publishers topic is equal to it's service id (as docummented in
        //sfsc_adapter.h).
        subscriber.topic.length = UUID_LEN;
        subscriber.topic.content = (const sfsc_uint8 *)publisher.service_id.id;
        // Next, we link our callback.
        subscriber.on_data = on_subscriber_data;
        // Finally, we register it with the adapter.
        // After this call, the publisher should detect that it has a
        // subscriber, so on_subscription_change is invoked.
        sfsc_int8 op_result = register_subscriber(&adapter, &subscriber);
        if (op_result == SFSC_OK) {
#ifdef ENABLE_PRINTS
            console_println_char("Registering subscriber successful!");
#endif
        } else {
#ifdef ENABLE_PRINTS
            console_print_char("Registering subscriber failed: ");
            console_println_int8(op_result);
#endif
        }
    }
}

static sfsc_int8 register_publisher_service() {
#ifdef ENABLE_PRINTS
    console_println_char("Attempting to create a publisher...");
#endif
    // Before registering the publisher, we should fill it.
    // We want the topic to be automatically generated, so normally we would use
    // the SERVICE_TOPIC_AUTOGEN macro. But the macro can only be used during
    // struct declaration, so we need to set the already existing topic struct of
    // our publisher manually to the values from the macro.
    publisher.topic.content = NULL;
    publisher.topic.length = 0;
    // We want to get notified if there is a new subscriber.
    publisher.on_subscription_change = on_subscription_change;
    // After filling the publisher, we register it.
    // We use the normal (not the unregistered) version of the
    // register_publisher function, since we want the publisher to show up in the
    // service registry. We also give the publisher a name, but ignore the other
    // fields for now. Lastly, we use a callback to get notified once our
    // publisher was written into the service registry.
    const char publisher_name[] = "Hello_World_Publisher";
    sfsc_buffer name = {(const sfsc_uint8 *)publisher_name,
                        strlen(publisher_name)};
    sfsc_buffer empty = sfsc_buffer_DEFAULT_INIT;
    return register_publisher(&adapter, &publisher, name, empty, empty,
                              on_publisher_created);
}

static sfsc_int8 publish_message_if_necessary() {
    sfsc_uint64 now = time_ms();
    if (next_publish_time < now) {
        sfsc_buffer message = {(const sfsc_uint8 *)&published_message_number,
                               4};  // We use the message number as payload
        sfsc_int8 op_result = publish(&adapter, &publisher, message);
        if (op_result == SFSC_OK) {
#ifdef ENABLE_PRINTS
            console_print_char("Published message number ");
            console_println_uint32(published_message_number);
#endif
            next_publish_time = now + PUBLISHING_INTERVAL_MS;
            published_message_number++;
            return SFSC_OK;
        } else {
            return op_result;
        }
    } else {
        return SFSC_OK;
    }
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
                // Since the adapter is now operational, lets register a
                // publisher.
                op_result = register_publisher_service();
                if (op_result != SFSC_OK) {
#ifdef ENABLE_PRINTS
                    console_print_char("Registering publisher failed: ");
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
            // We use this to determinate if we should publish a message.
            // This value is filled by the on_subscription_change callback
            // of the publisher.
            if (next_publish_time > 0) {
                op_result = publish_message_if_necessary();
                if (op_result != SFSC_OK) {
#ifdef ENABLE_PRINTS
                    console_print_char("Error publishing: ");
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