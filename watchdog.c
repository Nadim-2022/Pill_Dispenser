#include "watchdog.h"
#include "hardware/watchdog.h"

void watchdog_init(uint64_t timeout_ms) {
    watchdog_enable(timeout_ms, true);
}

void watchdog_feed(void) {
    watchdog_update();
}