#ifdef ESP_IDF

#include <stdio.h>

#include "esp_idf_wifi.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#ifdef ENABLE_PRINTS
#include "../shared/console.h"
#endif

#ifndef CONFIG_LWIP_SO_RCVBUF
#error "Bad configuration, see the README"
#endif
#ifndef CONFIG_LWIP_TCPIP_TASK_AFFINITY_CPU1
#error "Bad configuration, see the README"
#endif

// Configure you wifi settings
const sfsc_uint8 ssid[] = "The Promised Lan2.4";
const sfsc_uint8 password[] = "ericistsuper";

// Configure the SFSC core address and the cores control pub port (since we are
// using bootstraping)
const char epnw[] = "81.169.207.220";
const char stauli[] = "192.168.175.73";
const char local[] = "192.168.178.171";
const char pc[] = "192.168.178.92";
const char *address = epnw;
const sfsc_uint16 bootstrap_port = 1251;

void app_main(void) {
    for (int i = 10; i > 0; i--) {
#ifdef ENABLE_PRINTS
        console_print_char("Starting in ");
        console_println_uint8(i);
#endif
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    connect_wifi((const char *)ssid, (const char *)password);
}
#endif