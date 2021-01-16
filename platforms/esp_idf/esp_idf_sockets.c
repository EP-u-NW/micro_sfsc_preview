
#ifdef ESP_IDF
#include "esp_idf_config.h"
#ifndef VOID_SOCKETS
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

#include "../debug/console.h"
#include "../sfsc/platform/sfsc_sockets.h"

sfsc_int16 socket_connect(const char *host, sfsc_uint16 port)
{

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(host);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0)
    {
        console_print_char("Unable to create socket: errno ");
        console_println_int32(errno);
        return -1;
    }
    console_print_char("Socket created, connecting to ");
    console_print_char(host);
    console_print_char(":");
    console_print_int16(port);
    console_print_char(" with socket handle ");
    console_println_int32(sock);
    
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
    if (err != 0)
    {
        console_print_char("Socket unable to connect: errno ");
        console_println_int32(errno);
        return -1;
    }
    console_println_char("Successfully connected");

    return (sfsc_int16)sock;
}

sfsc_int16 socket_write(sfsc_int16 socket, const sfsc_uint8 *buf, sfsc_int16 size)
{
    return (sfsc_int16)send(socket, buf, size, 0);
}
sfsc_uint16 socket_available(sfsc_int16 socket)
{
    size_t bytes_available;
    ESP_ERROR_CHECK(ioctl(socket, FIONREAD, &bytes_available));
    return (sfsc_uint16)bytes_available;
}
sfsc_int16 socket_read(sfsc_int16 socket, sfsc_uint8 *buf, sfsc_int16 size)
{
    sfsc_int16 result = (sfsc_int16)recv(socket, buf, size, 0);
    /*if(errno!=0){
        console_print_char("Errno: ");
        console_println_int32(errno);
    }*/
    return result;
}
void socket_flush(sfsc_int16 socket)
{
    vTaskDelay(100 / portTICK_PERIOD_MS);
}
void socket_stop(sfsc_int16 socket)
{
    close(socket);
}
sfsc_uint8 socket_connected(sfsc_int16 socket)
{
    return 1;
}
#else
#include "../sfsc/platform/sfsc_platform.h"
sfsc_int16 socket_connect(const char *host, sfsc_uint16 port){return 0;}
sfsc_int16 socket_write(sfsc_int16 socket,const sfsc_uint8 *buf, sfsc_int16 size){return 0;}
sfsc_uint16 socket_available(sfsc_int16 socket){return 0;}
sfsc_int16 socket_read(sfsc_int16 socket, sfsc_uint8 *buf, sfsc_int16 size){return 0;}
void socket_flush(sfsc_int16 socket){}
void socket_stop(sfsc_int16 socket){}
sfsc_uint8 socket_connected(sfsc_int16 socket){return 0;}
#endif
#endif
