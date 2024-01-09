//
// Created by iamna on 20/12/2023.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "header.h"
#include "watchdog.h"


static bool pillDroped = false;
typedef enum {
    startDispense,
    continueDispense,
    pillDispensed,
    pillNotDispensed
}pillDispenseState;
void runDispenser(motor_pos *motorPos){
    int currentmicrostep = 0;
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
    watchdog_feed();
}
void isStopedRun(motor_pos *motorPos){
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
    watchdog_feed();
}
void dispensepills(motor_pos *motorPos, log_entry *le) {
    uint64_t start_time = time_us_64();
    uint64_t lastToggleTime = time_us_64();
    int ledToggleCount = 0;
    if(motorPos->recaliberate == true){
        motorPos->recaliberate = false;
        isStopedRun(motorPos);
    }
    loraMsg("Pill dispense start");
    pillDispenseState pillDispenseState = startDispense;
    while (motorPos->currentPillnum < 8){
        switch (pillDispenseState) {
            case startDispense:
                runDispenser(motorPos);
                start_time = time_us_64();
                while (time_us_64()-start_time < 100000){
                }
                motorPos->currentPillnum++;
                if(motorPos->currentPillnum < 8){
                    eeprom_write_bytes(EEPROM_motorPos - 5, (uint8_t *) &motorPos->pos, sizeof(motor_pos));
                }
                if(pillDroped && motorPos->currentPillnum < 8){
                    pillDispenseState = pillDispensed;
                }else{
                    pillDispenseState = pillNotDispensed;
                }
                break;
            case continueDispense:
                if(time_us_64() - start_time >= DispenTime){
                    runDispenser(motorPos);
                    start_time = time_us_64();
                    while (time_us_64()-start_time < 100000){
                    }
                    motorPos->currentPillnum++;
                    if(motorPos->currentPillnum < 8){
                        eeprom_write_bytes(EEPROM_motorPos - 5, (uint8_t *) &motorPos->pos, sizeof(motor_pos));
                    }
                    if (pillDroped && motorPos->currentPillnum < 8){
                        pillDispenseState = pillDispensed;
                    }else{
                        pillDispenseState = pillNotDispensed;
                    }
                }
                break;
            case pillDispensed:
                pillDroped = false;
                if(motorPos->currentPillnum < 8){
                    sprintf(le->message, "Log: Pill dispensed %d, Pills left on dispenser %d", motorPos->currentPillnum, 7-motorPos->currentPillnum);
                    write_log(le, &motorPos->address);
                    loraMsg(le->message);
                }
                pillDispenseState = continueDispense;
                break;
            case pillNotDispensed:
                while (ledToggleCount < 10 ){
                    if(time_us_64() - lastToggleTime >= TOGGLE_DELAY2){
                        gpio_put(LED_1, !gpio_get(LED_1));
                        lastToggleTime = time_us_64();
                        ledToggleCount++;
                    }
                }
                ledToggleCount = 0;
                if(motorPos->currentPillnum < 8){
                    sprintf(le->message, "Log: Pill Not dispensed %d, Pills left on dispenser %d", motorPos->currentPillnum, 7-motorPos->currentPillnum);
                    write_log(le, &motorPos->address);
                    loraMsg(le->message);
                }
                pillDispenseState = continueDispense;
                break;
        }

    }
    motorPos->revol = 0;
    loraMsg("Pill dispenser empty");
    read_log();
}

void piezoHandler(uint gpio, uint32_t events){
    while(!pillDroped){
        if(gpio_get(gpio) == 0){
            pillDroped = true;
            break;
        }
    }
}