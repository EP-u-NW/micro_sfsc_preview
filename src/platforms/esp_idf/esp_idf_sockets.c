
#ifdef ESP_IDF
#include <string.h>
#include <sys/param.h>

#include "../../sfsc/platform/sfsc_sockets.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"

#ifdef ENABLE_PRINTS
#include "../../examples/shared/console.h"
#endif

sfsc_int16 socket_connect(const char *host, sfsc_uint16 port) {
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(host);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0) {
#ifdef ENABLE_PRINTS
        console_print_char("Unable to create socket: errno ");
        console_println_int32(errno);
#endif
        return -1;
    }
#ifdef ENABLE_PRINTS
    console_print_char("Socket created, connecting to ");
    console_print_char(host);
    console_print_char(":");
    console_print_int16(port);
    console_print_char(" with socket handle ");
    console_println_int32(sock);
#endif

    int err = connect(sock, (struct sockaddr *)&dest_addr,
                      sizeof(struct sockaddr_in6));
    if (err != 0) {
#ifdef ENABLE_PRINTS
        console_print_char("Socket unable to connect: errno ");
        console_println_int32(errno);
#endif
        return -1;
    }
#ifdef ENABLE_PRINTS
    console_println_char("Successfully connected");
#endif
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
    ESP_ERROR_CHECK(ioctl(socket, FIONREAD, &bytes_available));
    if (bytes_available > 0) {
        sfsc_socket_size result = (sfsc_socket_size)recv(socket, buf, size, 0);
#ifdef ENABLE_PRINTS
        if (errno != 0) {
            console_print_char("Errno: ");
            console_println_int32(errno);
        }
#endif
        return result;
    } else {
        return (sfsc_socket_size)bytes_available;
    }
}

sfsc_int8 socket_flush(sfsc_int16 socket) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
    return SOCKET_OK;
}
#endif
