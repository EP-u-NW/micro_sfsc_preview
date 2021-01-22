/*
 * SFSC_Sockets.h
 *
 *  Created on: 20.07.2020
 *      Author: Eric
 */

#ifndef SFSC_SOCKETS_H_
#define SFSC_SOCKETS_H_

#include "sfsc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SOCKET_OK 0
/**
 * Usually, one would use the unsigned sfsc_size type to deal with sizes.
 * But this type needs to be able to return negative values to
 * indicate errors, so it needs to be signed.
 */
#define sfsc_socket_size signed int

/**
 * @brief Connects a host and a port via tcp, and returns a socket handle.
 *
 * The port parameter is a normal tcp port.
 *
 * The format of the host parameter is actually up to you: This function
 * recieves as host whatever you passed as address parameter to the
 * start_session or start_session_bootstraped function.
 *
 * The returned socket handle will be used to identify the socket and
 * perform operations on it. The numerical value of the handle must be
 * greater or equal than SOCKET_OK. If you return a value that is
 * smaller then SOCKET_OK (so, negative) this call will be treated as
 * failed, and the returned value will be seen as an error code, which
 * is then delivered upwards the stack to the function that called this
 * function. It is recommended to use error codes in the range of -100 to -126.
 *
 * This funciton should block programm execution until the socket is
 * operational, meaning that it can be read and written to.
 *
 * @param host The host to connect to, in a format defined by you
 * @param port A tcp port to connect to
 * @return sfsc_int16 A socket handle, or an error code
 */
sfsc_int16 socket_connect(const char *host, sfsc_uint16 port);
/**
 * @brief Writes size bytes from buf to socket in a non-blocking way.
 *
 * This call should be treated as success, only if it writes exactly size bytes
 * to the socket.
 *
 * This function must not block programm execution!
 *
 * If this call is succesfull, it should return SOCKET_OK. If you return a value
 * that is smaller then SOCKET_OK (so, negative) this call will be treated as
 * failed, and the returned value will be seen as an error code, which
 * is then delivered upwards the stack to the function that called this
 * function. It is recommended to use error codes in the range of -100 to -126.
 *
 * @param socket The socket to write to
 * @param buf The data to write
 * @param size The amount of data to write
 * @return sfsc_int8 SOCKET_OK on success, or an error code
 */
sfsc_int8 socket_write(sfsc_int16 socket, const sfsc_uint8 *buf,
                       sfsc_size size);
/**
 * @brief Reads up to size bytes from the socket into buf without blocking, and
 * returns the actually read amount.
 *
 * If there are no bytes available to read, this function MUST NOT block
 * programm execution, but instead return 0. If an End-Of-File (EOF) or other
 * error is detected, a negative value (usually -1) should be returned.
 * In any other case, the amount of data actually read should be returned,
 * which might be lower then the requested size.
 *
 * This function must not block programm execution!
 *
 * @param socket The socket to read from
 * @param buf The buffer to read into
 * @param size The maximum bytes to read
 * @return sfsc_socket_size The amount of data actually read (0 if none), or a
 * negative number to indicate an error
 */
sfsc_socket_size socket_read(sfsc_int16 socket, sfsc_uint8 *buf,
                             sfsc_socket_size size);
/**
 * @brief Ensures that all data previously written to a socket are written to
 * the network.
 *
 * Some network stacks will buffer data before writing them to the network, this
 * function should force write the buffered data.
 *
 * If this call is succesfull, it should return SOCKET_OK. If you return a value
 * other that SOCKET_OK this call will be treated as failed, and the returned
 * value will be seen as an error code, which is then delivered upwards the
 * stack to the function that called this function. It is recommended to use
 * error codes in the range of -100 to -126.
 *
 * @param socket The socket to flush
 * @return sfsc_int8 SOCKET_OK on success or an error code
 */
sfsc_int8 socket_flush(sfsc_int16 socket);

/**
 * @brief Releases a socket.
 *
 * This function is called if the socket is not longer used by an adapter and
 * not longer needed. You should perform cleanup here to avoide resource
 * leak. For some platforms (e.g. where your programm is ever-running, or when
 * you know that you will never call release_session on an adapter), it may be
 * reasonable to provide a no-op implementation.
 *
 * If this call is succesfull, it should return SOCKET_OK. If you return a value
 * other that SOCKET_OK this call will be treated as failed, and the returned
 * value will be seen as an error code, which is then delivered upwards the
 * stack to the function that called this function. It is recommended to use
 * error codes in the range of -100 to -126.
 *
 * There are no restrictions on the blocking behavior of this function.
 *
 * @param socket The socket to release
 * @return sfsc_int8 SOCKET_OK on success or an error code
 */
sfsc_int8 socket_release(sfsc_int16 socket);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SFSC_SOCKETS_H_ */
