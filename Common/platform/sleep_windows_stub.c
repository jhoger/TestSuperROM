/*
 * Platform Sleep - Windows Stub Implementation
 */

#include "platform_compat.h"
#include <unistd.h>

void platform_sleep_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void platform_sleep_us(uint32_t us) {
    usleep(us);
}
