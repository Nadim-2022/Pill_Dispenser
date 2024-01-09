//
// Created by iamna on 20/12/2023.
//

#ifndef PILL_DISPENSER_PROJECT_HEADER_H
#define PILL_DISPENSER_PROJECT_HEADER_H

#endif //PILL_DISPENSER_PROJECT_HEADER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "hardware/flash.h"
#include "hardware/structs/bus_ctrl.h"
#include "hardware/structs/sio.h"
#include "uart.h"


//LED
#define LED_1 20
#define LED_2 21
#define LED_3 22

//Button
#define BTN_1 9
#define BTN_2 8
#define BTN_3 7

//Stepper
#define STPER_GP2 2
#define STPER_GP3 3
#define STPER_GP6 6
#define STPER_GP13 13

//Piezo & Opto
#define GPIO_Piezo 27
#define GPIO_Opto 28

//I2C
#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17
#define DEVADDR 0x50
#define EEPROM_SIZE 32767  // Replace with your EEPROM size
#define EEPROM_motorPos 4000
#define LOG_ENTRY_ADD_MAX 2048  // Size of the log area in EEPROM
#define LOG_ENTRY_SIZE 64

//UART
#define UART_NR 1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define BAUD_RATE 9600
#define loraWaiting 500000
#define loraWaiting2 10000000
#define msgWaiting 5000000
#define STRLEN 256

#define DispenTime  5000000//30000000
#define TOGGLE_DELAY 500000
#define TOGGLE_DELAY2 250000
#define MOTOR_DELAY 2 // ms

#define Calicarate_count 1

typedef struct motor_position{
    int pos;
    int revol;
    int microstep;
    int currentPillnum;
    uint16_t address;
    bool recaliberate;
} motor_pos;

typedef enum {
    DisStart,
    DisCalib,
    DisRun,
    DisreCalib,
}dis_state;

typedef struct irq_handler_args {
    uint gpio;
    uint32_t events;
    uint64_t timestamp;
    bool pill;
} irq_handler_args;

typedef struct pil_state{
    dis_state state;
} pil_state;

typedef struct log_entry {
    char message[64];
} log_entry;


static const uint8_t clockwise[8][4] = {
        {1,0,0,0},
        {1,1,0,0},
        {0,1,0,0},
        {0,1,1,0},
        {0,0,1,0},
        {0,0,1,1},
        {0,0,0,1},
        {1,0,0,1}
};
void init();
void calib(motor_pos *motorPos);
void run(motor_pos *motorPos, int start, bool startCount, bool clockwiseDirection);
//void run(motor_pos *motorPos, int start, int end, bool startCount);
void calibStep(motor_pos *motorPos);
void dispensepills(motor_pos *motorPos, log_entry *le);
void piezoHandler(uint gpio, uint32_t events);
void eeprom_write_bytes(uint16_t addr, const uint8_t* values, uint16_t length);
void eeprom_read_bytes(uint16_t addr, uint8_t* values, uint16_t length);
void recalib(motor_pos *motorPos);
void write_log(log_entry *le, uint16_t *address);
void read_log();
void erase_log(uint16_t address);
bool lora();
bool loraMsg(const char *msg);

typedef struct boot_state{
    bool state;
    bool not_state;
} boot_state;

void init_motor_pos(motor_pos *motorPos);
void set_boot_state(boot_state *boot_state, bool state);
bool get_boot_state(boot_state *boot_state);