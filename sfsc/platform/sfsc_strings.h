/*
 * sfsc_strings.h
 *
 *  Created on: 05.12.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_PLATFORM_SFSC_STRINGS_H_
#define SRC_SFSC_PLATFORM_SFSC_STRINGS_H_

#include "../sfsc_adapter/sfsc_adapter_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAS_STRING_H

#include <string.h>

#else

void* memcpy(void* to, const void* from,sfsc_size size);
sfsc_size strlen(const char* buf);
void* memset(void* block,int c,sfsc_size size);

#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_PLATFORM_SFSC_STRINGS_H_ */
