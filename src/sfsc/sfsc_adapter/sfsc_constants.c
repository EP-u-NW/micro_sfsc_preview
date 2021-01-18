/*
 * sfsc_constants.cpp
 *
 *  Created on: 28.10.2020
 *      Author: Eric
 */

#include "sfsc_constants.h"

const sfsc_uint8 _heartbeat_topic[_HEARTBEAT_TOPIC_LEN] = { 'H', 'E', 'A', 'R',
		'T', 'B', 'E', 'A', 'T' };

const sfsc_uint8 _bootstrap_topic[_BOOTSTRAP_TOPIC_LEN] = { 'B', 'O', 'O', 'T',
		'S', 'T', 'R', 'A', 'P' };

const sfsc_uint8 _hello_topic[_HELLO_TOPIC_LEN] = { 'S', 'E', 'S', 'S', 'I',
		'O', 'N', };

const sfsc_uint8 *_welcome_topic = _hello_topic;

const sfsc_uint8 _query_topic[_QUERY_TOPIC_LEN] = { 'R', 'E', 'G', 'I', 'S',
		'T', 'R', 'Y', '_', 'Q', 'U', 'E', 'R', 'Y' };

const sfsc_uint8 _data_request_reply_topic[_DATA_REQUEST_REPLY_TOPIC_LEN] = { 'G', 'E', 'N', 'E', 'R', 'A',
		'L', '_', 'R', 'E', 'P', 'L', 'Y' };

const sfsc_uint8 _data_ack_topic[_DATA_ACK_TOPIC_LEN] = { 'G', 'E', 'N', 'E', 'R', 'A',
		'L', '_', 'A', 'C', 'K' };

const sfsc_uint8 _command_topic[_COMMAND_TOPIC_LEN] = { 'R', 'E', 'G', 'I', 'S',
		'T', 'R', 'Y', '_', 'C', 'O', 'M', 'M', 'A', 'N', 'D' };
