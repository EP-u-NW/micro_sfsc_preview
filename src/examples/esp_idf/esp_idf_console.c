#ifdef ESP_IDF
#ifdef ENABLE_PRINTS
#include <stdlib.h>
#include <stdio.h>
#include "../shared/console.h"
void console_begin() {}

#ifdef USE_BUFFERED_CONSOLE

#include "../debug/buffered_console.h"
void console_flush() {
#ifdef __cplusplus
	std::cout.write((const char*)global_print_buffer,global_print_buffer_index);
#else
	fwrite(global_print_buffer_index,global_print_buffer_index,len,stdin);
#endif
	global_print_buffer_index=0;
}
#else
void console_flush() {}

void console_println_char(const char value[]) {
	console_print_char(value);
	console_println();
}
void console_println_uint8(sfsc_uint8 value) {
	console_print_uint8(value);
	console_println();
}
void console_println_uint16(sfsc_uint16 value) {
	console_print_uint16(value);
	console_println();
}
void console_println_uint32(sfsc_uint32 value) {
	console_print_uint32(value);
	console_println();
}
void console_println_int8(sfsc_int8 value) {
	console_print_int8(value);
	console_println();
}
void console_println_int16(sfsc_int16 value) {
	console_print_int16(value);
	console_println();
}
void console_println_int32(sfsc_int32 value) {
	console_print_int32(value);
	console_println();
}

void console_println() {
#ifdef __cplusplus
	std::cout << "\r\n";
#else
	printf("\r\n");
#endif
}
void console_print_char(const char value[]) {
#ifdef __cplusplus
	std::cout << value;
#else
	printf("%s",value);
#endif
}
void console_print_uint8(sfsc_uint8 value) {
#ifdef __cplusplus
	std::cout << (sfsc_uint16)value;
#else
	printf("%u",value);
#endif
}
void console_print_uint16(sfsc_uint16 value) {
#ifdef __cplusplus
	std::cout << value;
#else
	printf("%u",value);
#endif
}
void console_print_uint32(sfsc_uint32 value) {
#ifdef __cplusplus
	std::cout << value;
#else
	printf("%u",value);
#endif
}
void console_print_int8(sfsc_int8 value) {
#ifdef __cplusplus
	std::cout << (sfsc_int16)value;
#else
	printf("%i",value);
#endif
}
void console_print_int16(sfsc_int16 value) {
#ifdef __cplusplus
	std::cout << value;
#else
	printf("%i",value);
#endif
}
void console_print_int32(sfsc_int32 value) {
#ifdef __cplusplus
	std::cout << value;
#else
	printf("%i",value);
#endif
}
void console_write(const sfsc_uint8* value,sfsc_size len) {
#ifdef __cplusplus
	std::cout.write((const char*)value,len);
#else
	fwrite(value,1,len,stdout);
#endif
}
#endif
#endif
#endif