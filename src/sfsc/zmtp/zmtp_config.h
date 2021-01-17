/*
 * zmtp_config.h
 *
 *  Created on: 29.10.2020
 *      Author: Eric
 */

#ifndef SRC_ZMTP_ZMTP_CONFIG_H_
#define SRC_ZMTP_ZMTP_CONFIG_H_


#ifdef __cplusplus
extern "C" {
#endif

#define ZMTP_VERSION_MAJOR 3
#define ZMTP_VERSION_MINOR 0

#define ZMTP_IN_BUFFER_SIZE 512 //if using curve at least 265 (INITIATE(257) + DECRPYTION_OFFSET(8))
#define ZMTP_METADATA_BUFFER_SIZE 32

#define NO_CURVE

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* SRC_ZMTP_ZMTP_CONFIG_H_ */
