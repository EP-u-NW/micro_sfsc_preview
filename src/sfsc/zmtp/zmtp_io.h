/*
 * zmtp_io.h
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#ifndef SRC_ZMTP_ZMTP_IO_H_
#define SRC_ZMTP_ZMTP_IO_H_

#include "zmtp.h"

#ifdef __cplusplus
extern "C" {
#endif


#define _FLAG_COMMAND 0
#define _FLAG_MESSAGE 1
#define _FLAG_LAST_MESSAGE 2

sfsc_int8 write_length_header(zmtp_socket* socket, sfsc_int64 length, sfsc_uint8 type);
void reset(zmtp_socket* socket);
sfsc_int8 read_next(zmtp_socket* socket);
sfsc_int8 zmtp_write(zmtp_socket* socket,const sfsc_uint8* msg,sfsc_size msg_len);
sfsc_uint8 read_expected(zmtp_socket* socket);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_ZMTP_ZMTP_IO_H_ */
