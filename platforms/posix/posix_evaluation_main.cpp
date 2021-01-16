#ifdef POSIX
#ifdef EVALUATION
#include "../evaluation/evaluation.h"
#include "../debug/console.h"

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

void single_task_task()
{
    while (1)
    {
        main_system_task();
        if (adapter_stats(&adapter)->state >= SFSC_STATE_OPERATIONAL)
        {
            main_user_task();
            create_service_step(&adapter);
        }
    }
}

int main()
{
    main_setup();
    single_task_task();
}
#endif
#endif