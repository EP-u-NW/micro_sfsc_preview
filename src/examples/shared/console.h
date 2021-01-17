/*
 * console.h
 *
 *  Created on: 31.10.2020
 *      Author: Eric
 */

#ifndef SRC_DEBUG_CONSOLE_H_
#define SRC_DEBUG_CONSOLE_H_
#ifdef ENABLE_PRINTS
#include "../../sfsc/sfsc_adapter/sfsc_adapter.h"
#include "../../sfsc/platform/sfsc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void print_byte_array(const sfsc_uint8 array[], sfsc_int32 size);

void print_relative_service_descriptor(
		relative_sfsc_service_descriptor* descriptor, sfsc_uint8* offset,
		sfsc_size len);

void console_flush();
void console_begin();
void console_println();
void console_print_char(const char value[]);
void console_println_char(const char value[]);
void console_print_uint8(sfsc_uint8 value);
void console_println_uint8(sfsc_uint8 value);
void console_print_uint16(sfsc_uint16 value);
void console_println_uint16(sfsc_uint16 value);
void console_print_uint32(sfsc_uint32 value);
void console_println_uint32(sfsc_uint32 value);
void console_print_int8(sfsc_int8 value);
void console_println_int8(sfsc_int8 value);
void console_print_int16(sfsc_int16 value);
void console_println_int16(sfsc_int16 value);
void console_print_int32(sfsc_int32 value);
void console_println_int32(sfsc_int32 value);
void console_write(const sfsc_uint8* value,sfsc_size len);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
#endif /* SRC_DEBUG_CONSOLE_H_ */
