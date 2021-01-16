#ifndef SFSC_TYPES_H
#define SFSC_TYPES_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif

#define sfsc_uint8 uint8_t
#define sfsc_int8 int8_t
#define sfsc_uint16 uint16_t
#define sfsc_int16 int16_t
#define sfsc_uint32 uint32_t
#define sfsc_int32 int32_t
#define sfsc_uint64 uint64_t
#define sfsc_int64 int64_t
#define sfsc_bool bool
#define sfsc_size size_t

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
