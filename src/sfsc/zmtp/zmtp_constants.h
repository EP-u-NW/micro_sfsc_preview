/*
 * zmtp_commands.h
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#ifndef SRC_ZMTP_ZMTP_CONSTANTS_H_
#define SRC_ZMTP_ZMTP_CONSTANTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IS_READY_COMMAND(socket,buffer) ((socket)->is_message == 0 && (buffer)[0] == 5 && (buffer)[1] == 'R' && (buffer)[2] == 'E' && (buffer)[3] == 'A' && (buffer)[4] == 'D' && (buffer)[5] == 'Y')
#define IS_ERROR_COMMAND(socket,buffer) ((socket)->is_message == 0 && (buffer)[0] == 5 && (buffer)[1] == 'E' && (buffer)[2] == 'R' && (buffer)[3] == 'R' && (buffer)[4] == 'O' && (buffer)[5] == 'R')
#define IS_INITIATE_COMMAND(socket,buffer) ((socket)->is_message == 0 && (buffer)[0] == 8 && (buffer)[1] == 'I' && (buffer)[2] == 'N' && (buffer)[3] == 'I' && (buffer)[4] == 'T' && (buffer)[5] == 'I'  && (buffer)[6] == 'A' && (buffer)[7] == 'T' && (buffer)[8] == 'E')
#define IS_HELLO_COMMAND(socket,buffer) ((socket)->is_message == 0 && (buffer)[0] == 5 && (buffer)[1] == 'H' && (buffer)[2] == 'E' && (buffer)[3] == 'L' && (buffer)[4] == 'L' && (buffer)[5] == 'O')
#define IS_WELCOME_COMMAND(socket,buffer) ((socket)->is_message == 0 && (buffer)[0] == 7 && (buffer)[1] == 'W' && (buffer)[2] == 'E' && (buffer)[3] == 'L' && (buffer)[4] == 'C' && (buffer)[5] == 'O'  && (buffer)[6] == 'M' && (buffer)[7] == 'E')

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_ZMTP_ZMTP_CONSTANTS_H_ */
