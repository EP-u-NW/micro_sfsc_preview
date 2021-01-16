#ifdef ESP_IDF
#ifdef EVALUATION

#include "esp_idf_config.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "../debug/console.h"
#include "esp_idf_wifi.h"
#include "../evaluation/evaluation.h"
#include "esp_task_wdt.h"
#include "esp_idf_platform.h"
#include "../evaluation/size_print.h"

const sfsc_uint8 ssid[] = "The Promised Lan2.4";
const sfsc_uint8 password[] = "ericistsuper";
const char epnw[] = "81.169.207.220";
const char stauli[] = "192.168.175.73";
const char local[] = "192.168.178.171";
const char pc[] = "192.168.178.92";
const char *address = pc;
const sfsc_uint16 bootstrap_port = 11251;

#include "../sfsc/sfsc_adapter/sfsc_adapter.h"
#include "../sfsc/sfsc_adapter/sfsc_adapter_struct.h"
sfsc_adapter adapter;
sfsc_uint8 operating = 0;
sfsc_uint64 operation_start_time = 0;
sfsc_uint8 start_up = 0;

static void main_system_task()
{
    if (operating)
    {
        sfsc_int8 op_result = system_task(&adapter);
        if (op_result == SFSC_OK)
        {
            if (adapter_stats(&adapter)->state == SFSC_STATE_OPERATIONAL && start_up == 0)
            {
                start_up = 1;
                console_print_char("My id: ");
                console_write(adapter_stats(&adapter)->adapter_id, 36);
                console_println();
                console_print_char("Connected to core ");
                console_write(adapter_stats(&adapter)->core_id, 36);
                console_println();
                operation_start_time = time_ms();
                vTaskDelay(1000/portTICK_PERIOD_MS);
            }
        }
        else if (op_result == W_MESSAGE_DROP)
        {
            console_println_char("Message drop!");
        }
        else
        {
            console_print_char("Error: ");
            console_println_int8(op_result);
            console_print_char("Total operational time: ");
            if (operation_start_time > 0)
            {
                console_println_uint32((sfsc_uint32)(time_ms() - operation_start_time));
            }
            operating = 0;
        }
    }
}

static void main_user_task()
{
    sfsc_uint64 start = time_ms();
    sfsc_int8 op_result = user_task(&adapter);
    sfsc_uint64 user_task_duration = time_ms() - start;
    if (user_task_duration > 200)
    {
        console_print_char("WARNING: user loop took ");
        console_print_uint32((sfsc_uint32)user_task_duration);
        console_println_char("ms");
    }
    if (op_result != ZMTP_OK)
    {
        console_print_char("User loop error ");
        console_println_int8(op_result);
        operating = 0;
    }
}

void main_setup()
{
    start_session_bootstraped(&adapter, address, bootstrap_port);
    operating = 1;
}

#ifdef MULTI_TASK
void system_task()
{
    lock_init();
    while (1)
    {
        main_system_task();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void user_task()
{
    while (1)
    {
        if (adapter_stats(&adapter)->state >= SFSC_STATE_OPERATIONAL)
        {
            main_user_task();
            for (sfsc_uint8 i = 0; i < 50; i++)
            {
                create_service_step(&adapter);
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void multi_task()
{

    xTaskCreatePinnedToCore(
        system_task,   // Task function.
        "System Task", // name of task.
        25000,         // Stack size of task
        NULL,          // parameter of the task
        12,            // priority of the task
        NULL,
        0);

    xTaskCreatePinnedToCore(
        user_task,   // Task function.
        "User Task", // name of task.
        25000,       // Stack size of task
        NULL,        // parameter of the task
        14,          // priority of the task
        NULL,
        0);
}
#else

#define SYSTEM_NO_SLEEP
//#define SYSTEM_NO_SLEEP_TILL_DONE
#define SYSTEM_SLEEP 10 / portTICK_PERIOD_MS
#define SYSTEM_TASK_PER_STEP 10
#define CREATE_TASK_PER_STEP 2
#define SUB_TEST
#define PUB_TEST
#define QUERY_TEST
#define CREATE_TEST

void single_task()
{
    console_print_char("TASK ON ");
    console_println_uint32(xPortGetCoreID());
    while (1)
    {
        for (sfsc_uint8 i = 0; i < SYSTEM_TASK_PER_STEP; i++)
        {
            esp_task_wdt_reset();
            main_system_task();
#ifndef SYSTEM_NO_SLEEP
#ifdef SYSTEM_NO_SLEEP_TILL_DONE
            if (create_service_done())
            {
#endif
                vTaskDelay(SYSTEM_SLEEP);
#ifdef SYSTEM_NO_SLEEP_TILL_DONE
            }
#endif
#endif
        }
        if (adapter_stats(&adapter)->state >= SFSC_STATE_OPERATIONAL)
        {
            main_user_task();
#ifdef PUB_TEST
            for (sfsc_uint16 i = 0; i < CREATE_TASK_PER_STEP; i++)
            {
                pub_test_step(&adapter);
            }
#endif
#ifdef SUB_TEST
            subscription_track_step(&adapter);
#endif
#ifdef QUERY_TEST
            query_test_step(&adapter);
#endif
#ifdef CREATE_TEST
                for (sfsc_uint16 i = 0; i < CREATE_TASK_PER_STEP; i++)
            {
                create_service_step(&adapter);
            }
#endif
        }
    }
}

#endif

static void esp_idf_checks()
{
    console_print_char("APP MAIN ON ");
    console_println_uint32(xPortGetCoreID());
#ifdef SYSTEM_SLEEP
    console_print_char("SYSTEM_SLEEP: ");
    console_println_uint32(SYSTEM_SLEEP);
#endif
    console_print_char("portTICK_PERIOD_MS: ");
    console_println_uint32(portTICK_PERIOD_MS);
    console_print_char("configTICK_RATE_HZ: ");
    console_println_uint32(configTICK_RATE_HZ);

#ifdef CONFIG_LWIP_SO_RCVBUF
    console_println_char("CONFIG_LWIP_SO_RCVBUF"); //Need this
#else
#error
#endif
#ifdef CONFIG_LWIP_TCPIP_TASK_AFFINITY_NO_AFFINITY
    console_println_char("CONFIG_LWIP_TCPIP_TASK_NO_AFFINITY");
#endif
#ifdef CONFIG_LWIP_TCPIP_TASK_AFFINITY_CPU0
    console_println_char("CONFIG_LWIP_TCPIP_TASK_AFFINITY_CPU0");
#endif
#ifdef CONFIG_LWIP_TCPIP_TASK_AFFINITY_CPU1
    console_println_char("CONFIG_LWIP_TCPIP_TASK_AFFINITY_CPU1"); //And this
#else
#error
#endif
}

void app_main(void)
{
    for (int i = 10; i > 0; i--)
    {
        console_print_char("Starting in ");
        console_println_uint8(i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    esp_idf_checks();
#ifdef ONLY_SIZE_PRINT
    print_sizes();
    return;
#endif
    connect_wifi((const char *)ssid, (const char *)password);
    main_setup();
#ifdef MULTI_TASK
    multi_task();
#else
    single_task();
#endif
}
#endif
#endif