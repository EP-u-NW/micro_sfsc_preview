#ifdef POSIX
#include <stdlib.h>
#include <sys/time.h>
#include "../../sfsc/platform/sfsc_platform.h";

static sfsc_bool seeded = 0;
void random_bytes(sfsc_uint8 *buffer, sfsc_size size)
{
	if (!seeded)
	{
		sfsc_uint64 seed = time_ms();
		srand(seed);
		seeded=true;
	}
	for (sfsc_size i = 0; i < size; i++)
	{
		buffer[i] = rand();
	}
}


void lock_socket(sfsc_uint8* lock_mem){}
void unlock_socket(sfsc_uint8* lock_mem){}

sfsc_uint64 time_ms()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return (sfsc_uint64)(t.tv_sec*1000.0+t.tv_usec/1000.0);
}
#endif
