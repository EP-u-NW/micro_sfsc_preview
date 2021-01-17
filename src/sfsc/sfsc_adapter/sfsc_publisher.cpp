/*
 * sfsc_publisher.c
 *
 *  Created on: 27.10.2020
 *      Author: Eric
 */
#include "sfsc_publisher.h"

#include "sfsc_utils.h"

static sfsc_buffer topic_or_autogen_plain(sfsc_buffer topic,
                                          sfsc_uint8* service_id) {
    if (topic.length == 0) {
        sfsc_buffer buf = {service_id, UUID_LEN};
        return buf;
    } else {
        return topic;
    }
}

static sfsc_buffer topic_or_autogen(sfsc_publisher* service) {
    return topic_or_autogen_plain(service->topic,
                                  (sfsc_uint8*)service->service_id.id);
}

sfsc_int8 user_task_publisher(sfsc_adapter* forward_pointer,
                              sfsc_publisher* publishers[]) {
    for (sfsc_size i = 0; i < MAX_PUBLISHERS; i++) {
        sfsc_publisher* p = publishers[i];
        if (p != NULL) {
            if (p->current_subscribers != p->last_subscribers) {
                if (p->on_subscription_change != NULL) {
                    p->on_subscription_change(forward_pointer, p,
                                              p->last_subscribers,
                                              p->current_subscribers);
                }
                p->last_subscribers = p->current_subscribers;
            }
        }
    }
    return ZMTP_OK;
}

sfsc_int8 system_task_publisher(_sfsc_adapter_data* adapter,
                                sfsc_publisher* publishers[]) {
    sfsc_uint8* message = get_message(&(adapter->data_pub));
    if (adapter->data_pub.is_message == 1 &&
        adapter->data_pub.last_message == 1) {
        for (sfsc_size i = 0; i < MAX_PUBLISHERS; i++) {
            sfsc_publisher* p = publishers[i];
            if (p != NULL) {
                sfsc_buffer p_topic = topic_or_autogen(p);
                if (p_topic.length == adapter->data_pub.buffer_index - 1 &&
                    b_bytes_equal(p_topic.content, message + 1,
                                  p_topic.length)) {
                    if (message[0] == 1) {  // SUB
                        p->current_subscribers++;
                    } else if (p->current_subscribers > 0) {  // UNSUB
                        p->current_subscribers--;
                    }
                    break;
                }
            }
        }
    }
    return ZMTP_OK;
}

sfsc_int8 sfsc_internal_register_publisher_unregistered(sfsc_publisher* publishers[],
    sfsc_publisher* publisher, sfsc_size* i) {
    *i = 0;
    for (; *i < MAX_PUBLISHERS; (*i)++) {
        if (publishers[*i] == NULL) {
            break;
        }
    }
    if (*i < MAX_PUBLISHERS) {
        publisher->unregistered = 1;
        publishers[*i] = publisher;
        random_uuid_if_needed((sfsc_uint8*)publisher->service_id.id);
        return ZMTP_OK;
    } else {
        return E_NO_FREE_SLOT;
    }
}

sfsc_int8 sfsc_internal_register_publisher(
    sfsc_adapter_stats* stats, _sfsc_adapter_data* adapter,
    _sfsc_commands* commands, sfsc_publisher* publishers[],
    sfsc_publisher* publisher, sfsc_buffer name, sfsc_buffer custom_tags,
    sfsc_buffer output_message_type, sfsc_command_callback* callback) {
		sfsc_size i=0;
    sfsc_int8 op_result = sfsc_internal_register_publisher_unregistered(publishers, publisher,&i);
    if (op_result == ZMTP_OK) {
        publisher->unregistered = 0;
        sfsc_CommandRequest request = sfsc_CommandRequest_init_default;
        sfsc_SfscServiceDescriptor* create =
            &request.create_or_delete.create_request;

        create->service_name.funcs.encode = b_encode_buffer_for_pb;
        create->service_name.arg = &name;
        create->custom_tags.funcs.encode = b_encode_buffer_for_pb;
        create->custom_tags.arg = &custom_tags;

        create->service_tags.which_serviceTags =
            sfsc_SfscServiceDescriptor_ServiceTags_publisher_tags_tag;

        sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags* publisher_tags =
            &create->service_tags.serviceTags.publisher_tags;
        publisher_tags->unregistered = 0;
        publisher_tags->output_message_type.type.funcs.encode =
            b_encode_buffer_for_pb;
        publisher_tags->output_message_type.type.arg = &output_message_type;
        publisher_tags->output_topic.topic.funcs.encode =
            b_encode_buffer_for_pb;
        sfsc_buffer output_topic = topic_or_autogen(publisher);
        publisher_tags->output_topic.topic.arg = &output_topic;

        sfsc_publisher_or_server as_service =
            sfsc_publisher_or_server_INIT_DEFAULT;
        as_service.is_server = 0;
        as_service.service.publisher = publisher;

        op_result = execute_command(stats, adapter, commands, as_service,
                                     &request, 1, callback);
        if (op_result == ZMTP_OK) {
            return ZMTP_OK;
        } else {
            publishers[i] = NULL;
            return op_result;
        }
    } else {
        return op_result;
    }
}
sfsc_int8 sfsc_internal_unregister_publisher(sfsc_adapter_stats* stats,
                                             _sfsc_adapter_data* adapter,
                                             _sfsc_commands* commands,
                                             sfsc_publisher* publishers[],
                                             sfsc_publisher* publisher,
                                             sfsc_command_callback* callback) {
    sfsc_size i = 0;
    for (; i < MAX_PUBLISHERS; i++) {
        if (publishers[i] == publisher) {
            break;
        }
    }
    if (i < MAX_PUBLISHERS) {
        publishers[i] = NULL;
        if (publisher->unregistered) {
            return ZMTP_OK;
        } else {
            sfsc_CommandRequest request = sfsc_CommandRequest_init_default;
            sfsc_publisher_or_server as_service =
                sfsc_publisher_or_server_INIT_DEFAULT;
            as_service.is_server = 0;
            as_service.service.publisher = publisher;
            return execute_command(stats, adapter, commands, as_service,
                                   &request, 0, callback);
        }
    } else {
        return ZMTP_OK;
    }
}

sfsc_int8 sfsc_internal_publish(_sfsc_adapter_data* adapter,
                                sfsc_publisher* publisher,
                                sfsc_buffer payload) {
    if (publisher->current_subscribers > 0) {
        sfsc_buffer topic = topic_or_autogen(publisher);
        write_message(&(adapter->data_pub), topic.content, topic.length, 0);
        write_message(&(adapter->data_pub), payload.content, payload.length, 1);
    }
    return ZMTP_OK;
}

void configure_descriptor_for_channel_answer(
    sfsc_SfscServiceDescriptor* descriptor, sfsc_channel_answer* channel_answer,
    sfsc_buffer* topic_buffer) {
    sfsc_copy((sfsc_uint8*)descriptor->service_id.id,
              channel_answer->service_id, UUID_LEN);
    sfsc_copy((sfsc_uint8*)descriptor->adapter_id.id,
              channel_answer->adapter_id, UUID_LEN);
    sfsc_copy((sfsc_uint8*)descriptor->core_id.id, channel_answer->core_id,
              UUID_LEN);

    descriptor->service_name.funcs.encode = b_encode_buffer_for_pb;
    descriptor->service_name.arg = &channel_answer->name;
    descriptor->custom_tags.funcs.encode = b_encode_buffer_for_pb;
    descriptor->custom_tags.arg = &channel_answer->custom_tags;

    descriptor->service_tags.which_serviceTags =
        sfsc_SfscServiceDescriptor_ServiceTags_publisher_tags_tag;

    sfsc_SfscServiceDescriptor_ServiceTags_PublisherTags* publisher_tags =
        &descriptor->service_tags.serviceTags.publisher_tags;
    publisher_tags->unregistered = channel_answer->unregistered;
    publisher_tags->output_message_type.type.funcs.encode =
        b_encode_buffer_for_pb;
    publisher_tags->output_message_type.type.arg =
        &channel_answer->output_message_type;

    publisher_tags->output_topic.topic.funcs.encode = b_encode_buffer_for_pb;
    *topic_buffer = topic_or_autogen_plain(
        channel_answer->publisher_output_topic, channel_answer->service_id);
    publisher_tags->output_topic.topic.arg = topic_buffer;
}
