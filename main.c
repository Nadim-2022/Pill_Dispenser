#include "my_header.h"

bool lora_connected = false;


// Dispense functions with piezo handler
static bool pill_dropped = false;

void my_interrupt_handler_piezo(uint gpio, uint32_t events) // remake all this leave onluy pill_dropped = true;
{
    while (!pill_dropped)
    {
        if (gpio_get(gpio) == 0)
        {
            pill_dropped = true;
            break;
        }
    }
}


void dispensepills(motor_pos *motorPos, log_entry *my_log_entry)
{
    uint64_t start_time = time_us_64();
    uint64_t lastToggleTime = time_us_64();
    int ledToggleCount = 0;
    if (motorPos->recalibrate == true)
    {
        motorPos->recalibrate = false;
        isStopedRun(motorPos);
    }
    lora_message("Pill dispense start", lora_connected);
    pillDispenseState pillDispenseState = STATE_DISPENSING_START;
    while (motorPos->current_pill_num < 8)
    {
        switch (pillDispenseState)
        {
        case STATE_DISPENSING_START:
            runDispenser(motorPos);
            start_time = time_us_64();
            while (time_us_64() - start_time < 100000)
            {
            }
            motorPos->current_pill_num++;
            if (motorPos->current_pill_num < 8)
            {
                eeprom_write_bytes(EEPROM_MOTOR_POSITION - 5, (uint8_t *)&motorPos->pos, sizeof(motor_pos));
            }
            if (pill_dropped && motorPos->current_pill_num < 8)
            {
                pillDispenseState = STATE_DISPENSING_PILL_DISPENSED;
            }
            else
            {
                pillDispenseState = STATE_DISPENSING_PILL_NOT_DISPENSED;
            }
            break;
        case STATE_DISPENSING_CONTINUE:
            if (time_us_64() - start_time >= TIME_DISPENSE)
            {
                runDispenser(motorPos);
                start_time = time_us_64();
                while (time_us_64() - start_time < 100000)
                {
                }
                motorPos->current_pill_num++;
                if (motorPos->current_pill_num < 8)
                {
                    eeprom_write_bytes(EEPROM_MOTOR_POSITION - 5, (uint8_t *)&motorPos->pos, sizeof(motor_pos));
                }
                if (pill_dropped && motorPos->current_pill_num < 8)
                {
                    pillDispenseState = STATE_DISPENSING_PILL_DISPENSED;
                }
                else
                {
                    pillDispenseState = STATE_DISPENSING_PILL_NOT_DISPENSED;
                }
            }
            break;
        case STATE_DISPENSING_PILL_DISPENSED:
            pill_dropped = false;
            if (motorPos->current_pill_num < 8)
            {
                sprintf(my_log_entry->message, "Log: Pill dispensed %d, Pills left on dispenser %d", motorPos->current_pill_num, 7 - motorPos->current_pill_num);
                write_log(my_log_entry, &motorPos->address);
                lora_message(my_log_entry->message, lora_connected);
            }
            pillDispenseState = STATE_DISPENSING_CONTINUE;
            break;
        case STATE_DISPENSING_PILL_NOT_DISPENSED:
            while (ledToggleCount < 10)
            {
                if (time_us_64() - lastToggleTime >= TOGGLE_DELAY_2)
                {
                    gpio_put(LED_1, !gpio_get(LED_1));
                    lastToggleTime = time_us_64();
                    ledToggleCount++;
                }
            }
            ledToggleCount = 0;
            if (motorPos->current_pill_num < 8)
            {
                sprintf(my_log_entry->message, "Log: Pill Not dispensed %d, Pills left on dispenser %d", motorPos->current_pill_num, 7 - motorPos->current_pill_num);
                write_log(my_log_entry, &motorPos->address);
                lora_message(my_log_entry->message, lora_connected);
            }
            pillDispenseState = STATE_DISPENSING_CONTINUE;
            break;
        }
    }
    motorPos->revol = 0;
    lora_message("Pill dispenser empty", lora_connected);
    read_log();
}

int main()
{
    stdio_init_all();
    my_init_all();

    motor_pos motorPos;
    log_entry my_log_entry;
    ProgramStatesPillDispenser program_state = STATE_CALIB_NO;
    boot_state bootState;

    // Connecting to Lora
    
    // UNCOMMENT THIS NOT TO SKIP LORA CONNECTION !
    // while (!lora_connected) // Infinite attempts to connect
    // {
    //     lora_connected = lora();
    // }

    // Boot message
    if (watchdog_caused_reboot())
    {
        lora_message("Log: Watchdog reboot.", lora_connected);
    }
    else
    {
        lora_message("Log: Boot.", lora_connected);
    }

    // Enabling Watchdog
    watchdog_enable(MY_WATCHDOG_TIMEOUT, true);

    // Reading EEPROM memory
    eeprom_read_bytes(EEPROM_SIZE - 1, (uint8_t *)&bootState, sizeof(boot_state));
    if (!get_boot_state(&bootState))
    {
        printf("Boot state: %d\n", bootState.state);
        set_boot_state(&bootState, true);
        eeprom_write_bytes(EEPROM_SIZE - 1, (uint8_t *)&bootState, sizeof(boot_state));
    }
    read_log();
    erase_log(0);
    eeprom_read_bytes(EEPROM_MOTOR_POSITION - 5, (uint8_t *)&motorPos, sizeof(motor_pos));

    if (motorPos.current_pill_num >= 7)
    {
        motorPos.current_pill_num = 0;
    }
    if (bootState.state && motorPos.current_pill_num > 0)
    {
        program_state = STATE_REALIGN;
    }

    // Interrupt
    gpio_set_irq_enabled_with_callback(PIN_PIEZO_SENSOR, GPIO_IRQ_EDGE_FALL, true, &my_interrupt_handler_piezo); // move to respective state

    // Other
    uint64_t lastToggleTime = time_us_64();
    bool ledToggle = true;

    while (true)
    {
        switch (program_state)
        {
        case STATE_CALIB_NO:
            watchdog_update();
            if (ledToggle && (time_us_32() - lastToggleTime >= TOGGLE_DELAY))
            {
                gpio_put(LED_1, !gpio_get(LED_1));
                lastToggleTime = time_us_64();
            }
            if (!gpio_get(BTN_1))
            {
                motorPos.revol = 0;
                motorPos.current_pill_num = 0;
                ledToggle = false;
                gpio_put(LED_1, 0);
                program_state = STATE_CALIBRATION;
            }
            break;

        case STATE_CALIBRATION:
            lora_message("Calib start", lora_connected);
            calib(&motorPos);
            gpio_put(LED_1, 1);
            program_state = STATE_CALIB_DONE;
            motorPos.current_pill_num = 0;
            lora_message("Calib done", lora_connected);
            break;

        case STATE_CALIB_DONE:
            while (1)
            {
                watchdog_update();
                if (!gpio_get(BTN_2))
                {
                    program_state = STATE_DISPENSING;
                    break;
                }
            }
            break;

        case STATE_DISPENSING:
            watchdog_update();
            gpio_put(LED_1, 0);
            dispensepills(&motorPos, &my_log_entry);
            ledToggle = true;
            program_state = STATE_CALIB_NO;
            break;

        case STATE_REALIGN:
            lora_message("Power off during pill dispense, realigning. Then continue to dispense.", lora_connected);
            watchdog_update();
            recalib(&motorPos);
            dispensepills(&motorPos, &my_log_entry);
            program_state = STATE_DISPENSING;
        default:
            program_state = STATE_CALIB_NO;
        }
    }
    return 0;
}
