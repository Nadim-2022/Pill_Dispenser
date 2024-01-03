//
// Created by iamna on 01/01/2024.
//
//
// Created by iamna on 20/12/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "header.h"

// Run the motor
void run(motor_pos *motorPos, int start, int end, bool startCount, bool clockwiseDirection) {
    int stepIncrement = clockwiseDirection ? 1 : -1;

    while (gpio_get(GPIO_Opto) == start) {
        for (int i = motorPos->pos; i >= 0 && i < 8; i += stepIncrement) {
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);

            motorPos->pos += stepIncrement;
            if (startCount) {
                motorPos->revol++;
            }
            // Handle circular motion
            if (motorPos->pos == 8) {
                motorPos->pos = 0;
            } else if (motorPos->pos < 0) {
                motorPos->pos = 7;
            }

            // Check for opto sensor state change
            if (gpio_get(GPIO_Opto) != start) {
                break;
            }
        }
    }
}

// Calibrate the motor
void calib(motor_pos *motorPos){
    bool startCount = false;
    run(motorPos, 0, 1, false, true);
    run(motorPos, 1, 0, false, true);
    for (int i = 0; i < Calicarate_count; i++) {
        run(motorPos, 0, 1, true, true);
        run(motorPos, 1, 0, true, true);
        sleep_ms(100);
    }
    printf("Current revolution %d\n", motorPos->revol/Calicarate_count);
    motorPos->microstep = (motorPos->revol / Calicarate_count) / 8;
    int startPositioning = motorPos->microstep;
    int extra = 0;
    int goStepForward = 0;
    run(motorPos, 0, 1, false, false);
    sleep_ms(500);
    while (goStepForward < startPositioning){
        for (int i = motorPos->pos; i < 8; i++) {
            //printf("Current Position %d\n", motorPos->pos);
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);
            motorPos->pos++;
            goStepForward++;
            if(gpio_get(GPIO_Opto) == 0){
                extra++;
            }
            sleep_ms(4);
            if (motorPos->pos == 8) {
                motorPos->pos = 0;
            }
            if (goStepForward == startPositioning) {
                break;
            }
        }
    }

    printf("extra: %d\n", extra);

    while (extra!=0){
        for (int i = motorPos->pos; i >= 0; i--) {
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);
            motorPos->pos--;
            extra--;
            //extra++;
            if(motorPos->pos < 0){
                motorPos->pos = 7;
            }
            if(extra== 0){
                break;
            }
        }
    }

}

void recalib(motor_pos *motorPos){
    sleep_ms(1000);
    run(motorPos, 1, 0, false, false);
    sleep_ms(1000);
    run(motorPos, 0, 1, false, false);
    int step_1 = 0;
    int extra = 0;
    sleep_ms(1000);
    while (step_1 < motorPos->microstep){
        for (int i = motorPos->pos; i < 8; i++) {
            //printf("Current Position %d\n", motorPos->pos);
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);
            motorPos->pos++;
            step_1++;
            if(gpio_get(GPIO_Opto) == 0){
                extra++;
            }
            sleep_ms(MOTOR_DELAY);
            if (motorPos->pos == 8) {
                motorPos->pos = 0;
            }
            if (step_1 == 512) {
                break;
            }
        }
    }
    printf("extra: %d\n", extra);
    sleep_ms(1000);
    while(extra != 0){
        for (int i = motorPos->pos; i >= 0; i--) {
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);
            motorPos->pos--;
            extra--;
            //extra++;
            if(motorPos->pos < 0){
                motorPos->pos = 7;
            }
            if(extra== 0){
                break;
            }
        }
    }
    motorPos->recaliberate = true;
    sleep_ms(2000);
}
