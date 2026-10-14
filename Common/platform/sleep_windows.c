/*
 * Platform Sleep - Windows Implementation
 */

#include "platform_compat.h"
#include <windows.h>

void platform_sleep_ms(uint32_t ms) {
    Sleep(ms);
}

void platform_sleep_us(uint32_t us) {
    /* Windows Sleep() is millisecond precision */
    /* For microsecond precision, use WaitableTimer */
    if (us < 1000) {
        /* Busy wait for very short durations */
        LARGE_INTEGER start, end;
        QueryPerformanceCounter(&start);

        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);

        uint64_t target_ticks = (uint64_t)us * freq.QuadPart / 1000000ULL;

        do {
            QueryPerformanceCounter(&end);
        } while ((end.QuadPart - start.QuadPart) < (LONGLONG)target_ticks);
    } else {
        /* For longer durations, use Sleep() */
        Sleep(us / 1000);
    }
}
