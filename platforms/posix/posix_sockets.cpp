#ifdef POSIX
#include "posix_config.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "../debug/console.h"
#include "../sfsc/platform/sfsc_sockets.h"

const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

sfsc_uint32 convert_to_ip(const char *host)
{
    sfsc_uint8 bytes[4] = {0};
    sfsc_uint8 tuple = sizeof(bytes) - 1;
    sfsc_uint8 multiplier = 1;
    for (sfsc_size i = strlen(host) - 1; 0 <= i; i--)
    {
        if (host[i] == '.')
        {
            tuple--;
            multiplier = 1;
        }
        else
        {
            sfsc_uint8 value = 0;
            for (; value < sizeof(digits); value++)
            {
                if (digits[value] == host[i])
                {
                    break;
                }
            }
            if (value >= sizeof(digits))
            {
                return 0xFFFFFFFF;
            }
            bytes[tuple] += (multiplier * value);
            if (multiplier == 1)
            {
                multiplier = 10;
            }
            else if (multiplier == 10)
            {
                multiplier = 100;
            }
        }
    }
    return *((sfsc_uint32 *)bytes);
}

sfsc_int16 socket_connect(const char *host, sfsc_uint16 port)
{
    int sock = 0;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        console_println_char("Socket creation error");
        return -1;
    }

    int flag = 1;
    int result = setsockopt(sock,          /* socket affected */
                            IPPROTO_TCP,   /* set option at TCP level */
                            TCP_NODELAY,   /* name of option */
                            (char *)&flag, /* the cast is historical cruft */
                            sizeof(int));  /* length of option value */

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    sfsc_uint32 ip = convert_to_ip(host);
    console_print_char("Found ip ");
    print_byte_array((sfsc_uint8 *)&ip, 4);
    *((sfsc_uint32 *)&addr.sin_addr) = ip;
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        console_print_char("Connection to ");
        console_print_char(host);
        console_print_char(":");
        console_print_uint16(port);
        console_println_char(" failed");
        return -1;
    }
    else
    {
        console_print_char("Gave out socket id ");
        console_print_int32(sock);
        console_print_char(" for connection to ");
        console_println_char(host);
    }
    return (sfsc_int16)sock;
}

sfsc_int16 socket_write(sfsc_int16 socket, const sfsc_uint8 *buf, sfsc_int16 size)
{
    return (sfsc_int16)send(socket, buf, size, 0);
}
sfsc_uint16 socket_available(sfsc_int16 socket)
{
    size_t bytes_available;
    ioctl(socket, FIONREAD, &bytes_available);
    return (sfsc_uint16)bytes_available;
}
sfsc_int16 socket_read(sfsc_int16 socket, sfsc_uint8 *buf, sfsc_int16 size)
{
    return (sfsc_int16)recv(socket, buf, size, 0);
}
void socket_flush(sfsc_int16 socket)
{
    //Not available
}
void socket_stop(sfsc_int16 socket)
{
    close(socket);
}
sfsc_uint8 socket_connected(sfsc_int16 socket)
{
    return 1;
}
#endif
