//
// Created by iamna on 20/12/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "header.h"



void run(motor_pos *motorPos, int start, int end, bool startCount){
    while(gpio_get(GPIO_Opto) == start){
        for(int i = motorPos->pos; i < 8; i++){
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);
            motorPos->pos++;
            if(startCount){
                motorPos->revol++;
            }
            //motorPos->revol++;
            if(motorPos->pos == 8){
                motorPos->pos = 0;
            }
            if(gpio_get(GPIO_Opto) != start){
                break;
            }
        }
    }
    printf("startCount: %d\n", startCount);
}
void rerun(motor_pos *motorPos, int start, int end, bool startCount , int *extra){
    while(gpio_get(GPIO_Opto) == start){
        for(int i = motorPos->pos; i < 8; i++){
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);
            motorPos->pos++;
            *extra++;
            //motorPos->revol++;
            if(motorPos->pos == 8){
                motorPos->pos = 0;
            }
            if(gpio_get(GPIO_Opto) != start){
                break;
            }
        }
    }
    printf("REstartCount: %d\n", startCount);
}

void calib(motor_pos *motorPos){
    //int start = gpio_get(GPIO_Opto);
    bool startCount = false;
    run(motorPos, 0, 8, startCount);
    run(motorPos, 1, 8, startCount);

    sleep_ms(1000);
    for (int i = 0; i < 1; i++) {
       // printf("start: %d\n", start);
        run(motorPos, 0, 8, startCount = true);
        run(motorPos, 1, 8, startCount = true);
        sleep_ms(1000);

    }
    motorPos->microstep = (motorPos->revol / 1) / 8;
    printf("Calibration done\n");
    printf("Total revolution: %d\n", motorPos->revol);
    printf("Total microstep: %d\n", motorPos->microstep);
    int startPositioning = motorPos->microstep;
    printf("startPositioning: %d\n", startPositioning);
    printf("Current Position %d\n", motorPos->pos);

    //positionIt(motorPos);a
    sleep_ms(1000);
    int extra = 0;
    printf("Starting the positionig\n");
    /*
    while (gpio_get(GPIO_Opto)==0){
        for (int i = motorPos->pos; i < 8; i++) {
            //printf("Current Position %d\n", motorPos->pos);
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            motorPos->pos++;
            sleep_ms(2);
            if (motorPos->pos == 8) {
                motorPos->pos = 0;
            }
            if (gpio_get(GPIO_Opto) == 1) {
                break;
            }
        }
    }
    sleep_ms(500);
    int step_1 = 0;
    while (step_1 < startPositioning){
        for (int i = motorPos->pos; i >= 0; i--) {
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(1);
            motorPos->pos--;
            step_1++;
            if(gpio_get(GPIO_Opto) == 0){
                extra++;
            }
            if(motorPos->pos < 0){
                motorPos->pos = 7;
            }
            if(step_1 == startPositioning){
                break;
            }
        }
    }
    sleep_ms(500);
    printf("extra: %d\n", extra);
    while (extra != 0){
        for (int i = motorPos->pos; i < 8; i++) {
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(1);
            motorPos->pos++;
            extra--;
            if (motorPos->pos == 8) {
                motorPos->pos = 0;
            }
            if(extra == 0){
                break;
            }
        }
    }
    */


    while(gpio_get(GPIO_Opto) == 0){
        printf("Current Position\n");
        for (int i = motorPos->pos; i >= 0; i--) {
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);
            motorPos->pos--;
            //extra++;
            if(motorPos->pos < 0){
                motorPos->pos = 7;
            }
            if(gpio_get(GPIO_Opto) == 1){
                break;
            }
        }
    }
    sleep_ms(500);
    int step_1 = 0;
    while (step_1 < startPositioning){
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
            sleep_ms(4);
            if (motorPos->pos == 8) {
                motorPos->pos = 0;
            }
            if (step_1 == startPositioning) {
                break;
            }
        }
    }

    printf("extra: %d\n", extra);
    int extra_revol = (extra/2);
    printf("Current Position %d\n", motorPos->pos);
    printf("Total revolution: %d\n", motorPos->revol);
    printf("Extra revolution %d\n", extra_revol);
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

    //printf("Extra revolution %d\n", extra_revol);

}

void recalib(motor_pos *motorPos){
    int count = 0;
    while(gpio_get(GPIO_Opto) == 1){
        //printf("Current Position\n");
        for (int i = motorPos->pos; i >= 0; i--) {
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);
            motorPos->pos--;
            count++;
            //extra++;
            if(motorPos->pos < 0){
                motorPos->pos = 7;
            }
            if(gpio_get(GPIO_Opto) == 0){
                break;
            }
        }
    }
    printf("Current Position %d\n", motorPos->pos);
    while(gpio_get(GPIO_Opto) == 0){
        //printf("Current Position\n");
        for (int i = motorPos->pos; i >= 0; i--) {
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);
            motorPos->pos--;
            //extra++;
            if(motorPos->pos < 0){
                motorPos->pos = 7;
            }
            if(gpio_get(GPIO_Opto) == 1){
                break;
            }
        }
    }

    int step_1 = 0;
    int extra = 0;
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
}