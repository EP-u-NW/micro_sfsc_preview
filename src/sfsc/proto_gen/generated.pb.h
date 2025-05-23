/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.2 */

#ifndef PB_SFSC_GENERATED_PB_H_INCLUDED
#define PB_SFSC_GENERATED_PB_H_INCLUDED
#include "../proto/pb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _sfsc_ChannelFactoryRequest {
    pb_callback_t payload;
} sfsc_ChannelFactoryRequest;

typedef struct _sfsc_CommandReply {
    char dummy_field;
} sfsc_CommandReply;

typedef struct _sfsc_MessageType {
    pb_callback_t type;
} sfsc_MessageType;

typedef struct _sfsc_SfscServiceDescriptor_CustomTagsEntry {
    pb_callback_t key;
    pb_callback_t value;
} sfsc_SfscServiceDescriptor_CustomTagsEntry;

typedef struct _sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex {
    pb_callback_t regex;
} sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex;

typedef struct _sfsc_Topic {
    pb_callback_t topic;
} sfsc_Topic;

typedef struct _sfsc_BootstrapMessage {
    int32_t core_control_pub_tcp_port;
    int32_t core_control_sub_tcp_port;
    int32_t core_data_pub_tcp_port;
    int32_t core_data_sub_tcp_port;
} sfsc_BootstrapMessage;

typedef struct _sfsc_QueryReply_Expired {
    int64_t oldest_valid_event_id;
} sfsc_QueryReply_Expired;

typedef struct _sfsc_QueryReply_Future {
    int64_t newest_valid_event_id;
} sfsc_QueryReply_Future;

typedef struct _sfsc_QueryRequest {
    int64_t event_id;
} sfsc_QueryRequest;

typedef struct _sfsc_Reply {
    sfsc_Topic acknowledge_topic;
    int32_t expected_acknowledge_id;
    int32_t reply_id;
    pb_callback_t reply_payload;
} sfsc_Reply;

typedef struct _sfsc_ReplyPattern {
    int32_t reply_id;
    pb_callback_t reply_payload;
} sfsc_ReplyPattern;

typedef struct _sfsc_RequestOrAcknowledge_Acknowledge {
    int32_t acknowledge_id;
} sfsc_RequestOrAcknowledge_Acknowledge;

typedef struct _sfsc_RequestOrAcknowledge_Request {
    sfsc_Topic reply_topic;
    int32_t expected_reply_id;
    pb_callback_t request_payload;
} sfsc_RequestOrAcknowledge_Request;

typedef struct _sfsc_RequestPattern {
    pb_callback_t reply_topic;
    int32_t expected_reply_id;
    pb_callback_t request_payload;
} sfsc_RequestPattern;

typedef struct _sfsc_SfscId {
    char id[37];
} sfsc_SfscId;

typedef struct _sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags {
    sfsc_Topic output_topic;
    sfsc_MessageType output_message_type;
    bool unregistered;
} sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags;

typedef struct _sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings {
    int32_t timeout_ms;
    int32_t send_max_tries;
} sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings;

typedef struct _sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex {
    int64_t lowerBound;
    int64_t upperBound;
} sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex;

typedef struct _sfsc_HeartbeatMessage {
    sfsc_SfscId adapter_id;
} sfsc_HeartbeatMessage;

typedef struct _sfsc_Hello {
    sfsc_SfscId adapter_id;
    sfsc_Topic heartbeat_topic;
} sfsc_Hello;

typedef struct _sfsc_RequestOrAcknowledge {
    pb_callback_t cb_request_or_acknowledge;
    pb_size_t which_request_or_acknowledge;
    union {
        sfsc_RequestOrAcknowledge_Request request;
        sfsc_RequestOrAcknowledge_Acknowledge acknowledge;
    } request_or_acknowledge;
} sfsc_RequestOrAcknowledge;

typedef struct _sfsc_SfscServiceDescriptor_ServiceTags_ServerTags {
    sfsc_Topic input_topic;
    sfsc_MessageType input_message_type;
    sfsc_MessageType output_message_type;
    sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings ack_settings;
} sfsc_SfscServiceDescriptor_ServiceTags_ServerTags;

typedef struct _sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex {
    pb_callback_t varName;
    pb_size_t which_regex;
    union {
        sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex string_regex;
        sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex number_regex;
    } regex;
} sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex;

typedef struct _sfsc_Welcome {
    sfsc_SfscId core_id;
} sfsc_Welcome;

typedef struct _sfsc_SfscServiceDescriptor_ServiceTags {
    pb_callback_t cb_serviceTags;
    pb_size_t which_serviceTags;
    union {
        sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags publisher_tags;
        sfsc_SfscServiceDescriptor_ServiceTags_ServerTags server_tags;
    } serviceTags;
} sfsc_SfscServiceDescriptor_ServiceTags;

typedef struct _sfsc_SfscServiceDescriptor {
    sfsc_SfscId service_id;
    sfsc_SfscId adapter_id;
    sfsc_SfscId core_id;
    pb_callback_t service_name;
    pb_callback_t custom_tags;
    sfsc_SfscServiceDescriptor_ServiceTags service_tags;
} sfsc_SfscServiceDescriptor;

typedef struct _sfsc_ChannelFactoryReply {
    sfsc_SfscServiceDescriptor service_descriptor;
} sfsc_ChannelFactoryReply;

typedef struct _sfsc_CommandRequest {
    sfsc_SfscId adapterId;
    pb_size_t which_create_or_delete;
    union {
        sfsc_SfscServiceDescriptor create_request;
        sfsc_SfscServiceDescriptor delete_request;
    } create_or_delete;
} sfsc_CommandRequest;

typedef struct _sfsc_QueryReply {
    int64_t event_id;
    pb_callback_t cb_created_or_deleted_or_expired_or_future;
    pb_size_t which_created_or_deleted_or_expired_or_future;
    union {
        sfsc_SfscServiceDescriptor created;
        sfsc_SfscServiceDescriptor deleted;
        sfsc_QueryReply_Expired expired;
        sfsc_QueryReply_Future future;
    } created_or_deleted_or_expired_or_future;
} sfsc_QueryReply;

typedef struct _sfsc_RegistryEntry {
    sfsc_SfscId adapterId;
    sfsc_SfscId coreId;
    sfsc_SfscServiceDescriptor data;
} sfsc_RegistryEntry;


/* Initializer values for message structs */
#define sfsc_BootstrapMessage_init_default       {0, 0, 0, 0}
#define sfsc_CommandRequest_init_default         {sfsc_SfscId_init_default, 0, {sfsc_SfscServiceDescriptor_init_default}}
#define sfsc_CommandReply_init_default           {0}
#define sfsc_QueryRequest_init_default           {0}
#define sfsc_QueryReply_init_default             {0, {{NULL}, NULL}, 0, {sfsc_SfscServiceDescriptor_init_default}}
#define sfsc_QueryReply_Expired_init_default     {0}
#define sfsc_QueryReply_Future_init_default      {0}
#define sfsc_Hello_init_default                  {sfsc_SfscId_init_default, sfsc_Topic_init_default}
#define sfsc_Welcome_init_default                {sfsc_SfscId_init_default}
#define sfsc_HeartbeatMessage_init_default       {sfsc_SfscId_init_default}
#define sfsc_SfscServiceDescriptor_init_default  {sfsc_SfscId_init_default, sfsc_SfscId_init_default, sfsc_SfscId_init_default, {{NULL}, NULL}, {{NULL}, NULL}, sfsc_SfscServiceDescriptor_ServiceTags_init_default}
#define sfsc_SfscServiceDescriptor_CustomTagsEntry_init_default {{{NULL}, NULL}, {{NULL}, NULL}}
#define sfsc_SfscServiceDescriptor_ServiceTags_init_default {{{NULL}, NULL}, 0, {sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_init_default}}
#define sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_init_default {sfsc_Topic_init_default, sfsc_MessageType_init_default, 0}
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_init_default {sfsc_Topic_init_default, sfsc_MessageType_init_default, sfsc_MessageType_init_default, sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_init_default}
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_init_default {{{NULL}, NULL}, 0, {sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_init_default}}
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_init_default {{{NULL}, NULL}}
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex_init_default {0, 0}
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_init_default {0, 0}
#define sfsc_RequestOrAcknowledge_init_default   {{{NULL}, NULL}, 0, {sfsc_RequestOrAcknowledge_Request_init_default}}
#define sfsc_RequestOrAcknowledge_Request_init_default {sfsc_Topic_init_default, 0, {{NULL}, NULL}}
#define sfsc_RequestOrAcknowledge_Acknowledge_init_default {0}
#define sfsc_Reply_init_default                  {sfsc_Topic_init_default, 0, 0, {{NULL}, NULL}}
#define sfsc_ChannelFactoryRequest_init_default  {{{NULL}, NULL}}
#define sfsc_ChannelFactoryReply_init_default    {sfsc_SfscServiceDescriptor_init_default}
#define sfsc_SfscId_init_default                 {""}
#define sfsc_Topic_init_default                  {{{NULL}, NULL}}
#define sfsc_MessageType_init_default            {{{NULL}, NULL}}
#define sfsc_RequestPattern_init_default         {{{NULL}, NULL}, 0, {{NULL}, NULL}}
#define sfsc_ReplyPattern_init_default           {0, {{NULL}, NULL}}
#define sfsc_RegistryEntry_init_default          {sfsc_SfscId_init_default, sfsc_SfscId_init_default, sfsc_SfscServiceDescriptor_init_default}
#define sfsc_BootstrapMessage_init_zero          {0, 0, 0, 0}
#define sfsc_CommandRequest_init_zero            {sfsc_SfscId_init_zero, 0, {sfsc_SfscServiceDescriptor_init_zero}}
#define sfsc_CommandReply_init_zero              {0}
#define sfsc_QueryRequest_init_zero              {0}
#define sfsc_QueryReply_init_zero                {0, {{NULL}, NULL}, 0, {sfsc_SfscServiceDescriptor_init_zero}}
#define sfsc_QueryReply_Expired_init_zero        {0}
#define sfsc_QueryReply_Future_init_zero         {0}
#define sfsc_Hello_init_zero                     {sfsc_SfscId_init_zero, sfsc_Topic_init_zero}
#define sfsc_Welcome_init_zero                   {sfsc_SfscId_init_zero}
#define sfsc_HeartbeatMessage_init_zero          {sfsc_SfscId_init_zero}
#define sfsc_SfscServiceDescriptor_init_zero     {sfsc_SfscId_init_zero, sfsc_SfscId_init_zero, sfsc_SfscId_init_zero, {{NULL}, NULL}, {{NULL}, NULL}, sfsc_SfscServiceDescriptor_ServiceTags_init_zero}
#define sfsc_SfscServiceDescriptor_CustomTagsEntry_init_zero {{{NULL}, NULL}, {{NULL}, NULL}}
#define sfsc_SfscServiceDescriptor_ServiceTags_init_zero {{{NULL}, NULL}, 0, {sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_init_zero}}
#define sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_init_zero {sfsc_Topic_init_zero, sfsc_MessageType_init_zero, 0}
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_init_zero {sfsc_Topic_init_zero, sfsc_MessageType_init_zero, sfsc_MessageType_init_zero, sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_init_zero}
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_init_zero {{{NULL}, NULL}, 0, {sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_init_zero}}
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_init_zero {{{NULL}, NULL}}
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex_init_zero {0, 0}
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_init_zero {0, 0}
#define sfsc_RequestOrAcknowledge_init_zero      {{{NULL}, NULL}, 0, {sfsc_RequestOrAcknowledge_Request_init_zero}}
#define sfsc_RequestOrAcknowledge_Request_init_zero {sfsc_Topic_init_zero, 0, {{NULL}, NULL}}
#define sfsc_RequestOrAcknowledge_Acknowledge_init_zero {0}
#define sfsc_Reply_init_zero                     {sfsc_Topic_init_zero, 0, 0, {{NULL}, NULL}}
#define sfsc_ChannelFactoryRequest_init_zero     {{{NULL}, NULL}}
#define sfsc_ChannelFactoryReply_init_zero       {sfsc_SfscServiceDescriptor_init_zero}
#define sfsc_SfscId_init_zero                    {""}
#define sfsc_Topic_init_zero                     {{{NULL}, NULL}}
#define sfsc_MessageType_init_zero               {{{NULL}, NULL}}
#define sfsc_RequestPattern_init_zero            {{{NULL}, NULL}, 0, {{NULL}, NULL}}
#define sfsc_ReplyPattern_init_zero              {0, {{NULL}, NULL}}
#define sfsc_RegistryEntry_init_zero             {sfsc_SfscId_init_zero, sfsc_SfscId_init_zero, sfsc_SfscServiceDescriptor_init_zero}

/* Field tags (for use in manual encoding/decoding) */
#define sfsc_ChannelFactoryRequest_payload_tag   1
#define sfsc_MessageType_type_tag                1
#define sfsc_SfscServiceDescriptor_CustomTagsEntry_key_tag 1
#define sfsc_SfscServiceDescriptor_CustomTagsEntry_value_tag 2
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_regex_tag 1
#define sfsc_Topic_topic_tag                     1
#define sfsc_BootstrapMessage_core_control_pub_tcp_port_tag 1
#define sfsc_BootstrapMessage_core_control_sub_tcp_port_tag 2
#define sfsc_BootstrapMessage_core_data_pub_tcp_port_tag 3
#define sfsc_BootstrapMessage_core_data_sub_tcp_port_tag 4
#define sfsc_QueryReply_Expired_oldest_valid_event_id_tag 1
#define sfsc_QueryReply_Future_newest_valid_event_id_tag 1
#define sfsc_QueryRequest_event_id_tag           1
#define sfsc_Reply_acknowledge_topic_tag         1
#define sfsc_Reply_expected_acknowledge_id_tag   2
#define sfsc_Reply_reply_id_tag                  3
#define sfsc_Reply_reply_payload_tag             4
#define sfsc_ReplyPattern_reply_id_tag           1
#define sfsc_ReplyPattern_reply_payload_tag      2
#define sfsc_RequestOrAcknowledge_Acknowledge_acknowledge_id_tag 1
#define sfsc_RequestOrAcknowledge_Request_reply_topic_tag 1
#define sfsc_RequestOrAcknowledge_Request_expected_reply_id_tag 2
#define sfsc_RequestOrAcknowledge_Request_request_payload_tag 3
#define sfsc_RequestPattern_reply_topic_tag      1
#define sfsc_RequestPattern_expected_reply_id_tag 2
#define sfsc_RequestPattern_request_payload_tag  3
#define sfsc_SfscId_id_tag                       1
#define sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_output_topic_tag 1
#define sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_output_message_type_tag 2
#define sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_unregistered_tag 3
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_timeout_ms_tag 1
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_send_max_tries_tag 2
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex_lowerBound_tag 1
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex_upperBound_tag 2
#define sfsc_HeartbeatMessage_adapter_id_tag     1
#define sfsc_Hello_adapter_id_tag                1
#define sfsc_Hello_heartbeat_topic_tag           2
#define sfsc_RequestOrAcknowledge_request_tag    1
#define sfsc_RequestOrAcknowledge_acknowledge_tag 2
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_input_topic_tag 1
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_input_message_type_tag 2
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_output_message_type_tag 3
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_ack_settings_tag 5
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_varName_tag 1
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_string_regex_tag 2
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_number_regex_tag 3
#define sfsc_Welcome_core_id_tag                 1
#define sfsc_SfscServiceDescriptor_ServiceTags_publisher_tags_tag 1
#define sfsc_SfscServiceDescriptor_ServiceTags_server_tags_tag 2
#define sfsc_SfscServiceDescriptor_service_id_tag 1
#define sfsc_SfscServiceDescriptor_adapter_id_tag 2
#define sfsc_SfscServiceDescriptor_core_id_tag   3
#define sfsc_SfscServiceDescriptor_service_name_tag 4
#define sfsc_SfscServiceDescriptor_custom_tags_tag 5
#define sfsc_SfscServiceDescriptor_service_tags_tag 6
#define sfsc_ChannelFactoryReply_service_descriptor_tag 1
#define sfsc_CommandRequest_adapterId_tag        1
#define sfsc_CommandRequest_create_request_tag   2
#define sfsc_CommandRequest_delete_request_tag   3
#define sfsc_QueryReply_event_id_tag             1
#define sfsc_QueryReply_created_tag              2
#define sfsc_QueryReply_deleted_tag              3
#define sfsc_QueryReply_expired_tag              4
#define sfsc_QueryReply_future_tag               5
#define sfsc_RegistryEntry_adapterId_tag         1
#define sfsc_RegistryEntry_coreId_tag            2
#define sfsc_RegistryEntry_data_tag              3

/* Struct field encoding specification for nanopb */
#define sfsc_BootstrapMessage_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    core_control_pub_tcp_port,   1) \
X(a, STATIC,   SINGULAR, INT32,    core_control_sub_tcp_port,   2) \
X(a, STATIC,   SINGULAR, INT32,    core_data_pub_tcp_port,   3) \
X(a, STATIC,   SINGULAR, INT32,    core_data_sub_tcp_port,   4)
#define sfsc_BootstrapMessage_CALLBACK NULL
#define sfsc_BootstrapMessage_DEFAULT NULL

#define sfsc_CommandRequest_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, MESSAGE,  adapterId,         1) \
X(a, STATIC,   ONEOF,    MESSAGE,  (create_or_delete,create_request,create_or_delete.create_request),   2) \
X(a, STATIC,   ONEOF,    MESSAGE,  (create_or_delete,delete_request,create_or_delete.delete_request),   3)
#define sfsc_CommandRequest_CALLBACK NULL
#define sfsc_CommandRequest_DEFAULT NULL
#define sfsc_CommandRequest_adapterId_MSGTYPE sfsc_SfscId
#define sfsc_CommandRequest_create_or_delete_create_request_MSGTYPE sfsc_SfscServiceDescriptor
#define sfsc_CommandRequest_create_or_delete_delete_request_MSGTYPE sfsc_SfscServiceDescriptor

#define sfsc_CommandReply_FIELDLIST(X, a) \

#define sfsc_CommandReply_CALLBACK NULL
#define sfsc_CommandReply_DEFAULT NULL

#define sfsc_QueryRequest_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT64,    event_id,          1)
#define sfsc_QueryRequest_CALLBACK NULL
#define sfsc_QueryRequest_DEFAULT NULL

#define sfsc_QueryReply_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT64,    event_id,          1) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (created_or_deleted_or_expired_or_future,created,created_or_deleted_or_expired_or_future.created),   2) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (created_or_deleted_or_expired_or_future,deleted,created_or_deleted_or_expired_or_future.deleted),   3) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (created_or_deleted_or_expired_or_future,expired,created_or_deleted_or_expired_or_future.expired),   4) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (created_or_deleted_or_expired_or_future,future,created_or_deleted_or_expired_or_future.future),   5)
#define sfsc_QueryReply_CALLBACK NULL
#define sfsc_QueryReply_DEFAULT NULL
#define sfsc_QueryReply_created_or_deleted_or_expired_or_future_created_MSGTYPE sfsc_SfscServiceDescriptor
#define sfsc_QueryReply_created_or_deleted_or_expired_or_future_deleted_MSGTYPE sfsc_SfscServiceDescriptor
#define sfsc_QueryReply_created_or_deleted_or_expired_or_future_expired_MSGTYPE sfsc_QueryReply_Expired
#define sfsc_QueryReply_created_or_deleted_or_expired_or_future_future_MSGTYPE sfsc_QueryReply_Future

#define sfsc_QueryReply_Expired_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT64,    oldest_valid_event_id,   1)
#define sfsc_QueryReply_Expired_CALLBACK NULL
#define sfsc_QueryReply_Expired_DEFAULT NULL

#define sfsc_QueryReply_Future_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT64,    newest_valid_event_id,   1)
#define sfsc_QueryReply_Future_CALLBACK NULL
#define sfsc_QueryReply_Future_DEFAULT NULL

#define sfsc_Hello_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, MESSAGE,  adapter_id,        1) \
X(a, STATIC,   SINGULAR, MESSAGE,  heartbeat_topic,   2)
#define sfsc_Hello_CALLBACK NULL
#define sfsc_Hello_DEFAULT NULL
#define sfsc_Hello_adapter_id_MSGTYPE sfsc_SfscId
#define sfsc_Hello_heartbeat_topic_MSGTYPE sfsc_Topic

#define sfsc_Welcome_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, MESSAGE,  core_id,           1)
#define sfsc_Welcome_CALLBACK NULL
#define sfsc_Welcome_DEFAULT NULL
#define sfsc_Welcome_core_id_MSGTYPE sfsc_SfscId

#define sfsc_HeartbeatMessage_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, MESSAGE,  adapter_id,        1)
#define sfsc_HeartbeatMessage_CALLBACK NULL
#define sfsc_HeartbeatMessage_DEFAULT NULL
#define sfsc_HeartbeatMessage_adapter_id_MSGTYPE sfsc_SfscId

#define sfsc_SfscServiceDescriptor_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, MESSAGE,  service_id,        1) \
X(a, STATIC,   SINGULAR, MESSAGE,  adapter_id,        2) \
X(a, STATIC,   SINGULAR, MESSAGE,  core_id,           3) \
X(a, CALLBACK, SINGULAR, STRING,   service_name,      4) \
X(a, CALLBACK, REPEATED, MESSAGE,  custom_tags,       5) \
X(a, STATIC,   SINGULAR, MESSAGE,  service_tags,      6)
#define sfsc_SfscServiceDescriptor_CALLBACK pb_default_field_callback
#define sfsc_SfscServiceDescriptor_DEFAULT NULL
#define sfsc_SfscServiceDescriptor_service_id_MSGTYPE sfsc_SfscId
#define sfsc_SfscServiceDescriptor_adapter_id_MSGTYPE sfsc_SfscId
#define sfsc_SfscServiceDescriptor_core_id_MSGTYPE sfsc_SfscId
#define sfsc_SfscServiceDescriptor_custom_tags_MSGTYPE sfsc_SfscServiceDescriptor_CustomTagsEntry
#define sfsc_SfscServiceDescriptor_service_tags_MSGTYPE sfsc_SfscServiceDescriptor_ServiceTags

#define sfsc_SfscServiceDescriptor_CustomTagsEntry_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   key,               1) \
X(a, CALLBACK, SINGULAR, BYTES,    value,             2)
#define sfsc_SfscServiceDescriptor_CustomTagsEntry_CALLBACK pb_default_field_callback
#define sfsc_SfscServiceDescriptor_CustomTagsEntry_DEFAULT NULL

#define sfsc_SfscServiceDescriptor_ServiceTags_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (serviceTags,publisher_tags,serviceTags.publisher_tags),   1) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (serviceTags,server_tags,serviceTags.server_tags),   2)
#define sfsc_SfscServiceDescriptor_ServiceTags_CALLBACK NULL
#define sfsc_SfscServiceDescriptor_ServiceTags_DEFAULT NULL
#define sfsc_SfscServiceDescriptor_ServiceTags_serviceTags_publisher_tags_MSGTYPE sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags
#define sfsc_SfscServiceDescriptor_ServiceTags_serviceTags_server_tags_MSGTYPE sfsc_SfscServiceDescriptor_ServiceTags_ServerTags

#define sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, MESSAGE,  output_topic,      1) \
X(a, STATIC,   SINGULAR, MESSAGE,  output_message_type,   2) \
X(a, STATIC,   SINGULAR, BOOL,     unregistered,      3)
#define sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_CALLBACK NULL
#define sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_DEFAULT NULL
#define sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_output_topic_MSGTYPE sfsc_Topic
#define sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_output_message_type_MSGTYPE sfsc_MessageType

#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, MESSAGE,  input_topic,       1) \
X(a, STATIC,   SINGULAR, MESSAGE,  input_message_type,   2) \
X(a, STATIC,   SINGULAR, MESSAGE,  output_message_type,   3) \
X(a, STATIC,   SINGULAR, MESSAGE,  ack_settings,      5)
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_CALLBACK NULL
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_DEFAULT NULL
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_input_topic_MSGTYPE sfsc_Topic
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_input_message_type_MSGTYPE sfsc_MessageType
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_output_message_type_MSGTYPE sfsc_MessageType
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_ack_settings_MSGTYPE sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings

#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   varName,           1) \
X(a, STATIC,   ONEOF,    MESSAGE,  (regex,string_regex,regex.string_regex),   2) \
X(a, STATIC,   ONEOF,    MESSAGE,  (regex,number_regex,regex.number_regex),   3)
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_CALLBACK pb_default_field_callback
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_DEFAULT NULL
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_regex_string_regex_MSGTYPE sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_regex_number_regex_MSGTYPE sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex

#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   regex,             1)
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_CALLBACK pb_default_field_callback
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_DEFAULT NULL

#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, SINT64,   lowerBound,        1) \
X(a, STATIC,   SINGULAR, SINT64,   upperBound,        2)
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex_CALLBACK NULL
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex_DEFAULT NULL

#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    timeout_ms,        1) \
X(a, STATIC,   SINGULAR, INT32,    send_max_tries,    2)
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_CALLBACK NULL
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_DEFAULT NULL

#define sfsc_RequestOrAcknowledge_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (request_or_acknowledge,request,request_or_acknowledge.request),   1) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (request_or_acknowledge,acknowledge,request_or_acknowledge.acknowledge),   2)
#define sfsc_RequestOrAcknowledge_CALLBACK NULL
#define sfsc_RequestOrAcknowledge_DEFAULT NULL
#define sfsc_RequestOrAcknowledge_request_or_acknowledge_request_MSGTYPE sfsc_RequestOrAcknowledge_Request
#define sfsc_RequestOrAcknowledge_request_or_acknowledge_acknowledge_MSGTYPE sfsc_RequestOrAcknowledge_Acknowledge

#define sfsc_RequestOrAcknowledge_Request_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, MESSAGE,  reply_topic,       1) \
X(a, STATIC,   SINGULAR, INT32,    expected_reply_id,   2) \
X(a, CALLBACK, SINGULAR, BYTES,    request_payload,   3)
#define sfsc_RequestOrAcknowledge_Request_CALLBACK pb_default_field_callback
#define sfsc_RequestOrAcknowledge_Request_DEFAULT NULL
#define sfsc_RequestOrAcknowledge_Request_reply_topic_MSGTYPE sfsc_Topic

#define sfsc_RequestOrAcknowledge_Acknowledge_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    acknowledge_id,    1)
#define sfsc_RequestOrAcknowledge_Acknowledge_CALLBACK NULL
#define sfsc_RequestOrAcknowledge_Acknowledge_DEFAULT NULL

#define sfsc_Reply_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, MESSAGE,  acknowledge_topic,   1) \
X(a, STATIC,   SINGULAR, INT32,    expected_acknowledge_id,   2) \
X(a, STATIC,   SINGULAR, INT32,    reply_id,          3) \
X(a, CALLBACK, SINGULAR, BYTES,    reply_payload,     4)
#define sfsc_Reply_CALLBACK pb_default_field_callback
#define sfsc_Reply_DEFAULT NULL
#define sfsc_Reply_acknowledge_topic_MSGTYPE sfsc_Topic

#define sfsc_ChannelFactoryRequest_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, BYTES,    payload,           1)
#define sfsc_ChannelFactoryRequest_CALLBACK pb_default_field_callback
#define sfsc_ChannelFactoryRequest_DEFAULT NULL

#define sfsc_ChannelFactoryReply_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, MESSAGE,  service_descriptor,   1)
#define sfsc_ChannelFactoryReply_CALLBACK NULL
#define sfsc_ChannelFactoryReply_DEFAULT NULL
#define sfsc_ChannelFactoryReply_service_descriptor_MSGTYPE sfsc_SfscServiceDescriptor

#define sfsc_SfscId_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, STRING,   id,                1)
#define sfsc_SfscId_CALLBACK NULL
#define sfsc_SfscId_DEFAULT NULL

#define sfsc_Topic_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, BYTES,    topic,             1)
#define sfsc_Topic_CALLBACK pb_default_field_callback
#define sfsc_Topic_DEFAULT NULL

#define sfsc_MessageType_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, BYTES,    type,              1)
#define sfsc_MessageType_CALLBACK pb_default_field_callback
#define sfsc_MessageType_DEFAULT NULL

#define sfsc_RequestPattern_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, BYTES,    reply_topic,       1) \
X(a, STATIC,   SINGULAR, INT32,    expected_reply_id,   2) \
X(a, CALLBACK, SINGULAR, BYTES,    request_payload,   3)
#define sfsc_RequestPattern_CALLBACK pb_default_field_callback
#define sfsc_RequestPattern_DEFAULT NULL

#define sfsc_ReplyPattern_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    reply_id,          1) \
X(a, CALLBACK, SINGULAR, BYTES,    reply_payload,     2)
#define sfsc_ReplyPattern_CALLBACK pb_default_field_callback
#define sfsc_ReplyPattern_DEFAULT NULL

#define sfsc_RegistryEntry_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, MESSAGE,  adapterId,         1) \
X(a, STATIC,   SINGULAR, MESSAGE,  coreId,            2) \
X(a, STATIC,   SINGULAR, MESSAGE,  data,              3)
#define sfsc_RegistryEntry_CALLBACK NULL
#define sfsc_RegistryEntry_DEFAULT NULL
#define sfsc_RegistryEntry_adapterId_MSGTYPE sfsc_SfscId
#define sfsc_RegistryEntry_coreId_MSGTYPE sfsc_SfscId
#define sfsc_RegistryEntry_data_MSGTYPE sfsc_SfscServiceDescriptor

extern const pb_msgdesc_t sfsc_BootstrapMessage_msg;
extern const pb_msgdesc_t sfsc_CommandRequest_msg;
extern const pb_msgdesc_t sfsc_CommandReply_msg;
extern const pb_msgdesc_t sfsc_QueryRequest_msg;
extern const pb_msgdesc_t sfsc_QueryReply_msg;
extern const pb_msgdesc_t sfsc_QueryReply_Expired_msg;
extern const pb_msgdesc_t sfsc_QueryReply_Future_msg;
extern const pb_msgdesc_t sfsc_Hello_msg;
extern const pb_msgdesc_t sfsc_Welcome_msg;
extern const pb_msgdesc_t sfsc_HeartbeatMessage_msg;
extern const pb_msgdesc_t sfsc_SfscServiceDescriptor_msg;
extern const pb_msgdesc_t sfsc_SfscServiceDescriptor_CustomTagsEntry_msg;
extern const pb_msgdesc_t sfsc_SfscServiceDescriptor_ServiceTags_msg;
extern const pb_msgdesc_t sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_msg;
extern const pb_msgdesc_t sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_msg;
extern const pb_msgdesc_t sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_msg;
extern const pb_msgdesc_t sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_msg;
extern const pb_msgdesc_t sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex_msg;
extern const pb_msgdesc_t sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_msg;
extern const pb_msgdesc_t sfsc_RequestOrAcknowledge_msg;
extern const pb_msgdesc_t sfsc_RequestOrAcknowledge_Request_msg;
extern const pb_msgdesc_t sfsc_RequestOrAcknowledge_Acknowledge_msg;
extern const pb_msgdesc_t sfsc_Reply_msg;
extern const pb_msgdesc_t sfsc_ChannelFactoryRequest_msg;
extern const pb_msgdesc_t sfsc_ChannelFactoryReply_msg;
extern const pb_msgdesc_t sfsc_SfscId_msg;
extern const pb_msgdesc_t sfsc_Topic_msg;
extern const pb_msgdesc_t sfsc_MessageType_msg;
extern const pb_msgdesc_t sfsc_RequestPattern_msg;
extern const pb_msgdesc_t sfsc_ReplyPattern_msg;
extern const pb_msgdesc_t sfsc_RegistryEntry_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define sfsc_BootstrapMessage_fields &sfsc_BootstrapMessage_msg
#define sfsc_CommandRequest_fields &sfsc_CommandRequest_msg
#define sfsc_CommandReply_fields &sfsc_CommandReply_msg
#define sfsc_QueryRequest_fields &sfsc_QueryRequest_msg
#define sfsc_QueryReply_fields &sfsc_QueryReply_msg
#define sfsc_QueryReply_Expired_fields &sfsc_QueryReply_Expired_msg
#define sfsc_QueryReply_Future_fields &sfsc_QueryReply_Future_msg
#define sfsc_Hello_fields &sfsc_Hello_msg
#define sfsc_Welcome_fields &sfsc_Welcome_msg
#define sfsc_HeartbeatMessage_fields &sfsc_HeartbeatMessage_msg
#define sfsc_SfscServiceDescriptor_fields &sfsc_SfscServiceDescriptor_msg
#define sfsc_SfscServiceDescriptor_CustomTagsEntry_fields &sfsc_SfscServiceDescriptor_CustomTagsEntry_msg
#define sfsc_SfscServiceDescriptor_ServiceTags_fields &sfsc_SfscServiceDescriptor_ServiceTags_msg
#define sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_fields &sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_msg
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_fields &sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_msg
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_fields &sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_msg
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_fields &sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_msg
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex_fields &sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex_msg
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_fields &sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_msg
#define sfsc_RequestOrAcknowledge_fields &sfsc_RequestOrAcknowledge_msg
#define sfsc_RequestOrAcknowledge_Request_fields &sfsc_RequestOrAcknowledge_Request_msg
#define sfsc_RequestOrAcknowledge_Acknowledge_fields &sfsc_RequestOrAcknowledge_Acknowledge_msg
#define sfsc_Reply_fields &sfsc_Reply_msg
#define sfsc_ChannelFactoryRequest_fields &sfsc_ChannelFactoryRequest_msg
#define sfsc_ChannelFactoryReply_fields &sfsc_ChannelFactoryReply_msg
#define sfsc_SfscId_fields &sfsc_SfscId_msg
#define sfsc_Topic_fields &sfsc_Topic_msg
#define sfsc_MessageType_fields &sfsc_MessageType_msg
#define sfsc_RequestPattern_fields &sfsc_RequestPattern_msg
#define sfsc_ReplyPattern_fields &sfsc_ReplyPattern_msg
#define sfsc_RegistryEntry_fields &sfsc_RegistryEntry_msg

/* Maximum encoded size of messages (where known) */
#define sfsc_BootstrapMessage_size               44
/* sfsc_CommandRequest_size depends on runtime parameters */
#define sfsc_CommandReply_size                   0
#define sfsc_QueryRequest_size                   11
/* sfsc_QueryReply_size depends on runtime parameters */
#define sfsc_QueryReply_Expired_size             11
#define sfsc_QueryReply_Future_size              11
/* sfsc_Hello_size depends on runtime parameters */
#define sfsc_Welcome_size                        40
#define sfsc_HeartbeatMessage_size               40
/* sfsc_SfscServiceDescriptor_size depends on runtime parameters */
/* sfsc_SfscServiceDescriptor_CustomTagsEntry_size depends on runtime parameters */
/* sfsc_SfscServiceDescriptor_ServiceTags_size depends on runtime parameters */
/* sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags_size depends on runtime parameters */
/* sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_size depends on runtime parameters */
/* sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_size depends on runtime parameters */
/* sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_StringRegex_size depends on runtime parameters */
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_RegexDefinition_VarRegex_NumberRegex_size 22
#define sfsc_SfscServiceDescriptor_ServiceTags_ServerTags_AckSettings_size 22
/* sfsc_RequestOrAcknowledge_size depends on runtime parameters */
/* sfsc_RequestOrAcknowledge_Request_size depends on runtime parameters */
#define sfsc_RequestOrAcknowledge_Acknowledge_size 11
/* sfsc_Reply_size depends on runtime parameters */
/* sfsc_ChannelFactoryRequest_size depends on runtime parameters */
/* sfsc_ChannelFactoryReply_size depends on runtime parameters */
#define sfsc_SfscId_size                         38
/* sfsc_Topic_size depends on runtime parameters */
/* sfsc_MessageType_size depends on runtime parameters */
/* sfsc_RequestPattern_size depends on runtime parameters */
/* sfsc_ReplyPattern_size depends on runtime parameters */
/* sfsc_RegistryEntry_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
