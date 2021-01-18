/*
 * sfsc_subscriber.c
 *
 *  Created on: 27.10.2020
 *      Author: Eric
 */

#include "sfsc_subscriber.h"

#include "../zmtp/zmtp.h"
#include "sfsc_utils.h"

sfsc_int8 sfsc_internal_register_subscriber(_sfsc_adapter_data* adapter,
                                            sfsc_subscriber* subscribers[],
                                            sfsc_subscriber* subscriber) {
    sfsc_size i = 0;
    for (; i < MAX_SUBSCRIBERS; i++) {
        if (subscribers[i] == NULL) {
            break;
        }
    }
    if (i < MAX_SUBSCRIBERS) {
        sfsc_int8 op_result = write_subscription_message(
            &(adapter->data_sub), subscriber->topic.content,
            subscriber->topic.length, 1);
        subscribers[i] = subscriber;
        return op_result;
    } else {
        return E_NO_FREE_SLOT;
    }
}

sfsc_int8 sfsc_internal_unregister_subscriber(_sfsc_adapter_data* adapter,
                                              sfsc_subscriber* subscribers[],
                                              sfsc_subscriber* subscriber) {
    sfsc_size i = 0;
    for (; i < MAX_SUBSCRIBERS; i++) {
        if (subscribers[i] == subscriber) {
            break;
        }
    }
    if (i < MAX_SUBSCRIBERS) {
        sfsc_int8 op_result = write_subscription_message(
            &(adapter->data_sub), subscriber->topic.content,
            subscriber->topic.length, 0);
        subscribers[i] = NULL;
        return op_result;
    } else {
        return ZMTP_OK;  // NOT FOUND IN THE FIRST PLACE
    }
}

sfsc_int8 user_task_data_check_subscribers(
    sfsc_adapter* forward_pointer, sfsc_subscriber* subscribers[],
    sfsc_uint8* next_topic, sfsc_size topic_len, sfsc_uint8* next_payload,
    sfsc_size payload_len, sfsc_uint8* consumed, sfsc_bool* b_auto_advance) {
    for (sfsc_size i = 0; i < MAX_SUBSCRIBERS; i++) {
        sfsc_subscriber* subscriber = subscribers[i];
        if (subscribers[i] != NULL && subscriber->topic.length == topic_len &&
            b_bytes_equal(subscriber->topic.content, next_topic, topic_len)) {
                sfsc_buffer next_payload_buffer= {next_payload, payload_len};
            subscriber->on_data(forward_pointer, subscriber,
                               next_payload_buffer, b_auto_advance);
            *consumed = 1;
            break;
        }
    }
    return ZMTP_OK;
}
