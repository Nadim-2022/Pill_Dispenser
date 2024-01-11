#ifndef PILL_DISPENSER_PROJECT_WATCHDOG_H
#define PILL_DISPENSER_PROJECT_WATCHDOG_H

#include "hardware/watchdog.h"

void watchdog_init(uint64_t timeout_ms);
void watchdog_feed(void);

#endif //PILL_DISPENSER_PROJECT_WATCHDOG_H
