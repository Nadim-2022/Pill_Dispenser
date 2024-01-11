#include "my_header.h"

void my_init_all()
{
    // Stepper motor
    gpio_init(PIN_MOTOR_1);
    gpio_init(PIN_MOTOR_2);
    gpio_init(PIN_MOTOR_3);
    gpio_init(PIN_MOTOR_4);
    gpio_set_dir(PIN_MOTOR_1, GPIO_OUT);
    gpio_set_dir(PIN_MOTOR_2, GPIO_OUT);
    gpio_set_dir(PIN_MOTOR_3, GPIO_OUT);
    gpio_set_dir(PIN_MOTOR_4, GPIO_OUT);

    // LED 
    gpio_init(LED_1);
    gpio_init(LED_2);
    gpio_init(LED_3);
    gpio_set_dir(LED_1, GPIO_OUT);
    gpio_set_dir(LED_2, GPIO_OUT);
    gpio_set_dir(LED_3, GPIO_OUT);

    // LED Buttons
    gpio_init(BTN_1);
    gpio_init(BTN_2);
    gpio_init(BTN_3);
    gpio_set_dir(BTN_1, GPIO_IN);
    gpio_set_dir(BTN_2, GPIO_IN);
    gpio_set_dir(BTN_3, GPIO_IN);
    gpio_pull_up(BTN_1);
    gpio_pull_up(BTN_2);
    gpio_pull_up(BTN_3);

    // Piezo sensor
    gpio_init(PIN_PIEZO_SENSOR);
    gpio_set_dir(PIN_PIEZO_SENSOR, GPIO_IN);
    gpio_pull_up(PIN_PIEZO_SENSOR);

    // Opto fork sensor
    gpio_init(PIN_OPTO_FORK);
    gpio_set_dir(PIN_OPTO_FORK, GPIO_IN);
    gpio_pull_up(PIN_OPTO_FORK);

    // I2C
    i2c_init(i2c0, I2C_BAUD_RATE);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);

    // UART
    uart_setup(UART_NR, UART_TX_PIN, UART_RX_PIN, UART_BAUD_RATE);
}
void init_motor_pos(motor_pos *motorPos)
{
    motorPos->pos = 0;
    motorPos->revol = 0;
    motorPos->microstep = 0;
    motorPos->current_pill_num = 0; // 0 = no pill, 1 = pill
    motorPos->address = 0;
    motorPos->recalibrate = false;
}

void set_boot_state(boot_state *boot_state, bool state)
{
    boot_state->state = state;
    boot_state->not_state = !state;
}
bool get_boot_state(boot_state *boot_state)
{
    return boot_state->state == !boot_state->not_state;
}

void runDispenser(motor_pos *motorPos)
{
    int currentmicrostep = 0;
    while (currentmicrostep < motorPos->microstep)
    {
        for (int i = motorPos->pos; i < 8; i++)
        {
            gpio_put(PIN_MOTOR_1, clockwise[i][0]);
            gpio_put(PIN_MOTOR_2, clockwise[i][1]);
            gpio_put(PIN_MOTOR_3, clockwise[i][2]);
            gpio_put(PIN_MOTOR_4, clockwise[i][3]);
            motorPos->pos++;
            currentmicrostep++;
            sleep_ms(1);
            if (motorPos->pos == 8)
            {
                motorPos->pos = 0;
            }
            if (currentmicrostep == motorPos->microstep)
            {
                break;
            }
        }
    }
    watchdog_update();
}
void isStopedRun(motor_pos *motorPos)
{
    int have_to = motorPos->microstep * motorPos->current_pill_num;
    int i = 0;
    while (i < have_to)
    {
        for (int j = motorPos->pos; j < 8; j++)
        {
            gpio_put(PIN_MOTOR_1, clockwise[j][0]);
            gpio_put(PIN_MOTOR_2, clockwise[j][1]);
            gpio_put(PIN_MOTOR_3, clockwise[j][2]);
            gpio_put(PIN_MOTOR_4, clockwise[j][3]);
            sleep_ms(MOTOR_STEP_DELAY_MS);
            motorPos->pos++;
            i++;
            if (motorPos->pos == 8)
            {
                motorPos->pos = 0;
            }
            if (i == have_to)
            {
                break;
            }
        }
    }
    watchdog_update();
}

uint16_t crc16(const uint8_t *data_p, size_t length)
{
    uint8_t x;
    uint16_t crc = 0xFFFF;
    while (length--)
    {
        x = crc >> 8 ^ *data_p++;
        x ^= x >> 4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ ((uint16_t)x);
    }
    return crc;
}

void eeprom_write_bytes(uint16_t addr, const uint8_t *values, uint16_t length)
{
    uint8_t data[length + 2];
    data[0] = (addr >> 8) & 0xFF; // High byte of address
    data[1] = addr & 0xFF;        // Low byte of address
    for (uint16_t i = 0; i < length; i++)
    {
        data[i + 2] = values[i];
    }
    i2c_write_blocking(i2c0, DEVADDR, data, length + 2, false);
    sleep_ms(10); // Wait for internal write to complete
}

void eeprom_read_bytes(uint16_t addr, uint8_t *values, uint16_t length)
{
    uint8_t data[2];
    data[0] = (addr >> 8) & 0xFF;                            // High byte of address
    data[1] = addr & 0xFF;                                   // Low byte of address
    i2c_write_blocking(i2c0, DEVADDR, data, 2, true);        // Send address
    i2c_read_blocking(i2c0, DEVADDR, values, length, false); // Read values
}

void write_log(log_entry *le, uint16_t *address)
{
    uint8_t data[LOG_ENTRY_SIZE]; // 64 bytes
    size_t le_size = strlen(le->message);
    if (le_size > 0 && le_size < 62)
    {
        memcpy(data, le->message, le_size);
        data[le_size] = '\0';
        uint16_t crc = crc16(data, 62);
        data[le_size + 1] = (uint8_t)(crc >> 8);
        data[le_size + 2] = (uint8_t)crc;
        uint8_t read_data[LOG_ENTRY_SIZE];
        if (*address < LOG_ENTRY_ADD_MAX)
        {
            eeprom_read_bytes(*address, read_data, LOG_ENTRY_SIZE);
            // printf("addr: %d\n", *address);
            if (read_data[0] == 0)
            {
                eeprom_write_bytes(*address, data, LOG_ENTRY_SIZE);
            }
            *address += LOG_ENTRY_SIZE;
        }
        else
        {
            printf("delete\n");
            *address = 0;
            erase_log(*address);
        }
    }
}
void read_log()
{
    uint16_t addr = 0;
    uint8_t read_data[LOG_ENTRY_SIZE];
    printf("LOG:\n");
    while (addr < LOG_ENTRY_ADD_MAX)
    {
        eeprom_read_bytes(addr, read_data, LOG_ENTRY_SIZE);
        if (read_data[0] != 0)
        {
            if (strchr(read_data, '\0') && strchr(read_data, '\0') < &read_data[62])
            {
                if (crc16(read_data, 64) != 0)
                {
                    printf("%s\n", read_data);
                }
            }
        }
        addr += LOG_ENTRY_SIZE;
    }
}
void erase_log(uint16_t address)
{
    // To erase all the log entries, write 0x00 to the all the byte of each log entry.
    uint8_t data[LOG_ENTRY_SIZE] = {0};
    while (address < LOG_ENTRY_ADD_MAX)
    {
        eeprom_write_bytes(address, data, LOG_ENTRY_SIZE);
        address += LOG_ENTRY_SIZE;
    }
    address = 0;
}

bool lora()
{
    const char command0[] = "AT\r\n";
    const char command1[] = "AT+MODE=LWOTAA\r\n";
    const char command2[] = "AT+KEY=APPKEY,\"AD50014F5E3B35A03EF7CF0909B5B975\"\r\n";
    const char command3[] = "AT+CLASS=A\r\n";
    const char command4[] = "AT+PORT=8\r\n";
    const char command5[] = "AT+JOIN\r\n";
    char str[STRLEN];
    loraState loarsState = lora_at;

    int pos = 0;
    switch (loarsState)
    {
    case lora_at:
        uart_send(UART_NR, command0);
        sleep_ms(500);
        // my_time = time_us_32();
        pos = uart_read(UART_NR, (uint8_t *)str, STRLEN);
        if (pos > 0)
        {
            str[pos] = '\0';
            if (strstr(str, "OK") != NULL)
            {
                loarsState = lora_lwotaa;
            }
        }
    case lora_lwotaa:
        uart_send(UART_NR, command1);
        sleep_ms(500);
        // my_time = time_us_32();
        pos = uart_read(UART_NR, (uint8_t *)str, STRLEN);
        if (pos > 0)
        {
            str[pos] = '\0';
            if (strstr(str, "+MODE: LWOTAA") != NULL)
            {
                loarsState = lora_appkey;
            }
        }
    case lora_appkey:
        uart_send(UART_NR, command2);
        sleep_ms(500);
        // my_time = time_us_32();
        pos = uart_read(UART_NR, (uint8_t *)str, STRLEN);
        if (pos > 0)
        {
            str[pos] = '\0';
            if (strstr(str, "+KEY: APPKEY,AD50014F5E3B35A03EF7CF0909B5B975") != NULL)
            {
                loarsState = lora_class;
            }
        }
    case lora_class:
        uart_send(UART_NR, command3);
        sleep_ms(500);
        pos = uart_read(UART_NR, (uint8_t *)str, STRLEN);
        if (pos > 0)
        {
            str[pos] = '\0';
            if (strstr(str, "+CLASS: A") != NULL)
            {
                loarsState = lora_port;
            }
        }
    case lora_port:
        uart_send(UART_NR, command4);
        sleep_ms(500);
        pos = uart_read(UART_NR, (uint8_t *)str, STRLEN);
        if (pos > 0)
        {
            str[pos] = '\0';
            if (strstr(str, "+PORT: 8") != NULL)
            {
                loarsState = lora_join;
            }
        }
    case lora_join:
        uart_send(UART_NR, command5);
        sleep_ms(10000);
        pos = uart_read(UART_NR, (uint8_t *)str, STRLEN);
        if (pos > 0)
        {
            str[pos] = '\0';
            if (strstr(str, "+JOIN: Network joined") != NULL)
            {
                return true;
            }
            // else
            // {
            //     return false;
            // }
            // added default case
        }
    default:
        return false;
    }
}

bool lora_message(const char *message, bool lora_connected)
{
    if (lora_connected)
    {
        char preMsg[STRLEN];
        strcpy(preMsg, "AT+MSG=\"");
        strcat(preMsg, message);
        strcat(preMsg, "\"\r\n");
        char str[STRLEN];
        int pos = 0;
        uart_send(UART_NR, preMsg);
        sleep_ms(5000);
        pos = uart_read(UART_NR, (uint8_t *)str, STRLEN);
        watchdog_update();
        if (pos)
        {
            str[pos] = '\0';
            if (strstr(str, "+MSG: Done") != NULL)
            {
                return true;
            }
        }
        return false;
    }
    else{
        printf("LORA is no connected. ");
        printf("%s\n", message);
    }
}

// Run the motor
void run(motor_pos *motorPos, int start, bool startCount, bool clockwiseDirection)
{
    int stepIncrement = clockwiseDirection ? 1 : -1;
    while (gpio_get(PIN_OPTO_FORK) == start)
    {
        for (int i = motorPos->pos; i >= 0 && i < 8; i += stepIncrement)
        {
            gpio_put(PIN_MOTOR_1, clockwise[i][0]);
            gpio_put(PIN_MOTOR_2, clockwise[i][1]);
            gpio_put(PIN_MOTOR_3, clockwise[i][2]);
            gpio_put(PIN_MOTOR_4, clockwise[i][3]);
            sleep_ms(MOTOR_STEP_DELAY_MS);
            motorPos->pos += stepIncrement;
            if (startCount)
            {
                motorPos->revol++;
            }
            // Handle circular motion
            if (motorPos->pos == 8)
            {
                motorPos->pos = 0;
            }
            else if (motorPos->pos < 0)
            {
                motorPos->pos = 7;
            }

            // Check for opto sensor state change
            if (gpio_get(PIN_OPTO_FORK) != start)
            {
                break;
            }
        }
    }
    watchdog_update();
}

// Calibrate the motor
void calib(motor_pos *motorPos)
{
    run(motorPos, 0, false, true);
    run(motorPos, 1, false, true);
    for (int i = 0; i < CALIBRATION_ROUNDS; i++)
    {
        run(motorPos, 0, true, true);
        run(motorPos, 1, true, true);
        sleep_ms(100);
    }
    motorPos->microstep = (motorPos->revol / CALIBRATION_ROUNDS) / 8;
    int startPositioning = motorPos->microstep;
    int extra = 0;
    int goStepForward = 0;
    run(motorPos, 0, false, false);
    sleep_ms(500);
    while (goStepForward < startPositioning)
    {
        for (int i = motorPos->pos; i < 8; i++)
        {
            gpio_put(PIN_MOTOR_1, clockwise[i][0]);
            gpio_put(PIN_MOTOR_2, clockwise[i][1]);
            gpio_put(PIN_MOTOR_3, clockwise[i][2]);
            gpio_put(PIN_MOTOR_4, clockwise[i][3]);
            sleep_ms(MOTOR_STEP_DELAY_MS);
            motorPos->pos++;
            goStepForward++;
            if (gpio_get(PIN_OPTO_FORK) == 0)
            {
                extra++;
            }
            if (motorPos->pos == 8)
            {
                motorPos->pos = 0;
            }
            if (goStepForward == startPositioning)
            {
                break;
            }
        }
    }
    watchdog_update();
    while (extra != 0)
    {
        for (int i = motorPos->pos; i >= 0; i--)
        {
            gpio_put(PIN_MOTOR_1, clockwise[i][0]);
            gpio_put(PIN_MOTOR_2, clockwise[i][1]);
            gpio_put(PIN_MOTOR_3, clockwise[i][2]);
            gpio_put(PIN_MOTOR_4, clockwise[i][3]);
            sleep_ms(MOTOR_STEP_DELAY_MS);
            motorPos->pos--;
            extra--;
            if (motorPos->pos < 0)
            {
                motorPos->pos = 7;
            }
            if (extra == 0)
            {
                break;
            }
        }
    }
    watchdog_update();
}

void recalib(motor_pos *motorPos)
{
    sleep_ms(1000);
    run(motorPos, 1, false, false);
    sleep_ms(1000);
    run(motorPos, 0, false, false);
    int step_1 = 0;
    int extra = 0;
    sleep_ms(1000);
    while (step_1 < motorPos->microstep)
    {
        for (int i = motorPos->pos; i < 8; i++)
        {
            gpio_put(PIN_MOTOR_1, clockwise[i][0]);
            gpio_put(PIN_MOTOR_2, clockwise[i][1]);
            gpio_put(PIN_MOTOR_3, clockwise[i][2]);
            gpio_put(PIN_MOTOR_4, clockwise[i][3]);
            sleep_ms(MOTOR_STEP_DELAY_MS);
            motorPos->pos++;
            step_1++;
            if (gpio_get(PIN_OPTO_FORK) == 0)
            {
                extra++;
            }
            if (motorPos->pos == 8)
            {
                motorPos->pos = 0;
            }
            if (step_1 == motorPos->microstep)
            {
                break;
            }
        }
    }
    watchdog_update();
    printf("extra: %d\n", extra);
    sleep_ms(1000);
    while (extra != 0)
    {
        for (int i = motorPos->pos; i >= 0; i--)
        {
            gpio_put(PIN_MOTOR_1, clockwise[i][0]);
            gpio_put(PIN_MOTOR_2, clockwise[i][1]);
            gpio_put(PIN_MOTOR_3, clockwise[i][2]);
            gpio_put(PIN_MOTOR_4, clockwise[i][3]);
            sleep_ms(MOTOR_STEP_DELAY_MS);
            motorPos->pos--;
            extra--;
            // extra++;
            if (motorPos->pos < 0)
            {
                motorPos->pos = 7;
            }
            if (extra == 0)
            {
                break;
            }
        }
    }
    motorPos->recalibrate = true;
    sleep_ms(500);
    watchdog_update();
}
