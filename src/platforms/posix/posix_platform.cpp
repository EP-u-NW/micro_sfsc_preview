#ifdef POSIX
#include "posix_config.h"
#include <stdlib.h>
#ifdef __cplusplus
#define HIGH_TIME_RES
#endif
#ifdef HIGH_TIME_RES
#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#else
#include <sys/time.h>
#endif
#ifndef HAS_DEV_RANDOM
#include <stdio.h>
#endif
#include "../sfsc/platform/sfsc_platform.h";

sfsc_uint8 seeded = 0;
void random_bytes(sfsc_uint8 *buffer, sfsc_size size)
{
	if (seeded == 0)
	{
		sfsc_uint64 seed;
#ifndef HAS_DEV_RANDOM
		seed = time_ms();
#else
		FILE *handle = fopen("/dev/urandom", "r");
		fread((sfsc_uint8 *)&seed, 1, 8, handle);
		fclose(handle);
#endif
		srand(seed);
	}
	for (sfsc_size i = 0; i < size; i++)
	{
		buffer[i] = rand();
	}
}


void lock_resource(void* address){}
void unlock_resource(void* address){}


#ifdef HIGH_TIME_RES
std::chrono::high_resolution_clock::time_point programm_start = std::chrono::high_resolution_clock::now();
#else
sfsc_uint64 s = 0;
#endif
sfsc_uint64 time_ms()
{
#ifdef HIGH_TIME_RES
	std::chrono::duration<double, std::milli> time_span = std::chrono::high_resolution_clock::now() - programm_start;
	return (sfsc_uint64)time_span.count();
#else
	if (s == 0)
	{
		struct timeval x;
		gettimeofday(&x, NULL);
		s = x.tv_sec;
	}
	struct timeval t;
	gettimeofday(&t, NULL);
	return (sfsc_uint64)((t.tv_sec-s)*1000.0+t.tv_usec/1000.0);
#endif
}
#endif
