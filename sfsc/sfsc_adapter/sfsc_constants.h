/*
 * sfsc_constants.h
 *
 *  Created on: 28.10.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_ADAPTER_SFSC_CONSTANTS_H_
#define SRC_SFSC_ADAPTER_SFSC_CONSTANTS_H_

#include "../platform/sfsc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _REPLY_ID_UNINTERESTED 0
#define _REPLY_ID_UNUSED 0
#define _ACK_ID_UNUSED 0

#define _HEARTBEAT_TOPIC_LEN 9
extern const sfsc_uint8 _heartbeat_topic[_HEARTBEAT_TOPIC_LEN];

#define _BOOTSTRAP_TOPIC_LEN 9
#define _HELLO_TOPIC_LEN 7
#define _WELCOME_TOPIC_LEN _HELLO_TOPIC_LEN

extern const sfsc_uint8 _bootstrap_topic[_BOOTSTRAP_TOPIC_LEN];
extern const sfsc_uint8 _hello_topic[_HELLO_TOPIC_LEN];
extern const sfsc_uint8 *_welcome_topic;

#define _QUERY_TOPIC_LEN 14
extern const sfsc_uint8 _query_topic[_QUERY_TOPIC_LEN];

#define _DATA_REQUEST_REPLY_TOPIC_LEN 13
extern const sfsc_uint8 _data_request_reply_topic[_DATA_REQUEST_REPLY_TOPIC_LEN];

#define _DATA_ACK_TOPIC_LEN 11
extern const sfsc_uint8 _data_ack_topic[_DATA_ACK_TOPIC_LEN];

#define _COMMAND_TOPIC_LEN 16
extern const sfsc_uint8 _command_topic[_COMMAND_TOPIC_LEN];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_CONSTANTS_H_ */
