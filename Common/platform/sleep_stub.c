/*
 * Platform Sleep - Stub Implementation (FAILING)
 *
 * These functions are intentionally non-functional to demonstrate
 * that tests fail without proper implementation.
 */

#include "platform_compat.h"
#include <stdio.h>

/* Stub implementations - these will cause tests to fail */

void platform_sleep_ms(uint32_t ms) {
    (void)ms;
    /* Stub: does nothing - tests will timeout/fail */
    /* No actual sleep occurs */
}

void platform_sleep_us(uint32_t us) {
    (void)us;
    /* Stub: does nothing - tests will timeout/fail */
    /* No actual sleep occurs */
}
