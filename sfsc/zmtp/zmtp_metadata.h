/*
 * zmtp_metadata.h
 *
 *  Created on: 15.07.2020
 *      Author: Eric
 */

#ifndef ZMTP_METADATA_H_
#define ZMTP_METADATA_H_

#include "zmtp.h"

#ifdef __cplusplus
extern "C" {
#endif

void transkript_metadata(zmtp_socket* socket, sfsc_size offset);
sfsc_int8 write_metadata(zmtp_socket* socket, sfsc_size len);
void write_metadata_to_buffer(zmtp_socket* socket, sfsc_size len, sfsc_uint8* target);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ZMTP_METADATA_H_ */
