//
// Created by iamna on 20/12/2023.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "header.h"


bool pill = false;

void dispensepills(motor_pos *motorPos) {
    int currentmicrostep = 0;
    uint64_t start_time = time_us_64();
    uint64_t lastToggleTime = time_us_64();
    int ledToggleCount = 0;
    while(motorPos->currentPillnum < 8){
        if (time_us_64()-start_time >= DispenTime){
            pill = true;
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
                //pillDispensed(GPIO_Piezo, 0);

            }
            start_time = time_us_64();
            currentmicrostep = 0;
            motorPos->currentPillnum++;
            printf("Current Pillnum %d\n", motorPos->currentPillnum);
            ledToggleCount = 0;

        }
        if(time_us_64()-start_time > 100000 && pill){
            if (ledToggleCount < 10 && (time_us_64() - lastToggleTime >= TOGGLE_DELAY)){
                printf("Pill not dispensed\n");
                gpio_put(LED_1, !gpio_get(LED_1));
                lastToggleTime = time_us_64();
                ledToggleCount++;
            }

        }
    }
    motorPos->currentPillnum = 0;
    currentmicrostep = 0;
    int toTheStep = motorPos->microstep*8;
    while (currentmicrostep < toTheStep){
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
            if (currentmicrostep == toTheStep) {
                break;
            }
        }
    }
    motorPos->revol = 0;
    printf("Dispensing complete\n");
}

void piezoHandler(uint gpio, uint32_t events){
    uint64_t start = time_us_64();
    while(time_us_64() - start < 85600 && pill){
        if(gpio_get(gpio) == 0){
            printf("Pill dispensed\n");
            pill = false;
            break;
        }
    }
}