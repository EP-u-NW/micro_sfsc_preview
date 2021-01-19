/**
 * @brief Demonstrates the usage of a server service and how to make a request.
 *
 * To run this example, define EXAMPLE_REQREPACK and configure
 * the host and CONTROL_PUB_PORT to match your core.
 *
 * The following steps are taken in this example:
 * 1. Create an adapter and get it operational
 * 2. Register a server
 * 3. Make a request to the server
 * 4. Receive the request on the server, start a asyncron calculation
 * 5. Send an answer from the server once the calculation is done
 * 6. Receive the answer on the requestor and print it (only if ENABLE_PRINTS is
 * defined)
 * 7. Receive the automatically sended ack message on the server and clean up to be ready for the next request
 * 8. Wait 2 seconds and make a new request to the server
 */

#ifdef EXAMPLE_REQREPACK
#include "../../sfsc/sfsc_adapter/sfsc_adapter.h"
#include "../../sfsc/sfsc_adapter/sfsc_adapter_struct.h"
#ifdef ENABLE_PRINTS
#include "../shared/console.h"
#endif

#define CONTROL_PUB_PORT 11251
const char host[] = "192.168.178.92";

// Declare an adapter
static sfsc_adapter adapter = sfsc_adapter_DEFAULT_INIT;
// Declare a server
static sfsc_server server = sfsc_server_DEFAULT_INIT;
// Other state variables
static bool first_time = 1;
static sfsc_uint64 next_request_time = 0;
#define WAIT_UNTIL_NEXT_REQUEST 2000
static sfsc_uint64 long_running_task_steps_needed = 0;
static sfsc_uint64 long_running_task_result = 0;
static sfsc_int32 global_expected_reply_id;
#define MAX_GLOBAL_REPLY_TOPIC_LENGTH 64
static sfsc_uint8 global_reply_topic[MAX_GLOBAL_REPLY_TOPIC_LENGTH];
static sfsc_size global_reply_topic_actual_length;
//This will make sense after reading long_running_task_step_nonblocking
static sfsc_buffer answer_payload_buffer = {(const sfsc_uint8 *)&long_running_task_result, 8};

static void on_ack(sfsc_adapter *a, sfsc_server *s, sfsc_bool timeout, void *mapping_arg)
{
#ifdef ENABLE_PRINTS
    if (timeout)
    {
        console_println_char("Timeout occurred when answering a request!");
    }
    else
    {
        console_println_char("Received ack in time!");
    }
#endif
    //Time to clean up to be ready for the next request
    global_reply_topic_actual_length = 0;
    long_running_task_result = 0;
    //Set the time when the next request should be send
    next_request_time = time_ms() + WAIT_UNTIL_NEXT_REQUEST;
}

/**
 * @brief Simulates starting long running, but non blocking task.
 * 
 * Our long running task will be computated in steps and answered
 * once finished, see long_running_task_step_nonblocking. As you can
 * see we are using the variable long_running_task_steps_needed as
 * global state memory, so naturally, there can only be one task
 * at a time, see the remarks in on_request_for_server if you
 * need to server multiple requests at a time.
 * 
 * @returns 1 if the task could be started, 0 if there is already another task running
 */
static bool long_running_task_start_nonblocking(sfsc_uint64 parameter)
{
    if (global_reply_topic_actual_length == 0) //if there is already a task running, this check will be false
    {
        long_running_task_steps_needed = parameter;
        return 1;
    }
    else
    {
        return 0;
    }
}

static void long_running_task_step_nonblocking()
{
    if (long_running_task_steps_needed != 0)
    {
        //See what we are doing here? It's time for Gauß!
        long_running_task_result += long_running_task_steps_needed;
        long_running_task_steps_needed--;
        if (long_running_task_steps_needed == 0)
        {
            //We have sucesfully computed our results over multiple steps.
            //Time to answer the requestor!
            //Answering in a proper manor can be a bit tricky, so be sure
            //to read the documentation of the answer_request function first.
            //Since we configured our server to be a non-fire-and-forget-server
            //we need to make sure that the payload we are going to answer is valid
            //until we receive the on_ack call. Luckly, this is quite easy in this example
            //since the long_running_task_result is already a global variable, we just
            //need to interprete it as a sfsc_uint8* to use it as a payload.
            //The second thing we need to make sure is, that since we are going to pass a
            //pointer to a sfsc_buffer (and not a sfsc_buffer itselfe, keyword: mutability),
            //the sfsc_buffer we point to is also valid till to on_ack call. So we can not
            //declare it here, but use a staticly declared buffer. Now, go ahead and take a look
            //at the global variable answer_payload_buffer, and hopefully, it will make sense now.
            //What we can do here, is to create a sfsc_buffer that holds the reply topic (since
            //the reply_topic parameter is not a pointer). We also have to make sure, that the
            //content of the reply_topic is valid until on_ack is called, which it will be, since
            //again, we are using refering to a global variable.
            sfsc_buffer reply_topic_buffer = {(const sfsc_uint8 *)global_reply_topic, global_reply_topic_actual_length};
            sfsc_int8 op_result = answer_request(&adapter, &server, global_expected_reply_id, reply_topic_buffer, &answer_payload_buffer, NULL, on_ack);
            if (op_result == SFSC_OK)
            {
#ifdef ENABLE_PRINTS
                console_println_char("Sended answer!");
#endif
            }
            else
            {
#ifdef ENABLE_PRINTS
                console_print_char("Error answering: ");
                console_println_int8(op_result);
#endif
            }
        }
    }
}

static void long_running_task_cancel()
{
    long_running_task_steps_needed = 0;
}

static void on_request_for_server(sfsc_adapter *a, sfsc_server *s,
                                  sfsc_buffer payload,
                                  sfsc_int32 expected_reply_id,
                                  sfsc_buffer reply_topic,
                                  sfsc_bool *b_auto_advance)
{
    //Since we only got one service active we know to whom this request was made.
    //We are now going to start a long running, non blocking task which will requrie
    //multiple steps to solve.
    sfsc_uint64 task_parameter = *(const sfsc_uint64 *)payload.content;
    //Note that this example is only capable of one simultanious task, since the
    //function we are invoking will use global memory to store state varaibles.
    //If you need to serve multiple requests at the same time, you need to come up
    //with a solution for your use-case, e.g. using an struct type for state memory and
    //declarding a global array of that struct type.
    if (long_running_task_start_nonblocking(task_parameter))
    {
        //Once the task is compelted, it will answer the request.
        //To answer the request we need to know the reply_topic and the expected_reply_id
        //but these two things are only valid in this callback (see documentation).
        //We can solve this problem 2 ways: 1. either copy both things somewhere global OR
        //2. pause the user task. Option 2 is discussed in the 'The Execution model' section
        //of the readme, so this example will demonstrate option 1.
        //Since we are not pausing the user task to this:
        *b_auto_advance = 1;
        //Next, copy the expected_reply_id somewhere global...
        global_expected_reply_id = expected_reply_id;
        //... and then reply_topic
        //We allocated a global buffer to copy the topic in.
        //But first we need to check if this buffer is big enough to
        //hold the actual topic.
        if (reply_topic.length <= MAX_GLOBAL_REPLY_TOPIC_LENGTH)
        {
            //It is, so lets copy the topic over
            global_reply_topic_actual_length = reply_topic.length;
            memcpy(global_reply_topic, reply_topic.content, global_reply_topic_actual_length);
        }
        else
        {
#ifdef ENABLE_PRINTS
            console_println_char("Reply topic to long to topic, increase MAX_GLOBAL_REPLY_TOPIC_LENGTH");
#endif
            long_running_task_cancel();
        }
    }
    else
    {
#ifdef ENABLE_PRINTS
        console_println_char("There is already a request running, can not serve a second one!");
#endif
    }
}

static void on_answer(sfsc_adapter *a, sfsc_buffer payload,
                      sfsc_bool timeout, void *mapping_arg,
                      sfsc_bool *b_auto_advance)
{
    if (!timeout)
    {
        //The request got answered
        //Extract the answer
        sfsc_uint64 result_number = *(const sfsc_uint64 *)payload.content;
#ifdef ENABLE_PRINTS
        console_print_char("Received an answer with payload ");
        console_println_uint64(result_number);
#endif
    }
    else
    {
//We didn't get an answer in time
#ifdef ENABLE_PRINTS
        console_println_char("Request timed out...");
#endif
    }
    *b_auto_advance = 1;
}

static sfsc_int8 send_a_request()
{
    //We are doing a request now, so reset this
    next_request_time = 0;
    // Since the server is part of the service registry, we can
    // create issue a request to it. Usually, we would query the
    // service registry, but we are using a shortcut here: We already know
    // the topic of our server, since we used SERVICE_TOPIC_AUTOGEN, the
    // servers topic is equal to it's service id (as docummented in
    // sfsc_adapter.h).
    sfsc_buffer request_topic = {(const sfsc_uint8 *)server.service_id.id, UUID_LEN};
    //Generate a random payload
    sfsc_uint16 random_number;
    random_bytes((sfsc_uint8 *)&random_number, 2);
#ifdef ENABLE_PRINTS
    console_print_char("Sending a request with payload ");
    console_println_uint16(random_number);
#endif
    sfsc_uint64 payload_number = (sfsc_uint64)random_number;
    sfsc_buffer payload_buffer = {(const sfsc_uint8 *)&payload_number, 8};
    //We want to receive an answer within 30 seconds (30000ms)
    return request(&adapter, request_topic, payload_buffer, 30000, on_answer, NULL);
}

static void on_server_created(sfsc_adapter *a, sfsc_publisher_or_server service,
                              sfsc_bool created)
{
    if (created)
    {
        // We got here from the register_server call
#ifdef ENABLE_PRINTS
        console_println_char("Server created!");
#endif
        //We are not sending the request from here, but rather set a flag
        //so that the request is made from the global execution loop
        next_request_time = time_ms();
    }
}

static sfsc_int8 register_server_service()
{
#ifdef ENABLE_PRINTS
    console_println_char("Attempting to create a server...");
#endif
    // Before registering the server, we should fill it.
    // We want the topic to be automatically generated, so normally we would use
    // the SERVICE_TOPIC_AUTOGEN macro. But the macro can only be used during
    // struct declaration, so we need to set the already existing topic struct
    // of our publisher manually to the values from the macro.
    server.topic.content = NULL;
    server.topic.length = 0;
    // We now specify the function to be called on a request to this server
    server.on_request = on_request_for_server;
    // Also, we want this server to be a normal and not a channel server
    server.is_channel = 0;
    // Lastly, we need to specify the acknowledge stragegy.
    // We want that all answers we send are acknowledge by the requestor.
    // If we do not receive such an acknowledge after 3000ms (3 seconds),
    // we send our answer agian, up to 4 times, and if we don't recieve a
    // acknowledge by then, the answer will be treated as a failure
    server.ack_settings.timeout_ms = 3000;
    server.ack_settings.send_max_tries = 4;
    // After filling the server, we register it.
    // We also give the server a name, but ignore the
    // other fields for now. Lastly, we use a callback to get notified once our
    // publisher was written into the service registry.
    const char server_name[] = "Hello_World_Server";
    sfsc_buffer name = {(const sfsc_uint8 *)server_name, strlen(server_name)};
    sfsc_buffer empty = sfsc_buffer_DEFAULT_INIT;
    return register_server(&adapter, &server, name, empty, empty, empty,
                           on_server_created);
}

static sfsc_bool example_step()
{
    sfsc_int8 op_result = system_task(&adapter);
    if (op_result == SFSC_OK ||
        op_result ==
            W_MESSAGE_DROP) // Message drop is just a warning, not an error
    {
        sfsc_adapter_stats *stats = adapter_stats(&adapter);
        if (stats->state >= SFSC_STATE_OPERATIONAL)
        {
            if (first_time)
            {
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
                // server.
                op_result = register_server_service();
                if (op_result != SFSC_OK)
                {
#ifdef ENABLE_PRINTS
                    console_print_char("Registering server failed: ");
                    console_println_int8(op_result);
#endif
                    return 0;
                }
            }
            op_result = user_task(&adapter);
            if (op_result != SFSC_OK)
            {
#ifdef ENABLE_PRINTS
                console_print_char("User task error: ");
                console_println_int8(op_result);
#endif
                return 0;
            }
            long_running_task_step_nonblocking();
            if (next_request_time != 0 && next_request_time < time_ms())
            {
                op_result = send_a_request();
                if (op_result != SFSC_OK)
                {
#ifdef ENABLE_PRINTS
                    console_print_char("Failed sending a request: ");
                    console_println_int8(op_result);
#endif
                    return 0;
                }
            }
        }
        return 1;
    }
    else
    {
#ifdef ENABLE_PRINTS
        console_print_char("System task error: ");
        console_println_int8(op_result);
#endif
        return 0;
    }
}

int example_main()
{
    // Start a session
    sfsc_int8 op_result =
        start_session_bootstraped(&adapter, host, CONTROL_PUB_PORT);
    if (op_result == SFSC_OK)
    {
        while (1) // global execution loop
        {
            if (!example_step())
            {
                return -1;
            }
        }
    }
    else
    {
#ifdef ENABLE_PRINTS
        console_print_char("Failed to start bootstrapping: ");
        console_println_int8(op_result);
        return -1;
#endif
    }
}
#endif