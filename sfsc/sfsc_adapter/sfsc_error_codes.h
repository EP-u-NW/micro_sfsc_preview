/** @file
 *  @brief SFSC related error codes.
 */


#ifndef SRC_SFSC_ADAPTER_SFSC_ERROR_CODES_H_
#define SRC_SFSC_ADAPTER_SFSC_ERROR_CODES_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Failed to decode an incoming sfsc message.
 *
 * This usually means that your communication peer sends malformed
 * messages, or at least that you are receiveing malformed messages.
 * There is not much you can do to prevent this error.
 *
 * This error is not recoverable, meaning that you start a new sfsc
 * session once you encounter it!
 *
 */
#define E_PROTO_DECODE -19
/**
 * @brief Failed to encode an outgoing sfsc message.
 *
 * This usually happens if you call a sfsc function with malformed
 * content. If you are working with sfsc_buffers, check that they
 * are valid for as long as the function you are using requires them.
 *
 * This error is not recoverable, meaning that you start a new sfsc
 * session once you encounter it!
 */
#define E_PROTO_ENCODE -20
/**
 * @brief This incdicates that you are calling the system_task function with a
 * too low frequency.
 *
 * This error happens if the system task can not send outgoing heartbeats in
 * time. The solution is not to rise the HEARTBEAT_SEND_RATE_MS, but to call the
 * system task more often.
 *
 * This error is not recoverable, meaning that you start a new sfsc
 * session once you encounter it!
 */
#define E_TOO_SLOW -21
/**
 * @brief The core failed to send a heartbeat in time.
 *
 * If this error happens, the core is most likely not longer available,
 * either because of a core or a network failure. You should treat this
 * session as expired and start a new one.
 *
 * This error is not recoverable, meaning that you start a new sfsc
 * session once you encounter it!
 */
#define E_HEARTBEAT_MISSING -22
/**
 * @brief Indicates that the system task was not able to write a message to the
 * user ring, so the message is dropped instead.
 *
 * To understand the relation between system task and user ring, see the readme,
 * section execution model.
 *
 * This is more a warning than a error, and the framework should continue
 * working.
 *
 * If you encounter message drops very frequent, try to rise USER_RING_SIZE (see
 * sfsc_adapter_config.h).
 *
 */
#define W_MESSAGE_DROP -23
/**
 * @brief Indicates that there is already a query process ongoing.
 * 
 * This error will usually occur if there is already a query process ongoing,
 * and you try to start another one. Keep in mind that you have to end a query
 * process manually and that it counts as running until you end id. See the
 * qery_services function for more details.
 * 
 * This error is recoverable, meaning that you can continue to use the sfsc
 * adapter.
 */
#define E_QUERY_IN_PROGRESS -24
/**
 * @brief Indicates that an adapter has no free memory slot of a certain type,
 * but a function would need one.
 *
 * This error is recoverable, meaning that you can continue to use the sfsc
 * adapter.
 */
#define E_NO_FREE_SLOT -25

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_ADAPTER_SFSC_ERROR_CODES_H_ */
