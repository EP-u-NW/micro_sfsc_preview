#ifdef POSIX
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../../sfsc/platform/sfsc_sockets.h"
#ifdef ENABLE_PRINTS
#include "../../examples/shared/console.h"
#endif

static const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

sfsc_uint32 convert_to_ip(const char *host) {
    sfsc_uint8 bytes[4] = {0};
    sfsc_uint8 tuple = sizeof(bytes) - 1;
    sfsc_uint8 multiplier = 1;
    for (sfsc_size i = strlen(host) - 1; 0 <= i; i--) {
        if (host[i] == '.') {
            tuple--;
            multiplier = 1;
        } else {
            sfsc_uint8 value = 0;
            for (; value < sizeof(digits); value++) {
                if (digits[value] == host[i]) {
                    break;
                }
            }
            if (value >= sizeof(digits)) {
                return 0xFFFFFFFF;
            }
            bytes[tuple] += (multiplier * value);
            if (multiplier == 1) {
                multiplier = 10;
            } else if (multiplier == 10) {
                multiplier = 100;
            }
        }
    }
    return *((sfsc_uint32 *)bytes);
}

sfsc_int16 socket_connect(const char *host, sfsc_uint16 port) {
    int sock = 0;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#ifdef ENABLE_PRINTS
        console_println_char("Socket creation error");
#endif
        return -101;
    }

    // We need to use TCP delay because of SFSC issue 55 (see on github)
    // Once that is fixed, NODELAY should be removed!
    int flag = 1;
    int result = setsockopt(sock,          /* socket affected */
                            IPPROTO_TCP,   /* set option at TCP level */
                            TCP_NODELAY,   /* name of option */
                            (char *)&flag, /* the cast is historical cruft */
                            sizeof(int));  /* length of option value */

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    // sfsc_uint32 ip = convert_to_ip(host);
    // console_print_char("Found ip ");
    // print_byte_array((sfsc_uint8 *)&ip, 4);
    // *((sfsc_uint32 *)&addr.sin_addr) = ip;
    memcpy((void*)(&addr.sin_addr), (const void*)host, strlen(host));
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
#ifdef ENABLE_PRINTS
        console_print_char("Connection to ");
        console_print_char(host);
        console_print_char(":");
        console_print_uint16(port);
        console_println_char(" failed");
#endif
        return -102;
    } else {
#ifdef ENABLE_PRINTS
        console_print_char("Gave out socket id ");
        console_print_int32(sock);
        console_print_char(" for connection to ");
        console_println_char(host);
#endif
    }
    return (sfsc_int16)sock;
}

sfsc_int8 socket_write(sfsc_int16 socket, const sfsc_uint8 *buf,
                       sfsc_size size) {
    send(socket, buf, size, 0);
    return SOCKET_OK;
}

sfsc_socket_size socket_read(sfsc_int16 socket, sfsc_uint8 *buf,
                             sfsc_socket_size size) {
    sfsc_size bytes_available;
    ioctl(socket, FIONREAD, &bytes_available);
    if (bytes_available > 0) {
        sfsc_socket_size result = (sfsc_socket_size)recv(socket, buf, size, 0);
        return result;
    } else {
        return (sfsc_socket_size)bytes_available;
    }
}

sfsc_int8 socket_flush(sfsc_int16 socket) {
    // No-op
    return SOCKET_OK;
}

#endif
