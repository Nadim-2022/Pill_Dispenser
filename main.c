#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "header.h"
#include "watchdog.h"

int main()
{
    stdio_init_all();
    init();
    motor_pos motorPos;
    log_entry le;
    // init_motor_pos(&motorPos);
    pil_state pilState = {.state = DisStart};
    boot_state bootState;
    bool loraConn = false;
    while (!loraConn)
    {
        loraConn = lora();
        loraConn = true;
    }
    if (watchdog_caused_reboot())
    {
        loraMsg("Log: Watchdog reboot");
    }
    else
    {
        loraMsg("Log: Boot");
    }
    watchdog_init(8300);

    eeprom_read_bytes(EEPROM_SIZE - 1, (uint8_t *)&bootState, sizeof(boot_state));
    if (!get_boot_state(&bootState))
    {
        printf("Boot state: %d\n", bootState.state);
        set_boot_state(&bootState, true);
        eeprom_write_bytes(EEPROM_SIZE - 1, (uint8_t *)&bootState, sizeof(boot_state));
    }
    read_log();
    erase_log(0);
    eeprom_read_bytes(EEPROM_motorPos - 5, (uint8_t *)&motorPos, sizeof(motor_pos));
    if (motorPos.currentPillnum >= 7)
    {
        motorPos.currentPillnum = 0;
    }
    if (bootState.state && motorPos.currentPillnum > 0)
    {
        pilState.state = DisreCalib;
    }
    gpio_set_irq_enabled_with_callback(GPIO_Piezo, GPIO_IRQ_EDGE_FALL, true, &piezoHandler);
    uint64_t lastToggleTime = time_us_64();
    bool ledToggle = true;
    while (true)
    {
        switch (pilState.state)
        {
        case DisStart:
            watchdog_feed();
            if (ledToggle && (time_us_32() - lastToggleTime >= TOGGLE_DELAY))
            {
                gpio_put(LED_1, !gpio_get(LED_1));
                lastToggleTime = time_us_64();
            }
            if (!gpio_get(BTN_1))
            {
                motorPos.revol = 0;
                motorPos.currentPillnum = 0;
                ledToggle = false;
                gpio_put(LED_1, 0);
                pilState.state = DisCalib;
            }
            break;
        case DisCalib:
            loraMsg("Calib start");
            calib(&motorPos);
            gpio_put(LED_1, 1);
            pilState.state = DisRun;
            loraMsg("Calib done");
            break;
        case DisRun:
            watchdog_feed();
            if (!gpio_get(BTN_2))
            {
                motorPos.currentPillnum = 0;
                gpio_put(LED_1, 0);
                dispensepills(&motorPos, &le);
                ledToggle = true;
                pilState.state = DisStart;
            }
            break;
        case DisreCalib:
            loraMsg("Power off during pill dispense, recalibrating");
            watchdog_feed();
            recalib(&motorPos);
            dispensepills(&motorPos, &le);
            pilState.state = DisStart;
        }
    }
    return 0;
}
