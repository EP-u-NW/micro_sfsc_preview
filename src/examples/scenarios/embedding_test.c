#ifdef EMBEDDING_TEST
#include "../../sfsc/platform/sfsc_platform.h"
#include "../../sfsc/platform/sfsc_types.h"
#include "../shared/console.h"
#include "../../sfsc/platform/sfsc_sockets.h"

#define DELAYED_ECHO_SERVER_PORT 54321
static const char delayed_echo_server_host[] = "81.169.207.220";

#ifndef ENABLE_PRINTS
#error "The embedding test needs prints to let you know what is working!"
#endif

/**
 * @brief Tests random_bytes, time_ms, and networking.
 * 
 * The configured host is a delayed echo server, it will return whaterver data you send it
 * with a delay of 10 seconds. This delay is necessary to test if calls to socket_read behave nonblocking:
 * They should return if there are no bytes readable instead of waiting for readable bytes.
 * 
 * If the delayed echo server is down, this test will fail. You should then impelement your own delayed echo server.
 * 
 * @return int 0 if OK, -1 on failure
 */
int embedding_test()
{
    //First, generate a random number to test the rng
    sfsc_uint8 number[4];
    random_bytes(number, 4);
    console_print_char("Here is a random number: ");
    console_println_uint32(*(sfsc_uint32 *)number);
    //Test if the time_ms is working
    console_print_char("The time is ");
    console_println_uint32((sfsc_uint32)time_ms());
    //Test if networking works
    sfsc_int16 handle = socket_connect(delayed_echo_server_host, DELAYED_ECHO_SERVER_PORT);
    if (handle >= SOCKET_OK)
    {
        sfsc_uint16 calls_without_data = 0;
        sfsc_uint8 data[10] = {0, 0, 0, 0, 0, 0, 0, 0, 13, 10};
        sfsc_uint8 in_buff[10];
        sfsc_size in_read = 10;
        sfsc_uint64 *time = (sfsc_uint64 *)data;
        *time = time_ms();
        socket_write(handle, data, 10);
        sfsc_socket_size r = 0;
        while (in_read > 0)
        {
            r = socket_read(handle, in_buff, in_read);
            if (r == 0)
            {
                calls_without_data++;
            }
            else if (r < 0)
            {
                console_print_char("Error: ");
                console_println_int32(r);
                return -1;
            }
            else
            {
                in_read -= r;
            }
        }
        console_print_char("Done in ");
        console_print_uint32((sfsc_uint32)(time_ms() - *((sfsc_uint64 *)in_buff)));
        console_println_char("ms");
        console_print_char("Calls without data: ");
        console_println_uint16(calls_without_data);
        console_println_char("Calls without data should be > 100 to prove that nonblocking I/O is working");
        if (calls_without_data > 100)
        {
            console_println_char("OK: nonblocking I/O seems good");
        }
        else
        {
            console_println_char("ERROR: NONBLOCKING I/O SEEMS TO FAILE!");
            console_println_char("Note: Try running this example again, the first time it is used, it report false positives (due to warmup effects).");
            return -1;
        }
    }
    else
    {
        console_print_char("Socket error: ");
        console_println_int16(handle);
        return -1;
    }
    console_println_char("All tests passed!");
    return 0;
}

int example_main(){
    return embedding_test();
}

#endif