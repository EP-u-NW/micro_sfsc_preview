/*
 * SFSC_Platform.c
 *
 *  Created on: 30.06.2020
 *      Author: Eric
 */
#ifdef ESP_IDF
#include "../../sfsc/platform/sfsc_platform.h"
#include "esp_system.h"
#include "esp_timer.h"

void random_bytes(sfsc_uint8 *buffer, sfsc_size size)
{
	esp_fill_random(buffer, size);
}

sfsc_uint64 time_ms()
{
	return (sfsc_uint64)(esp_timer_get_time() / 1000);
}
void lock_socket(sfsc_uint8* lock_mem){}
void unlock_socket(sfsc_uint8* lock_mem){}
#endif
