/*
 * SFSC_Platform.c
 *
 *  Created on: 30.06.2020
 *      Author: Eric
 */
#ifdef ESP_IDF
#include "../sfsc/platform/sfsc_platform.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_idf_platform.h"

void random_bytes(sfsc_uint8 *buffer, sfsc_size size)
{
	esp_fill_random(buffer, size);
}

sfsc_uint64 time_ms()
{
	return (sfsc_uint64)(esp_timer_get_time() / 1000);
}
#ifdef MULTI_TASK
#ifdef SEM_BASED
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define FOREVER 0xffffffff
#define SEM_COUNT 32
SemaphoreHandle_t semaphores[SEM_COUNT] = {NULL};
void *addresses[SEM_COUNT] = {NULL};
sfsc_uint8 sem_takes[SEM_COUNT] = {0};
SemaphoreHandle_t find_handle;

void lock_init()
{
	find_handle = xSemaphoreCreateBinary();
	xSemaphoreGive(find_handle);
	for (sfsc_uint8 i = 0; i < SEM_COUNT; i++)
	{
		semaphores[i] = xSemaphoreCreateRecursiveMutex();
	}
}

static sfsc_uint8 find_for(void *address)
{
	xSemaphoreTake(find_handle, FOREVER);
	sfsc_uint8 i = 0;
	for (; i < SEM_COUNT; i++)
	{
		if (addresses[i] == address)
		{
			break;
		}
		else if (addresses[i] == NULL)
		{
			addresses[i] = address;
			break;
		}
	}
	xSemaphoreGive(find_handle);
	return i;
}

void lock_resource(void *address)
{
	sfsc_uint8 i = find_for(address);
	xSemaphoreTakeRecursive(semaphores[i], FOREVER);
	sem_takes[i]++;
}
void unlock_resource(void *address)
{
	sfsc_uint8 i = find_for(address);
	while (sem_takes[i] > 0)
	{
		xSemaphoreGiveRecursive(semaphores[i]);
		sem_takes[i]--;
	}
}
#else
#include "freertos/FreeRTOS.h"
static portMUX_TYPE spinlock;
void lock_init()
{
	vPortCPUInitializeMutex(&spinlock);
}
void lock_resource(void *address)
{
	vTaskEnterCritical(&spinlock);
}
void unlock_resource(void *address)
{
	vTaskExitCritical(&spinlock);
}
#endif
#else
void lock_resource(void *address)
{
}
void unlock_resource(void *address) {}
#endif
#endif
