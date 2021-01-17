#ifdef POSIX
#include <unistd.h>
#include <fcntl.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/tcp.h> //For no-delay on posix
#endif

#include "../../sfsc/platform/sfsc_sockets.h"
#ifdef ENABLE_PRINTS
#include "../../examples/shared/console.h"
#endif

static fd_set select_set;
static struct timeval instant = {0};
static bool inited = 0;
sfsc_int16 socket_connect(const char *host, sfsc_uint16 port)
{
    if (!inited)
    {
        inited = 1;
        FD_ZERO(&select_set);
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(0x202, &wsaData) != 0)
        {
#ifdef ENABLE_PRINTS
            console_println_char("WSAStartup failure!");
#endif
            return -104;
        }
#endif
    }
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
#ifdef ENABLE_PRINTS
        console_println_char("Socket creation error: ");
        console_print_int32((sfsc_int32)sock);
#ifdef _WIN32
        console_print_char("/");
        console_println_int32((sfsc_int32)WSAGetLastError());
#else
        console_println();
#endif
#endif
        return -101;
    }

    // We need to use TCP delay because of SFSC issue 55 (see on github)
    // Once that is fixed, NODELAY should be removed!
    int flag = 1;
    setsockopt(sock,          /* socket affected */
               IPPROTO_TCP,   /* set option at TCP level */
               TCP_NODELAY,   /* name of option */
               (char *)&flag, /* the cast is historical cruft */
               sizeof(int));  /* length of option value */
#ifndef _WIN32
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);
#endif
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    int lookup_result = 0;
#ifdef _WIN32
    sfsc_uint32 address = (sfsc_uint32)inet_addr(host);
    if (address != INADDR_NONE)
    {
        memcpy(&addr.sin_addr, (sfsc_uint8 *)&address, 4);
        lookup_result = 1;
    }
#else
    lookup_result = inet_pton(AF_INET, host, &addr.sin_addr);
#endif
    if (lookup_result == 1)
    {
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
#ifdef ENABLE_PRINTS
            console_print_char("Connection to ");
            console_print_char(host);
            console_print_char(":");
            console_print_uint16(port);
            console_println_char(" failed");
#endif
            return -102;
        }
        else
        {
#ifdef ENABLE_PRINTS
            console_print_char("Gave out socket id ");
            console_print_int32(sock);
            console_print_char(" for connection to ");
            console_println_char(host);
#endif
        }
        return (sfsc_int16)sock;
    }
    else
    {
#ifdef ENABLE_PRINTS
        console_println_char("Error getting ip from host");
#endif
        return -103;
    }
}

sfsc_int8 socket_write(sfsc_int16 socket, const sfsc_uint8 *buf,
                       sfsc_size size)
{
#ifdef _WIN32
    send(socket, (const char *)buf, size, 0);
#else
    send(socket, buf, size, 0);
#endif
    return SOCKET_OK;
}

sfsc_socket_size socket_read(sfsc_int16 socket, sfsc_uint8 *buf,
                             sfsc_socket_size size)
{
    FD_SET(socket, &select_set);
    select(socket + 1, &select_set, NULL, NULL, &instant);
    if (FD_ISSET(socket, &select_set))
    {
#ifdef _WIN32
        sfsc_socket_size result = (sfsc_socket_size)recv(socket, (char *)buf, size, 0);
#else
        sfsc_socket_size result = (sfsc_socket_size)recv(socket, buf, size, 0);
#endif
        return result;
    }
    else
    {
        return 0;
    }
}

sfsc_int8 socket_flush(sfsc_int16 socket)
{
    // No-op
    return SOCKET_OK;
}

#endif
