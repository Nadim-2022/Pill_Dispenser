// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"

#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "hardware/flash.h"
#include "hardware/structs/bus_ctrl.h"
#include "hardware/structs/sio.h"
#include "hardware/watchdog.h"

#include "uart.h"
#include "ring_buffer.h"

// Definitions
// LEDs
#define LED_1 20
#define LED_2 21
#define LED_3 22

// Buttons
#define BTN_1 9
#define BTN_2 8
#define BTN_3 7

// Motor
#define PIN_MOTOR_1 2
#define PIN_MOTOR_2 3
#define PIN_MOTOR_3 6
#define PIN_MOTOR_4 13
#define MOTOR_STEP_DELAY_MS 1

// Piezo sensor
#define PIN_PIEZO_SENSOR 27

// Opto fork sensor
#define PIN_OPTO_FORK 28

// I2C
#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17
#define I2C_BAUD_RATE 100000

// EEPROM
#define DEVADDR 0x50
#define EEPROM_SIZE 32767 // Replace with your EEPROM size
#define EEPROM_MOTOR_POSITION 4000
#define LOG_ENTRY_ADD_MAX 2048 // Size of the log area in EEPROM
#define LOG_ENTRY_SIZE 64

// UART
#define UART_NR 1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define UART_BAUD_RATE 9600

#define loraWaiting 500000
#define loraWaiting2 10000000
#define msgWaiting 5000000
#define STRLEN 256

// Time
#define TIME_DISPENSE 5000000 // 30000000
#define TOGGLE_DELAY 500000
#define TOGGLE_DELAY_2 250000
#define MY_WATCHDOG_TIMEOUT 8300

// Other
#define CALIBRATION_ROUNDS 3

typedef struct motor_position
{
    int pos;
    int revol;
    int microstep;
    int current_pill_num;
    uint16_t address;
    bool recalibrate;
} motor_pos;

typedef enum
{
    STATE_CALIB_NO,
    STATE_CALIBRATION,
    STATE_CALIB_DONE,
    STATE_DISPENSING,
    STATE_REALIGN,
} ProgramStatesPillDispenser;

typedef struct irq_handler_args
{
    uint gpio;
    uint32_t events;
    uint64_t timestamp;
    bool pill;
} irq_handler_args;

typedef struct log_entry
{
    char message[64];
} log_entry;

typedef enum
{
    STATE_DISPENSING_START,
    STATE_DISPENSING_CONTINUE,
    STATE_DISPENSING_PILL_DISPENSED,
    STATE_DISPENSING_PILL_NOT_DISPENSED
} pillDispenseState;

typedef enum
{
    lora_at,
    lora_lwotaa,
    lora_appkey,
    lora_class,
    lora_port,
    lora_join,
} loraState;

static const uint8_t clockwise[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}};

typedef struct boot_state
{
    bool state;
    bool not_state;
} boot_state;

// Global variables

// Function declarations
void my_init_all();
void init_motor_pos(motor_pos *motorPos);
void set_boot_state(boot_state *boot_state, bool state);
bool get_boot_state(boot_state *boot_state);
void runDispenser(motor_pos *motorPos);
void isStopedRun(motor_pos *motorPos);
uint16_t crc16(const uint8_t *data_p, size_t length);
void eeprom_write_bytes(uint16_t addr, const uint8_t *values, uint16_t length);
void eeprom_read_bytes(uint16_t addr, uint8_t *values, uint16_t length);
void write_log(log_entry *le, uint16_t *address);
void read_log();
void erase_log(uint16_t address);
bool lora();
bool lora_message(const char *message, bool lora_connected);
void run(motor_pos *motorPos, int start, bool startCount, bool clockwiseDirection);
void calib(motor_pos *motorPos);
void recalib(motor_pos *motorPos);

