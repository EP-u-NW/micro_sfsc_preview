/** @file
 *  @brief Public header that contains all SFSC functions.
 */

#ifndef SRC_SFSC_ADAPTER_SFSC_ADAPTER_H_
#define SRC_SFSC_ADAPTER_SFSC_ADAPTER_H_

#include "../platform/sfsc_strings.h"
#include "../platform/sfsc_types.h"
#include "../proto_gen/generated.pb.h"
#include "sfsc_adapter_config.h"
#include "sfsc_error_codes.h"
#include "sfsc_states.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SFSC_OK 0

#define SERVICE_TYPE_SERVER 1
#define SERVICE_TYPE_PUBLISHER 0

#define UUID_LEN 36

/**
 * @brief Contains all state memory for a sfsc_adapter instance
 *
 * In most cases, you do not need to interact with the fields of a sfsc_adapter
 * struct directly. Therefor, the struct members are not exposed in this header.
 * The for you relevant fields should be accessed throught the adapter_stats
 * function.
 *
 * Note however, that the compilation unit declaring the sfsc_adapter struct
 * needs a full specification of it. Only this compilation unit should include
 * sfsc_adapter_struct.h.
 */
typedef struct _sfsc_adapter sfsc_adapter;

#define sfsc_adapter_stats_DEFAULT_INIT \
    { NULL, {0}, {0}, SFSC_STATE_NONE, 0, 0 }

/**
 * @brief The sfsc_adapter_stats struct contains the infromation needed by the
 * user.
 *
 * The stats of an adapter should be accessd throught the adapter_stats
 * function. The fields of this struct are all read-only and should not be
 * modified by you.
 *
 * The address field specifies the address of the core and is implicitly set by
 * you during the start_session_bootstraped or start_session functions.
 * The format of the address is up to you and can be anything, as long it is
 * understood by your implementation of socket_connect (see sfsc_sockets.h).
 *
 * The adapter_id and core_id fields indicate this adapters id and the id of
 * the core it is connected to respectivly. They are both filled during the
 * handshake, and ready to use once the adapters state is operational.
 *
 * The state indicates to current connection state of an adapter. The various
 * states are defined in sfsc_states.h and are all prefixed with SFSC_STATE_.
 * An adapter is considered operational if the value of this field is >=
 * SFSC_STATE_OPERATIONAL.
 *
 * Due to the execution and memory model of this framework it can occur that
 * some messages receieved from the network are dropped (see the system_task
 * function for details). The discarded_message_count keeps track of the number
 * of lost messages.
 *
 * The query_in_progress is set to 1 if you started a query process using
 * query_services, and will be reset to 0 if the query process is terminated.
 * See the query functions for more information.
 *
 */
typedef struct _sfsc_adapter_stats {
    const char* address;
    sfsc_uint8 adapter_id[UUID_LEN];
    sfsc_uint8 core_id[UUID_LEN];
    sfsc_uint8 state;
    sfsc_uint32 discarded_message_count;
    sfsc_bool query_in_progress;
} sfsc_adapter_stats;

#define sfsc_buffer_DEFAULT_INIT \
    { NULL, 0 }
#define SERVICE_TOPIC_AUTOGEN sfsc_buffer_DEFAULT_INIT
/**
 * @brief A simple structure to store binary data and their length.
 *
 * This structure type is widely used in the framework for compact data storing.
 * It is important to notice that some functions will take a sfsc_buffer struct
 * as parameter directly, while others will work with pointers. In the most
 * cases, this says something about the mutality of the content.
 *
 * In general, for the use-time of a sfsc_buffer (usually defined in the
 * respective functions documentation), the memory area the buffer points to
 * must be valid and of the in the struct specified length.
 *
 * When passing the struct to a function, it must be immutable, meaning that for
 * the use-time of the sfsc_buffer, the content must not change.
 *
 * When passing a pointer to a function, the content is allowed to be mutable,
 * meaning that during the use-time of the sfsc_buffer, you are allowed to
 * change the content pointer and thereby change the memory area are this
 * sfsc_buffer is backed by. If you do so, don't forget to update the length
 * field accordingly.
 *
 */
typedef struct _sfsc_buffer {
    const sfsc_uint8* content;
    sfsc_size length;
} sfsc_buffer;
extern const sfsc_buffer sfsc_buffer_default;

#define sfsc_subscriber_DEFAULT_INIT \
    { sfsc_buffer_DEFAULT_INIT, NULL }
typedef struct _sfsc_subscriber sfsc_subscriber;
/**
 * @brief Struct that contains the necessary state memory to subscribe to a
 * publisher service.
 *
 * To subscribe to a publisher service, declare a sfsc_subscriber somewhere,
 * fill it and then register it with an adapter using the register_subscriber
 * function. The subscriber struct must be valid as long as the subscriber is
 * registered.
 *
 * Filling a subscriber means to configure the topic field and the on_data
 * callback.
 *
 * Usually, you will recieve the
 * topic of the publisher service you want to subscribe to from the
 * query_services function. The topics region must be immutable while the
 * subscriber is registered (for more information what immutable means, see the
 * sfsc_buffer documentation).
 *
 * The on_data field is allowed to change. It is invoked, when the corresponding
 * publisher publishes a message. The payload buffer is only valid during the
 * current user task micro step (see the user_task documentation).
 * b_auto_advance is an out-parameter (meaning that you should set it), that
 * lets you pause the user task on the current mirco step: if you set it to 0,
 * you will enter the pause state and the payload pointer will be valid, even
 * after the callback returns. On the other hand, the user task will not advance
 * to the next micro step until you leave the pause state manually (see
 * advance_user_ring). Usually, you want to set b_auto_advance to 1.
 *
 */
struct _sfsc_subscriber {
    sfsc_buffer topic;
    void (*on_data)(sfsc_adapter* adapter, sfsc_subscriber* subscriber,
                    sfsc_buffer payload, sfsc_bool* b_auto_advance);
};

#define sfsc_publisher_DEFAULT_INIT \
    { NULL, 0, 0, sfsc_SfscId_init_default, SERVICE_TOPIC_AUTOGEN, 0 }
typedef struct _sfsc_publisher sfsc_publisher;
/**
 * @brief State memory for a publisher service.
 *
 * To create a publisher, declare a sfsc_publisher struct, fill it, and use
 * either the register_publisher or the register_publisher_unregistered
 * function. The sfsc_publisher struct must be valid until it is unregistered by
 * an call to the unregister_publisher function.
 *
 * Filling a publisher is optional and means to set values to the
 * on_subscription_change, the service_id and topic fields.
 *
 * If you want to choose a service id for the publisher yourself, you can fill
 * the service_id before registration with an valid 128bit UUID in
 * standard-hexgroup-format (see the random_uuid for more information). Usually,
 * you want to set this field to sfsc_SfscId_init_default (what it already is
 * for any instance initalized with sfsc_publisher_DEFAULT_INIT) and let the
 * framework automatically generate a service id. After registration, you should
 * not change the value of this field.
 *
 * If you want to choose a topic for the publisher yourself you can configure
 * the topic buffer to point to a valid topic. The topic buffer must be valid
 * and immutable (see the sfsc_buffer documentation for more insight on this) as
 * long as the publisher is registered. If you want the framework to choose a
 * topic for you, set this field to SERVICE_TOPIC_AUTOGEN (what it already is
 * for any instance initalized with sfsc_publisher_DEFAULT_INIT). To save
 * memory, the framework will then simply use the service_id as topic.
 *
 * If the subscription count of a publisher changes, the on_subscription_change
 * callback is invoked. The parameters of the callback indicate the old and new
 * subscription count. They are either 0 or at maximum 1, even if there might be
 * more subscribers, since sfsc can only tell if there is at least 1 subscriber.
 * During execution of the callback function, the last_subscribers and
 * current_subscribers fields (not callback parameters!) are unfedined. After
 * the callback function returns, they will match the values of the parameters.
 * In any case, they are read-only!
 *
 * The unregistered field is set by the framework and depends on wether you
 * registered a publisher with the register_publisher or the
 * register_publisher_unregistered function. It indicates if the publisher is
 * registered in the cores service registry. You must not change it!
 *
 */
struct _sfsc_publisher {
    void (*on_subscription_change)(sfsc_adapter* adapter,
                                   sfsc_publisher* publisher,
                                   sfsc_uint8 last_subscribers,
                                   sfsc_uint8 current_subscribers);
    sfsc_uint8 last_subscribers;
    sfsc_uint8 current_subscribers;
    sfsc_SfscId service_id;
    sfsc_buffer topic;
    sfsc_bool unregistered;
};

#define sfsc_server_DEFAULT_INIT                                                        \
    {                                                                                   \
        NULL,                                                                           \
            sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_init_default, \
            sfsc_SfscId_init_default, SERVICE_TOPIC_AUTOGEN, 0                          \
    }
typedef struct _sfsc_server sfsc_server;
/**
 * @brief State memory for a server service.
 *
 * To create a server, declare a sfsc_server struct, fill it, and use the
 * register_service function. The sfsc_server struct must be valid until it is
 * unregistered by an call to the unregister_server function.
 *
 * In constrast to the creating process of a publisher, filling a server is NOT
 * optional.
 *
 * If you want to choose a service id for the server yourself, you can fill the
 * service_id before registration with an valid 128bit UUID in
 * standard-hexgroup-format (see the random_uuid for more information). Usually,
 * you want to set this field to sfsc_SfscId_init_default (what it already is
 * for any instance initalized with sfsc_server_DEFAULT_INIT) and let the
 * framework automatically generate a service id. After registration, you should
 * not change the value of this field.
 *
 * If you want to choose a topic for the server yourself you can configure the
 * topic buffer to point to a valid topic. The topic buffer must be valid and
 * immutable (see the sfsc_buffer documentation for more insight on this) as
 * long as the server is registered. If you want the framework to choose a topic
 * for you, set this field to SERVICE_TOPIC_AUTOGEN (what it already is for any
 * instance initalized with sfsc_server_DEFAULT_INIT). To save memory, the
 * framework will then simply use the service_id as topic.
 *
 * The is_channel field indicates if this server service is a channel service.
 * Channel services do not answer requests with normal binary payload, but
 * answer them with publisher service definitions. Then, the adapter that made
 * the request can subscribe to the publisher and recive values in a streamlike
 * way. After registration, you should not change the value of this field.
 *
 * The ack_settings field describe this servers acknwoledge strategy: usually,
 * if a server answers a request, the requestor will send back an acknowledge
 * message to the server, so that the server knows that the request was
 * succesfully served. If this acknowledge message does not reach the server
 * after ack_settings.timeout_ms milliseconds, the server will attempt to
 * retransmit the answer, up to ack_settings.send_max_tries times. If
 * ack_settings.send_max_tries is set to 0, this server won't wait for
 * acknowledges and use a fire-and-forget approach. This has some beneficial
 * implications to the answer_request and answer_channel_request functions,
 * documented there. In most cases, using a fire-and-forget approach is valid,
 * since most transmission errors will be corrected on the tcp layer, and a
 * performant sfsc core will rarely drop messages on the zmtp layer.
 *
 * The on_request callback is invoked every time a request for this server
 * service is receieved. It is allowed to change the on_request callback, even
 * after registering th service. After receiving a request, you will usually
 * take some actions based on the payload and eventually send an answer back to
 * the requestor using the answer_request or answer_channel_request function.
 *
 * The payload buffer is only valid during the current user task micro step
 * (see the user_task documentation). b_auto_advance is an out-parameter
 * (meaning that you should set it), that lets you pause the user task on the
 * current mirco step: if you set it to 0, you will enter the pause state and
 * the payload pointer will be valid, even after the callback returns. On the
 * other hand, the user task will not advance to the next micro step until you
 * leave the pause state manually (see advance_user_ring).
 *
 * The expected_reply_id and reply_topic are pull-throught parameters, meaning
 * that you don't need to interact with them, but pass them to the
 * answer_request or answer_channel_request once you want to answer the request.
 * The reply_topic is like the payload also only valid during the current user
 * task micro step.
 *
 * If you attempt to answer an request right in the callback consider the
 * following: For fire-and-forget servers, this is valid, since the reply_topic
 * you receive by the on_request callback will be passed to the answer_request
 * function, which will return before the on_request callback returns. For
 * non-fire-and-forget servers, this is not valid, since the reply_topic must be
 * valid till the corresonding on_ack call, and not till the answer_request
 * function returns. To work arround this, you can either copy the content of
 * the reply_topic somewhere global (recommended) or pause the user task, until
 * the corresonding on_ack is invoked. Even if the user task is in pause state,
 * the on_ack callback of an answer_request will be invoked, so there will be no
 * deadlock.
 */

struct _sfsc_server {
    void (*on_request)(sfsc_adapter* adapter, sfsc_server* server,
                       sfsc_buffer payload, sfsc_int32 expected_reply_id,
                       sfsc_buffer reply_topic, sfsc_bool* b_auto_advance);
    sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings ack_settings;
    sfsc_SfscId service_id;
    sfsc_buffer topic;
    sfsc_bool is_channel;
};

#define sfsc_publisher_or_server_INIT_DEFAULT \
    { 0, {NULL} }
/**
 * @brief Container to point either to a sfsc_server or sfsc_publisher.
 *
 */
typedef struct _sfsc_publisher_or_server {
    sfsc_bool is_server;
    union {
        sfsc_publisher* publisher;
        sfsc_server* server;
    } service;
} sfsc_publisher_or_server;

#define relative_publisher_tags_DEFAULT_INIT \
    { 0, 0, 0, 0, 0 }
/**
 * @brief Represents service tags specific to publisher services.
 *
 * The format and idea of the fields is very similar to
 * relative_sfsc_service_descriptor, see there for an explanation.
 *
 * An exception to this is the unregistered field. See the sfsc_publisher struct
 * documentation for an explanation.
 */
typedef struct _relative_publisher_tags {
    sfsc_size topic_offset;
    sfsc_size topic_len;
    sfsc_size output_message_type_offset;
    sfsc_size output_message_type_len;
    sfsc_bool unregistered;
} relative_publisher_tags;
extern const relative_publisher_tags relative_publisher_tags_default;

#define relative_server_tags_DEFAULT_INIT                                              \
    {                                                                                  \
        0, 0, 0, 0, 0, 0,                                                              \
            sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_init_default \
    }
/**
 * @brief Represents service tags specific to server services.
 *
 * The format and idea of the fields is very similar to
 * relative_sfsc_service_descriptor, see there for an explanation.
 *
 * An exception to this is the ack_settings field. See the sfsc_server struct
 * documentation for an explanation.
 */
typedef struct _relative_server_tags {
    sfsc_size topic_offset;
    sfsc_size topic_len;
    sfsc_size input_message_type_offset;
    sfsc_size input_message_type_len;
    sfsc_size output_message_type_offset;
    sfsc_size output_message_type_len;
    sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings ack_settings;
} relative_server_tags;
extern const relative_server_tags relative_server_tags_default;

#define relative_sfsc_service_descriptor_DEFAULT_INIT                     \
    {                                                                     \
        sfsc_SfscId_init_default, sfsc_SfscId_init_default,               \
            sfsc_SfscId_init_default, 0, 0, 0, 0, SERVICE_TYPE_PUBLISHER, \
            {relative_publisher_tags_DEFAULT_INIT}                         \
    }

/**
 * @brief Represents all infromation about a service.
 *
 * A relative_sfsc_service_descriptor is relative because it does not contain
 * information about a service directly, but it merely serves as an index
 * structure. The indexes (which end with _offset) are relative to a memory
 * area. The memory areas address (called start in the following) is usually
 * delivered with the descriptor. For example, to now access the name of the
 * service, read name_len bytes from (start+name_offset). For reasoning, why
 * this relative apporach is used, read the last paragraph.
 *
 * An exception to the above are the core_id, adapter_id, and service_id fields,
 * which actually contain the respective ids, and thereby denote the core and
 * adapter this service belongs to, as well as this services own id.
 *
 * The service_type field is either SERVICE_TYPE_SERVER or
 * SERVICE_TYPE_PUBLISHER and indicates, how the service_tags union should be
 * treated.
 *
 * Why is this relative approach used?
 * The binary size of a serivce in sfsc is not limited, so we can not know the
 * size of the respective service fields in advance. On the other hand, this
 * framework does not use dynamic memory allocation (malloc). A possible
 * solution is to statically allocate memory for each field and add an length
 * field, to indicate, how much memory is actually used. The difference between
 * the allocated and actually used size is called waste. Instead of allocation
 * a memory area for each field, we use one bigger memory area for all fields.
 * The idea is that, because of the indivudual waste, this single field size can
 * be smaller then sum of all indivudual field sizes, while still containing
 * enough space to store all necessary information.
 */
typedef struct _relative_sfsc_service_descriptor {
    sfsc_SfscId core_id;
    sfsc_SfscId adapter_id;
    sfsc_SfscId service_id;
    sfsc_size name_offset;
    sfsc_size name_len;
    sfsc_size custom_tags_offset;
    sfsc_size custom_tags_len;
    sfsc_uint8 service_type;
    union {
        relative_publisher_tags publisher_tags;
        relative_server_tags server_tags;
    } service_tags;
} relative_sfsc_service_descriptor;

/**
 * @brief A struct containing all information needed to answer a channel
 * request.
 *
 * Instead of normal binary payload, a channel request is answered with the
 * definition of a publisher service.
 *
 * The core_id, adapter_id and service_id represent the core and adapter the
 * publisher service is conencted to, as well as the publisher service itself.
 * Their length must be UUID_LEN and their format must be
 * standard-hexgroup-format (see the random_uuid function). The reason a
 * sfsc_channel_answer uses sfsc_uint8* instead of sfsc_uint8[] is to remove
 * copying and to save memory: The publisher you are describing is most likly
 * hosted by the same adapter that will send this channel answer. Instead of
 * allocating 3*UUID_LEN memory and copy the details from the adapter to the
 * sfsc_channel_answer, you can just let the fields point to
 * &adapter_states()->core_id and &adapter_states()->adapter_id respectivly.
 *
 * All sfsc_buffers must be valid during the use-time of the sfsc_channel_answer
 * struct and are optional. If publisher_output_topic is set to
 * SERVICE_TOPIC_AUTOGEN the topic autogenerate rule (as described in
 * sfsc_publisher) will be applied, and the service_id will be used as topic.
 *
 * The unregistered should be set to 0 if the publisher you are describing in
 * this sfsc_channel_answer is also registered in the service registry, or to 1
 * if it is not.
 */
typedef struct _sfsc_channel_answer {
    sfsc_uint8* service_id;
    sfsc_uint8* adapter_id;
    sfsc_uint8* core_id;
    sfsc_buffer publisher_output_topic;
    sfsc_buffer name;
    sfsc_buffer custom_tags;
    sfsc_buffer output_message_type;
    sfsc_bool unregistered;
} sfsc_channel_answer;

/**
 * @brief Called when a command to create or delete a service succeeds.
 *
 * A create command means that the service was registered in the cores
 * event-log, a delete command means taht the service was unregistred from it.
 *
 * @param adapter the executing adapter
 * @param service a struct describing the service
 * @param created 1 if the command was a create command, 0 if it was a delete
 * command
 */
typedef void(sfsc_command_callback)(sfsc_adapter* adapter,
                                    sfsc_publisher_or_server service,
                                    sfsc_bool created);

/**
 * @brief Called during a query process with a service descriptor, or to
 * indicate that the query process is done.
 *
 * The descriptor is valid as long as you don't continue the query process
 * (using query_services_next). If you need to access the service data of a
 * service delivered by the callback after continuation, you need to write the
 * descriptor to a global place, and also copy length bytes from offset to a
 * global place. Alternatively, if you call query_services_next to end the query
 * process (by setting next to 0), the descriptor is valid until the next query
 * process.
 *
 * In the last call to this method, is_last is set to 1. The last call might not
 * contain a service. An offset value of NULL indicates that this invokation of
 * the callback does not contain a service.
 *
 * Even after the is_last call you have to invoke query_services_next with next
 * set to 0, to explicitly end the query process.
 */
typedef void(sfsc_query_callback)(sfsc_adapter* adapter,
                                  relative_sfsc_service_descriptor descriptor,
                                  sfsc_uint8* offset, sfsc_size length,
                                  sfsc_bool is_last);

/**
 * @brief Recommended way to access the stats of an adapter.
 *
 * @param adapter The target adapter
 * @return sfsc_adapter_stats* Pointer to that adapers stats
 */
sfsc_adapter_stats* adapter_stats(sfsc_adapter* adapter);

/**
 * @brief Starts a sfsc adapter session with bootstraping.
 *
 * The adapter is not operational after this function returns.
 * It first needs to do a handshake and connect to the other sfsc sockets of the
 * core. You should start system-tasking (see the system_task function) with
 * this adapter and check the state field of the stats object (accessible with
 * the adapter_stats function) after each step. It will eventually become
 * SFSC_STATE_OPERATIONAL, making the adapter operational.
 *
 * @param adapter Pointer to the adapter struct, all state information will be
 * saved there
 * @param address The address of the core, it will be passed to your socket
 * implementation
 * @param original_control_pub_port The port of the cores control pub socket
 * @return sfsc_int8 SFSC_OK or an error code
 */
sfsc_int8 start_session_bootstraped(sfsc_adapter* adapter, const char* address,
                                    int original_control_pub_port);

/**
 * @brief Stats a sfsc adapter session without bootstraping.
 *
 * See start_session_bootstraped for details.
 */
sfsc_int8 start_session(sfsc_adapter* adapter, const char* address,
                        int original_control_pub_port,
                        int original_control_sub_port,
                        int original_data_pub_port, int original_data_sub_port);

/**
 * @brief Subscribes to a sfsc publisher through the given adapter.
 *
 * The publishers topic which should be subscribed to and
 * the callback function which should be invoked can be
 * set in the corresponding sfsc_subscriber struct.
 * For mutability rules of topic and callback function (if they are allowed
 * to change), see the sfsc_subscriber structs documentation.
 *
 * Registering a subscriber saves a pointer to that subscriber in the adapter
 * state, so you must not copy the subscriber struct around as long as it is
 * registered with the adapter! Also, registering will fail if there is no free
 * subscriber memory slot to store the pointer to that subscriber in the
 * adapter. In this case E_NO_FREE_SLOT will be returned. The maximal number of
 * at the same time registered subscribers per adapter can be configured using
 * MAX_SUBSCRIBERS.
 *
 * Calling this function with an non-operational adapter will result in
 * unpredictable behaviour! Registering the same subscriber multiple times will
 * also result in unpredictable behaviour!
 *
 * @param adapter An operational adapter which will be used for the subscription
 * @param subscriber Pointer to the subscriber struct holding the topic and
 * callback information
 * @return sfsc_int8 SFSC_OK on success or one of the error codes below
 * @retval E_NO_FREE_SLOT There are already MAX_SUBSCRIBERS registered with this
 * adapter, so there is no memory slot left for this one
 * @retval Various Network Errors	There are serveral other,
 * network-related errors that can occure
 */
sfsc_int8 register_subscriber(sfsc_adapter* adapter,
                              sfsc_subscriber* subscriber);
/**
 * @brief Unregisters a subscriber and unsubscribe messages on that topic.
 *
 * If the given subscriber is registered with this adapter to receieve published
 * messages, it will be unregistered and a subscriber memory slot in the adapter
 * will be freed. After this function returns, the callback of the subscriber
 * will not be invoked again.
 *
 * Unregistering a not registered subscriber will result in a success.
 *
 * @param adapter The operational adapter, from which to unregister the
 * subscriber
 * @param subscriber The subscriber to remove
 * @return sfsc_int8 SFSC_OK on success or one of the Various Network Errors
 */
sfsc_int8 unregister_subscriber(sfsc_adapter* adapter,
                                sfsc_subscriber* subscriber);

/**
 * @brief Starts a query process to obtain registered services from the core.
 *
 * Only one query process might be running simultaniously. During a query
 * process, all valid services will be delivered through the callback. The query
 * process queries the cores event log in reversed order, so that more recently
 * registered services will be found first.
 *
 * After a service is found and delivered to the calling application throught
 * the callback, the query process is paused, until it is explicitly continued
 * by a call to the query_services_next function. See the documentation of the
 * sfsc_query_callback for more information on the delivered service data.
 *
 * @param adapter An operational adapter throught which the core is queried
 * @param callback The callback all receieved services are delivered to
 * @return sfsc_int8 SFSC_OK if starting the query process was successfull, or
 * one of the error codes below
 * @retval E_QUERY_IN_PROGRESS	If there is already an other query process
 * @retval Various Network Errors	There are serveral other,
 * network-related errors that can occure
 *
 */
sfsc_int8 query_services(sfsc_adapter* adapter, sfsc_query_callback* callback);

/**
 * @brief Tells the framework to continue or to end a currently ongoing query
 * process.
 *
 * Once a service is found during the query process, the associated callback
 * is invoked and the query process enters a pause state. It is then either
 * continued by a call to this function with next set to 1 or ended with
 * next set to 0.
 *
 * It is not neccesary, but allowed, that this function is invoked from
 * a query callback. It is also allowed for it to be invoked from any other
 * part of the programm, as long as it is invoked eventually.
 *
 * Calling this function while there is no query process ongoing (checkable
 * by the adapter_stats function) will result in undefined behaviour.
 *
 * @param adapter The adapter which is currently in a query process
 * @param next 1 to continue the query process, 0 to end it
 */
void query_services_next(sfsc_adapter* adapter, sfsc_bool next);

/**
 * @brief Sets up a publisher service and registers it with the core.
 *
 * If you want to set up the services id and topic manually, see the
 * sfsc_publisher struct documentation for instructions.
 *
 * After successfull registration, this function will set the unregistered field
 * of the publisher to 0.
 *
 * All sfsc_buffer parameters for this method must be immutable and mut be valid
 * until this function returns (their use-time equals this functions run time).
 * For more information about mutability, see the sfsc_buffer struct
 * documentation. To omit an optional sfsc_buffer parameter use
 * sfsc_buffer_default as value.
 *
 * Registering a publisher saves a pointer to that publisher in the adapter
 * state, so you must not copy the publisher struct around as long as it is
 * registered with the adapter! Also, registering will fail if there is no free
 * publisher memory slot to store the pointer to that publisher in the adapter.
 * In this case E_NO_FREE_SLOT will be returned. The maximal number of at the
 * same time registered publisher per adapter can be configured using
 * MAX_PUBLISHERS.
 *
 * Also, since the publisher will be registered in the service registry, this
 * call requires a free command memory slot and will also return E_NO_FREE_SLOT
 * if their are currently already MAX_SIMULTANIOUS_COMMANDS ongoing. The command
 * memory slot will be occupied until the callback is invoked.
 *
 * @param adapter An operational adapter
 * @param publisher The publisher to register with this adapter, see the
 * sfsc_publisher struct for more information
 * @param name The name of this publisher
 * @param custom_tags Optional; custom tags for the publisher
 * @param output_message_type Optional; can be used to annotate the format of
 * the published messages
 * @param callback Optional; a callback that is invoked after service
 * registration with the core was successful
 * @return sfsc_int8 SFSC_OK or one of the error codes below
 * @retval SFSC_OK This indicates that registering the publisher into the
 * adapter was successfull, and that the appropriate command to register the
 * publisher into the cores service registry was issued.
 * @retval E_NO_FREE_SLOT There are already MAX_PUBLISHERS registered with this
 * adapter, so there is no memory slot left for this one OR if there are
 * currently already MAX_SIMULTANIOUS_COMMANDS ongoing
 * @retval Various Network Errors	There are serveral other,
 * network-related errors that can occure
 */
sfsc_int8 register_publisher(sfsc_adapter* adapter, sfsc_publisher* publisher,
                             sfsc_buffer name, sfsc_buffer custom_tags,
                             sfsc_buffer output_message_type,
                             sfsc_command_callback* callback);
/**
 * @brief Sets up a publisher you can publish with, but does not register it in
 * the cores service registry.
 *
 * This function behaves mostly like register_publisher, but since this
 * publisher is not registered into the service registry, it does not have a
 * name, custom_tags or an ouput_message_type.
 *
 * It will still have an service id and a topic, which are either setup manually
 * or automaticly generated, according to the rules specified in the
 * sfsc_publisher documentation. Also, for registering the publisher with the
 * adapter, the adapter needs to have a free publisher memory slot (again, see
 * register_publisher).
 *
 * This function will set the unregistred field of the publisher to 1.
 *
 * A subscriber can subscribe to an unregistred publisher, if it knows the
 * unregistered publishers topic. In most cases, this will happen in the context
 * of a channel request.
 *
 * Even if the publisher is not registered into the service registry, it sill
 * needs to be properly unregistered form the adapter after usage by the
 * unregister_publisher function!
 *
 * @param adapter An operational adapter
 * @param publisher The publisher to register with this adapter, see the
 * sfsc_publisher struct for more information
 * @return sfsc_int8 SFSC_OK or one of the error codes below
 * @retval SFSC_OK This indicates that registering the publisher into the
 * adapter was successfull, and that the appropriate command to register the
 * publisher into the cores service registry was issued.
 * @retval E_NO_FREE_SLOT There are already MAX_PUBLISHERS registered with this
 * adapter, so there is no memory slot left for this one
 * @retval Various Network Errors	There are serveral other,
 * network-related errors that can occure
 */
sfsc_int8 register_publisher_unregistered(sfsc_adapter* adapter,
                                          sfsc_publisher* publisher);
/**
 * @brief Unregisters a publisher.
 *
 * Unregistering frees the publishers publisher memory slot in the adapter.
 *
 * If this publisher is registered with the service registry, it will be removed
 * from it. Removing requires a free command memory slot and will return
 * E_NO_FREE_SLOT if there are currently already MAX_SIMULTANIOUS_COMMANDS
 * ongoing. The command memory slot will be occupied until the callback is
 * invoked.
 *
 * Even if the publisher is not registered with the service registry (because it
 * was created with register_publisher_unregistered), this function must still
 * be called!
 *
 * The callback is invoked after the publisher was removed from the service
 * registry, with created set to 0. Since with unregistered publishers this is
 * never the case, the callback is ignored and should be set to NULL.
 *
 * @param adapter An operational adapter
 * @param publisher The publisher to shut down
 * @param callback For registered publishers only; Invoked after the publisher
 * was removed from the service registry
 * @return sfsc_int8 SFSC_OK or one of the error codes below
 * @retval SFSC_OK This indicates that unregistering the publisher from the
 * adapter was successfull, and if this publisher is registered in the service
 * registry, that the appropriate command to unregister the publisher from there
 * was issued
 * @retval E_NO_FREE_SLOT There are currently already MAX_SIMULTANIOUS_COMMANDS
 * ongoing
 * @retval Various Network Errors	There are serveral other,
 * network-related errors that can occure
 */
sfsc_int8 unregister_publisher(sfsc_adapter* adapter, sfsc_publisher* publisher,
                               sfsc_command_callback* callback);
/**
 * @brief Publishes data through a publisher.
 *
 * The publisher must be registered with this adapter, either by the
 * register_publisher or the register_publisher_unregistered function.
 *
 * The use-time of the payload is this functions runtime and must be immutable
 * for the duration.
 *
 * @param adapter An operational adapter this publisher is registered to
 * @param publisher The publisher to publish the data
 * @param payload The data to publish
 * @return sfsc_int8 SFSC_OK
 */
sfsc_int8 publish(sfsc_adapter* adapter, sfsc_publisher* publisher,
                  sfsc_buffer payload);

/**
 * @brief Invoked when an answer to a request is receieved, or when the request
 * times out.
 *
 * The timeout parameter is set to 1 if the callback invokation is caused by a
 * timeout, to 0 if an answer is deleivered.
 *
 * mapping_arg is a mapping-argument (explained in the request functions
 * documentation with an example).
 *
 * The payload buffer is only valid during the current user task micro step
 * (see the user_task documentation). b_auto_advance is an out-parameter
 * (meaning that you should set it), that lets you pause the user task on the
 * current mirco step: if you set it to 0, you will enter the pause state and
 * the payload pointer will be valid, even after the callback returns. On the
 * other hand, the user task will not advance to the next micro step until you
 * leave the pause state manually (see advance_user_ring). Usually, you want to
 * set b_auto_advance to 1. If the request timed out, you should not modify
 * b_auto_advance, as it will point to NULL.
 */
typedef void(sfsc_request_callback)(sfsc_adapter* adapter, sfsc_buffer payload,
                                    sfsc_bool timeout, void* mapping_arg,
                                    sfsc_bool* b_auto_advance);

/**
 * @brief Invoked when an answer to a channel request is receieved, or when the
 * request times out.
 *
 * The timeout parameter is set to 1 if the callback invokation is caused by a
 * timeout, to 0 if an answer is deleivered.
 *
 * The mapping_arg parameter is a mapping-argument (explained in the request
 * functions documentation with an example).
 *
 * If decode_error is not SFSC_OK, an error occured during decoding the received
 * service definition. This will most likley happen due to a too small memory
 * area for the descriptor. In this case decode_error is set to
 * E_BUFFER_INSUFFICIENT, and descirptor_length indicates, how much memory would
 * have been needed.
 *
 * In contrast to the payload in a normal sfsc_request_callback, you have
 * allocated the descriptor struct and the descriptor memory area yourself, so
 * they will be valid until you do something with them, and won't become invalid
 * once the callback returns. Thus, the b_auto_advance does not influence the
 * validity, and is just there as a convenience tool to enter the user task
 * pause state (see the user_task documentation).
 */
typedef void(sfsc_channel_request_callback)(
    sfsc_adapter* adapter, sfsc_bool timeout, sfsc_int8 decode_error,
    relative_sfsc_service_descriptor* descriptor, sfsc_uint8* descirptor_offset,
    sfsc_size descirptor_length, void* mapping_arg, sfsc_bool* b_auto_advance);

/**
 * @brief Makes a request call to a server service.
 *
 * The topic of the server service is usually obtained by a registry query.
 * The use-time of the topic and the payload equal the functions runtime (
 * until this function returns) and
 * must both be immutable for this time (see the sfsc_buffer documentation for
 * more information about mutability).
 *
 * If an answer to the request is received, the callback is invoked.
 * For information on how the data will be delivered, see the callbacks
 * documentation.
 *
 * You can specify a timeout_time in ms. If this time passes without receiving
 * an answer to this request, the callback is invoked with timeout set to 1.
 *
 * Each request you want to make needs a request memory slot in the given
 * adapter for the time of the request (until the callback is invoked).
 * If there is no free request memory slot, E_NO_FREE_SLOT will be returned.
 * You can configure the amount of available request memory slots using
 * MAX_SIMULTANIOUS_REQUESTS.
 *
 * As the use-time of the request topic and payload is only the functions
 * runtime, they are both not necessarily valid when an answer is recieved, and
 * thus can not be passed to the callback. But most certainly you want to know
 * to which request the recieved answer belongs. One way is to declare a
 * separate callback function for each of your requests. An other way to let you
 * know which request function call the callbacks invokation belongs to, is to
 * use the optional mapping_arg parameter as a so called mapping-argument. If
 * and what you store behind the mapping_arg pointer is opaque to the framework.
 * The callback function will be invoked with this mapping_arg. You can then,
 * based on the mapping_arg, reason, what request call the answer belongs to.
 * For example, if you know that even if the use-time of the request topic
 * buffer is over it will continue to be valid, you can pass this as
 * mapping_arg. Or you define that you will use the mapping_arg pointer as
 * numerical value and treat it as an normal integer number instead of a
 * pointer. Then you can assign unique request ids to your topics.
 *
 * @param adapter The operational adapter to make the request with
 * @param topic The server services topic to make the request to
 * @param payload The payload of the request
 * @param timeout_time The timeout time in ms, 0 for no timeout restrictions
 * @param callback The callback that is invoked when receiving an answer or on
 * timeout
 * @param mapping_arg Optional; Serves as mapping-argument (see the text above
 * for explanation)
 * @return sfsc_int8 SFSC_OK or one of the error codes below
 * @retval SFSC_OK This indicates that registering the publisher into the
 * adapter was successfull, and that the appropriate command to register the
 * publisher into the cores service registry was issued.
 * @retval E_NO_FREE_SLOT There currently already MAX_SIMULTANIOUS_REQUESTS
 * MAX_PUBLISHERS ongoing
 * @retval Various Network Errors	There are serveral other,
 * network-related errors that can occure
 */
sfsc_int8 request(sfsc_adapter* adapter, sfsc_buffer topic, sfsc_buffer payload,
                  sfsc_uint64 timeout_time, sfsc_request_callback* callback,
                  void* mapping_arg);

/**
 * @brief Makes a request call to a channel server service.
 *
 * This function behaves like the request function, so see there for
 * documentation. The only difference is, what the answer is, and how it is
 * delivered.
 *
 * Instead of normal binary payload, a channel_request will result in a
 * publisher service description as answer, in the same format as the
 * query_service function would return it. The difference to the query_service
 * function is, that the framework can not know, how many simultaneous channel
 * requests you want to make. Thus it can not statically allocate memory for the
 * service descriptors. As consequence, you have to allocate this memory
 * yourself and pass it to this method. The memory must be large enough to hold
 * the received service. If you do not know anything about the size of the
 * service, it might be a good idea to use REGISTRY_BUFFER_SIZE as orientation.
 * What happens if the descriptor_space_length is insufficient is described in
 * the sfsc_channel_request_callback documentation.
 *
 * @param descriptor Pointer to a relative_sfsc_service_descriptor which will be
 * filled
 * @param descriptor_space Pointer to the start of the usable memory area
 * @param descriptor_space_lenght Length of the usable memory area
 */
sfsc_int8 channel_request(sfsc_adapter* adapter, sfsc_buffer topic,
                          sfsc_buffer payload, sfsc_uint64 timeout_time,
                          relative_sfsc_service_descriptor* descriptor,
                          sfsc_uint8* descriptor_space,
                          sfsc_size descriptor_space_lenght,
                          sfsc_channel_request_callback* callback,
                          void* mapping_arg);

/**
 * @brief Sets up a server service and registers it with the core.
 *
 * If you want to set up the services id and topic manually, see the sfsc_server
 * struct documentation for instructions.
 *
 * Also, the function that is invoked when recieving requests,
 * the servers acknowledge strategy and if this is a channel service must be
 * configured in the sfsc_server before calling this function, so see their for
 * instructions.
 *
 * All sfsc_buffer parameters for this method must be immutable and must be
 * valid until this function returns (their use-time equals this functions run
 * time). For more information about mutability, see the sfsc_buffer struct
 * documentation. To omit an optional sfsc_buffer parameter use
 * sfsc_buffer_default as value.
 *
 * Registering a server saves a pointer to that server in the adapter state,
 * so you must not copy the server struct around as long as it is registered
 * with the adapter! Also, registering will fail if there is no free server
 * memory slot to store the pointer to that server in the adapter. In this case
 * E_NO_FREE_SLOT will be returned. The maximal number of at the same time
 * registered server per adapter can be configured using MAX_SERVERS.
 *
 * Also, since the server will be registered in the service registry, this call
 * requires a free command memory slot and will also return E_NO_FREE_SLOT if
 * their are currently already MAX_SIMULTANIOUS_COMMANDS ongoing. The command
 * memory slot will be occupied until the callback is invoked.
 *
 * @param adapter An operational adapter
 * @param server The server to register with this adapter, see the sfsc_server
 * struct for more information
 * @param name The name of this server
 * @param custom_tags Optional; custom tags for the server
 * @param output_message_type Optional; can be used to annotate the format of
 * the messages the server sends as answers
 * @param input_message_type  Optional; can be used to annotate the format of
 * the messages the server accepts on requests
 * @param callback Optional; a callback that is invoked after service
 * registration with the core was successful
 * @return sfsc_int8 SFSC_OK or one of the error codes below
 * @retval SFSC_OK This indicates that registering the server into the adapter
 * was successfull, and that the appropriate command to register the into into
 * the cores service registry was issued.
 * @retval E_NO_FREE_SLOT There are already MAX_SERVERS registered with this
 * adapter, so there is no memory slot left for this one OR if there are
 * currently already MAX_SIMULTANIOUS_COMMANDS ongoing
 * @retval Various Network Errors	There are serveral other,
 * network-related errors that can occure
 */
sfsc_int8 register_server(sfsc_adapter* adapter, sfsc_server* server,
                          sfsc_buffer name, sfsc_buffer custom_tags,
                          sfsc_buffer output_message_type,
                          sfsc_buffer input_message_type,
                          sfsc_command_callback* callback);

/**
 * @brief Unregisters a server.
 *
 * Unregistering frees the servers server memory slot in the adapter.
 *
 * The server will be removed from the service registry.
 * Removing requires a free command memory slot and will return E_NO_FREE_SLOT
 * if there are currently already MAX_SIMULTANIOUS_COMMANDS ongoing. The command
 * memory slot will be occupied until the callback is invoked. The callback is
 * invoked after the server was removed from the service registry, with created
 * set to 0.
 *
 * @param adapter An operational adapter
 * @param server The server to shut down
 * @param callback Invoked after the server was removed from the service
 * registry
 * @return sfsc_int8 SFSC_OK or one of the error codes below
 * @retval SFSC_OK Indicates that unregistering the server from the adapter was
 * successfull and that the appropriate command to unregister the server from
 * the service registry was issued
 * @retval E_NO_FREE_SLOT There are currently already MAX_SIMULTANIOUS_COMMANDS
 * ongoing
 * @retval Various Network Errors	There are serveral other,
 * network-related errors that can occure
 */
sfsc_int8 unregister_server(sfsc_adapter* adapter, sfsc_server* server,
                            sfsc_command_callback* callback);

/**
 * @brief Invoked if an ack message for an answer was receieved, or the servers
 * timeout condition was meet.
 *
 * If timeout is set to 1 all retransmission attempts failed (see the
 * sfsc_server.ack_settings), if it is set to 0, the requestor acknowledged the
 * answer.
 *
 * The mapping_arg parameter is a mapping-argument (explained in the request
 * functions documentation with an example).
 *
 */
typedef void(sfsc_answer_ack_callback)(sfsc_adapter* adapter,
                                       sfsc_server* server, sfsc_bool timeout,
                                       void* mapping_arg);
/**
 * @brief Answers a request.
 *
 * After sending an answer, the original requestor will (hopefully) receive it
 * and send an acknowledge for the answer. If the acknowledge is received in
 * time, the on_ack callback will be invoked. If not, an attempt is
 * automatically made to send it again. The maximal send again attempt count and
 * the wait time are configured in the server_struct (see there for more
 * information). If a server does not require its answers to be acknowledged and
 * which are thus only send once, it is called a fire-and-forget server.
 *
 * The expected_reply_id and reply_topic are given to you by the on_request
 * function of the corresponding sfsc_server. The use-time of the reply_topic is
 * the time until the on_ack callback is invoked. During this time, it must be
 * valid and immutable. For fire-and-forget servers, the use-time of the
 * reply_topic is only this functions runtime, meaning that its only neccessary
 * to be vaid and immutable until this function returns.
 *
 * The payload parameter is a pointer to the actual payload you want to
 * transmitt in the answer. The use-time for this is either until the on_ack
 * callback is invoked, or for fire-and-forget servers, this functions runtime.
 * Since this is a pointer to a sfsc_buffer and not a sfsc_buffer, both, the
 * pointer to the sfsc_buffer and the content pointer inside that buffer must be
 * valid during the use-time. The payload buffer can be mutable: you are allowed
 * to change the content pointer or the content it points to. This is usefull in
 * some situation, e.g.: Imaging answering a request with a sensor measurement,
 * stored behind payload->content. You do not receieve an acknowledge in time,
 * so the answer is send again. But during this peroid, the the measurement
 * changed. Since you were allowed to change payload->content, you updated it,
 * and the retransmission of the answer will contain the new measurement.
 *
 * Answering a request to an non-fire-and-forget server will require a free
 * acknowledge memory slot. If an attempt is made to answer a request on such a
 * server and there are already MAX_PENDING_ACKS answer processes ongoing, a
 * E_NO_FREE_SLOT will be returned. The acknowledge memory slot is freed once
 * the on_ack callback is invoked.
 *
 * The mapping_arg parameter is a mapping-argument (explained in the request
 * functions documentation with an example).
 *
 * @param adapter An operational adapter
 * @param server The server that is answering a request
 * @param expected_reply_id An answer identification number obtained by the
 * original request
 * @param reply_topic The answers reply topic, obtained by the original request
 * @param payload The actual answer payload
 * @param mapping_arg Optional; Serves as mapping-argument
 * @param on_ack A callback to invoke once an acknowledge is received, can be
 * NULL
 * @return sfsc_int8	SFSC_OK or one of the error codes below
 * @retval E_NO_FREE_SLOT	If there is no free acknowledge memory slot
 * while trying to answer a non-fire-and-forget request
 * @retval Various Network Errors	There are serveral other,
 * network-related errors that can occure
 */
sfsc_int8 answer_request(sfsc_adapter* adapter, sfsc_server* server,
                         sfsc_int32 expected_reply_id, sfsc_buffer reply_topic,
                         sfsc_buffer* payload, void* mapping_arg,
                         sfsc_answer_ack_callback* on_ack);

/**
 * @brief Used to answer a channel request.
 *
 * This function must only be called by channel server services.
 * It behaves just like answer_request, so see there for documentation.
 *
 * The only difference is that instead of passing a pointer to a binary payload,
 * you have to pass a pointer to a sfsc_channel_answer. The channel_answers
 * use-time equals the use-time of the payload in the answer_request function.
 * It is also mutable, meaning that you are allowed to edit it.
 */
sfsc_int8 answer_channel_request(sfsc_adapter* adapter, sfsc_server* server,
                                 sfsc_int32 expected_reply_id,
                                 sfsc_buffer reply_topic,
                                 sfsc_channel_answer* channel_answer,
                                 void* mapping_arg,
                                 sfsc_answer_ack_callback* callback);

/**
 * @brief Generates and writes a random 128bit UUID in standard-hexgroup-format
 * to the target buffer.
 *
 * Since the generated UUID is random, it can be classified as type 4 UUID.
 *
 * This method generates 2 random sfsc_uint64 and formats them in
 * standard-hexgroup-format. Standard-hexgroup-format consists of 5 datagroups,
 * each group represended by ASCII letters a-f and numbers 0-9. The groups are
 * separated by the - ASCII symbol.
 *
 * Examples: 23236572-6963-6973-7473-757065722323,
 * 			 550e8400-e29b-11d4-a716-446655440000
 *
 * @param target A at least UUID_LEN bytes long buffer to write the formated
 * UUID to
 */
void random_uuid(sfsc_uint8 target[UUID_LEN]);

/**
 * @brief Executes a single system task step on the adapter.
 *
 * This function is non-blocking, and must be called cyclicly, with a high
 * enough frequency on an adapter, on which the start_session or
 * start_session_bootstraped function was called. To read more about the
 * execution model, see the readme.
 *
 * This function returns an error code, which is SFSC_OK on success.
 * An other return code indicates an error, a list or error codes can
 * be found in sfsc_errors.h and zmtp_states.h. Some errors are
 * recoverable, again, see the readme.
 *
 * @param adapter An adapter
 * @return sfsc_int8 SFSC_OK or an error code
 */
sfsc_int8 system_task(sfsc_adapter* adapter);
/**
 * @brief Executes a single system task step on the adapter.
 *
 * The adapter must be in an operational state, or this call
 * will lead to undefined behaviour.
 *
 * If this function is called, it will execute callbacks you registered.
 * Whether using blocking or long taking code in your callbacks is allowed or
 * not, depends on your execution modell (see the corresponding section in the
 * readme).
 *
 * It is important to continue calling this function, even if you entered the
 * user task pause state (mentioned in the documentation of most callback
 * functions).
 *
 * This function returns an error code, which is SFSC_OK on success.
 * An other return code indicates an error, a list or error codes can
 * be found in sfsc_errors.h and zmtp_states.h. Some errors are
 * recoverable, again, see the readme.
 *
 * @param adapter An operational adapter
 * @return sfsc_int8 SFSC_OK or an error code
 */
sfsc_int8 user_task(sfsc_adapter* adapter);
/**
 * @brief Leaves the user task pause state.
 *
 * Some callback functions allow you to pause the user task and freeze it on
 * the current micro step. To continue execution you have to call this function.
 * You should only call this funtion if you entered the pause state, and never
 * call it from the callback you entered the pause state, as this will skip
 * messages and leave your adapter in a undefined state.
 *
 * @param adapter The adapter whichs user task is currently in the pause state
 * and should continue execution
 */
void advance_user_ring(sfsc_adapter* adapter);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_ADAPTER_H_ */
