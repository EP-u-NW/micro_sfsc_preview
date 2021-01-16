#ifndef SRC_SFSC_ADAPTER_SFSC_ADAPTER_CONFIG_H_
#define SRC_SFSC_ADAPTER_SFSC_ADAPTER_CONFIG_H_

#include "../zmtp/zmtp_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HAS_STRING_H

//Constants
#define HEARTBEAT_SEND_RATE_MS 400
#define HEARTBEAT_DEADLINE_OUTGOING_MS HEARTBEAT_SEND_RATE_MS * 4
#define HEARTBEAT_DEADLINE_INCOMING_MS 4000

#define MAX_PUBLISHERS 6
#define MAX_SUBSCRIBERS 12
#define MAX_PENDING_ACKS 6
#define MAX_SERVERS 6
#define MAX_SIMULTANIOUS_COMMANDS 6
#define MAX_SIMULTANIOUS_REQUESTS 6

#define REGISTRY_BUFFER_SIZE 512 //big enough to hold one service
#if ZMTP_IN_BUFFER_SIZE < REGISTRY_BUFFER_SIZE
#error "That does not make sense at all: You can't buffer a service you can't even receive!"
#endif

#define MAX_DELETED_MEMORY 32

#define USER_RING_SIZE 1024*5

#define REPLAYS_PER_TASK 0 //0 for as many as available

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_ADAPTER_CONFIG_H_ */
