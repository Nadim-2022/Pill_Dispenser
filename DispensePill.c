//
// Created by iamna on 20/12/2023.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "header.h"


static bool pillDroped = false;
static uint64_t pilstart;

void dispensepills(motor_pos *motorPos, log_entry *le) {
    bool write = false;
    if(motorPos->recaliberate == true){
        motorPos->recaliberate = false;
        int have_to = motorPos->microstep * motorPos->currentPillnum;
        int i = 0;
        while(i < have_to){
            for(int j = motorPos->pos; j < 8; j++){
                gpio_put(STPER_GP2, clockwise[j][0]);
                gpio_put(STPER_GP3, clockwise[j][1]);
                gpio_put(STPER_GP6, clockwise[j][2]);
                gpio_put(STPER_GP13, clockwise[j][3]);
                sleep_ms(MOTOR_DELAY);
                motorPos->pos++;
                i++;
                if(motorPos->pos == 8){
                    motorPos->pos = 0;
                }
                if(i == have_to){
                    break;
                }
            }

        }
    }
    int currentmicrostep = 0;
    uint64_t start_time = time_us_64();
    uint64_t lastToggleTime = time_us_64();
    int ledToggleCount = 0;
    while(motorPos->currentPillnum < 8){
        if (time_us_64()-start_time >= DispenTime){
            pillDroped = false;
            write = true;
            while (currentmicrostep < motorPos->microstep){
                for (int i = motorPos->pos; i < 8; i++) {
                    gpio_put(STPER_GP2, clockwise[i][0]);
                    gpio_put(STPER_GP3, clockwise[i][1]);
                    gpio_put(STPER_GP6, clockwise[i][2]);
                    gpio_put(STPER_GP13, clockwise[i][3]);
                    motorPos->pos++;
                    currentmicrostep++;
                    sleep_ms(1);
                    if (motorPos->pos == 8) {
                        motorPos->pos = 0;
                    }
                    if (currentmicrostep == motorPos->microstep) {
                        break;
                    }
                }

            }
            start_time = time_us_64();
            pilstart = time_us_64();
            currentmicrostep = 0;
            motorPos->currentPillnum++;
            printf("Current Pillnum %d\n", motorPos->currentPillnum);
            ledToggleCount = 0;
            // write to the eprom after every pill dispensed all motorPos

            if(motorPos->currentPillnum < 8) {
                eeprom_write_bytes(EEPROM_motorPos - 5, (uint8_t *) &motorPos->pos, sizeof(motor_pos));
            }

        }
        if(pillDroped && write && motorPos->currentPillnum > 0 && motorPos->currentPillnum < 8){
            sprintf(le->message, "Pill dispensed %d", motorPos->currentPillnum);
            write_log(le, &motorPos->address);
            printf("Pill dispensed %d\n", motorPos->currentPillnum);
            write = false;
        }
        /*
        if(time_us_64()-start_time > 100000 && !pillDroped){
            while (ledToggleCount < 10 ){
                if(time_us_64() - lastToggleTime >= TOGGLE_DELAY){
                    gpio_put(LED_1, !gpio_get(LED_1));
                    lastToggleTime = time_us_64();
                    ledToggleCount++;
                }
                //write = true;
            }
        }
    */

        /*
        else if(!pillDroped && write && motorPos->currentPillnum > 0 && motorPos->currentPillnum < 8){
            sprintf(le->message, "Pill not dispensed %d", motorPos->currentPillnum);
            write_log(le, &motorPos->address);
            printf("Pill not dispensed %d\n", motorPos->currentPillnum);
            write = false;
        }
         */
        /*
        // write log to eeprom after every pill dispensed
        if(motorPos->currentPillnum > 0 && motorPos->currentPillnum < 8 && pillDroped){
            sprintf(le->message, "Pill dispensed %d", motorPos->currentPillnum);
            write_log(le, &motorPos->address);
            printf("Pill dispensed %d\n", motorPos->currentPillnum);
            pillDroped = false;
        }
         */

        /*
        else if(write && motorPos->currentPillnum > 0 ) {
            sprintf(le->message, "Pill not dispensed %d", motorPos->currentPillnum);
            write_log(le, &motorPos->address);
            printf("Pill not dispensed %d\n", motorPos->currentPillnum);
            write = false;
        }
         */

    }
    motorPos->revol = 0;
    printf("Dispensing complete\n");
    read_log();
}

void piezoHandler(uint gpio, uint32_t events){
    while(time_us_64() - pilstart < 85600 && !pillDroped){
        if(gpio_get(gpio) == 0){
            printf("Pill dispensed\n");
            pillDroped = true;
            break;
        }
    }
}