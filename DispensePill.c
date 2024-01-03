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
typedef enum {
    errorFound,
    startDispense,
    continueDispense,
    pillDispensed,
    pillNotDispensed
}pillDispenseState;
void runDispenser(motor_pos *motorPos, log_entry *le){
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
}
static void isStopedRun(motor_pos *motorPos){
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
void dispensepills(motor_pos *motorPos, log_entry *le) {
    uint64_t start_time = time_us_64();
    uint64_t lastToggleTime = time_us_64();
    int ledToggleCount = 0;
    if(motorPos->recaliberate == true){
        motorPos->recaliberate = false;
        isStopedRun(motorPos);
    }
    static pillDispenseState pillDispenseState = startDispense;
    while (motorPos->currentPillnum < 8){
        switch (pillDispenseState) {
            case startDispense:
                runDispenser(motorPos, le);
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
                /*
                if (time_us_64()-start_time >= DispenTime){
                    pillDroped = false;
                    while (motorPos->currentPillnum < 8){
                        runDispenser(motorPos, le);
                        start_time = time_us_64();
                        while (time_us_64()-start_time < 100000){
                        }
                        motorPos->currentPillnum++;
                        if(pillDroped && motorPos->currentPillnum < 8){
                            pillDispenseState = pillDispensed;
                        }else{
                            pillDispenseState = pillNotDispensed;
                        }
                    }
                }
                 */
                if(time_us_64() - start_time >= DispenTime){
                    runDispenser(motorPos, le);
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
                    sprintf(le->message, "Pill dispensed %d, Pills left on dispenser %d", motorPos->currentPillnum, 7-motorPos->currentPillnum);
                    write_log(le, &motorPos->address);
                    printf("Pill dispensed %d, Pills left on dispenser %d\n", motorPos->currentPillnum, 7-motorPos->currentPillnum);
                }
                //printf("Pill dispensed %d\n", motorPos->currentPillnum);
                pillDispenseState = continueDispense;
                break;
            case pillNotDispensed:
                if(motorPos->currentPillnum < 8){
                    sprintf(le->message, "Pill Not dispensed %d, Pills left on dispenser %d", motorPos->currentPillnum, 7-motorPos->currentPillnum);
                    write_log(le, &motorPos->address);
                    printf("Pill Not dispensed %d, Pills left on dispenser %d \n", motorPos->currentPillnum, 7-motorPos->currentPillnum);
                }
                //printf("Pill not dispensed %d\n", motorPos->currentPillnum);
                ledToggleCount = 0;
                while (ledToggleCount < 10 ){
                    if(time_us_64() - lastToggleTime >= TOGGLE_DELAY){
                        gpio_put(LED_1, !gpio_get(LED_1));
                        lastToggleTime = time_us_64();
                        ledToggleCount++;
                    }
                }
                pillDispenseState = continueDispense;
                break;
            case errorFound:
                break;
        }

    }
    motorPos->revol = 0;
    printf("Dispensing complete\n");
    read_log();
}

void piezoHandler(uint gpio, uint32_t events){
    while(/*time_us_64() - pilstart < 85600 &&*/ !pillDroped){
        if(gpio_get(gpio) == 0){
            printf("Pill dispensed\n");
            pillDroped = true;
            break;
        }
    }
}