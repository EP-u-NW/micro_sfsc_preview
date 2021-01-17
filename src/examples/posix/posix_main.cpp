
#ifdef POSIX
#include "../../sfsc/platform/sfsc_platform.h"
#include "../../sfsc/platform/sfsc_types.h"
#include "../shared/console.h"
#include "../../sfsc/platform/sfsc_sockets.h"
int main()
{
    sfsc_uint8 number[4];
    random_bytes(number,4);
    console_print_char("Here is a random number: ");
    console_println_uint32(*(sfsc_uint32*)number);
    sfsc_int16 handle = socket_connect("127.0.0.1", 54321);
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
        console_print_uint32((sfsc_uint32)(time_ms() - *((sfsc_uint64*)in_buff)));
        console_println_char("ms");
        console_print_char("Calls without data: ");
        console_println_uint16(calls_without_data);
    }
    else
    {
        console_print_char("Socket error: ");
        console_println_int16(handle);
    }
}

#endif