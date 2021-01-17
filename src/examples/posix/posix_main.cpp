
#ifdef POSIX
#include "../../sfsc/platform/sfsc_platform.h"
#include "../../sfsc/platform/sfsc_types.h"
#include "../shared/console.h"
int main() {
    sfsc_uint8 ran_buf[4];
    for (sfsc_size i = 0; i < 10; i++) {
        random_bytes(ran_buf,4);
        console_print_char("Time: ");
        console_println_uint32((sfsc_uint32)time_ms());
        console_print_char("Random: ");
        console_println_uint32(*((sfsc_uint32*)ran_buf));
    }
}

#endif