
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "header.h"
#include "watchdog.h"
// Run the motor
void run(motor_pos *motorPos, int start, bool startCount, bool clockwiseDirection) {
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
    watchdog_feed();
}
/*
typedef struct {
    int extra;
    int oneStep;
}extraAlignState;

void countExtraAlign(motor_pos *motorPos,bool clockwiseDirection, extraAlignState *alignState){
    printf("%d\n", clockwiseDirection);
    int stepIncrement = clockwiseDirection ? 1 : -1;
    while (alignState->oneStep < motorPos->microstep || alignState->extra >= 0) {
        for (int i = motorPos->pos; i < 8 && i >= 0; i+= stepIncrement) {
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);
            motorPos->pos += stepIncrement;
            if(clockwiseDirection){
                if(gpio_get(GPIO_Opto) == 0){
                    alignState->extra++;
                }
                alignState->oneStep++;
            }else{
                printf("Decrease\n");
                alignState->oneStep=0;
                alignState->extra--;
                printf("extra: %d\n", alignState->extra);
            }
            if (motorPos->pos == 8) {
                motorPos->pos = 0;
            } else if (motorPos->pos < 0) {
                motorPos->pos = 7;
            }
            if (alignState->oneStep == motorPos->microstep) {
                printf("extra: %d\n", alignState->extra);
                break;
            }
            if(alignState->extra == 0){
                printf("Break\n");
                break;
            }
            printf("oneStep: %d, microstep: %d\n", alignState->oneStep, motorPos->microstep);
        }
        if (alignState->oneStep == motorPos->microstep) {
            printf("extra: %d\n", alignState->extra);
            break;
        }
    }
}
 */

// Calibrate the motor
void calib(motor_pos *motorPos){
    run(motorPos, 0, false, true);
    run(motorPos, 1, false, true);
    for (int i = 0; i < Calicarate_count; i++) {
        run(motorPos, 0,true, true);
        run(motorPos, 1, true, true);
        sleep_ms(100);
    }
    motorPos->microstep = (motorPos->revol / Calicarate_count) / 8;
    int startPositioning = motorPos->microstep;
    int extra = 0;
    int goStepForward = 0;
    run(motorPos, 0, false, false);
    sleep_ms(500);
    while (goStepForward < startPositioning){
        for (int i = motorPos->pos; i < 8; i++) {
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
            if (motorPos->pos == 8) {
                motorPos->pos = 0;
            }
            if (goStepForward == startPositioning) {
                break;
            }
        }
    }
    watchdog_feed();
    while (extra!=0){
        for (int i = motorPos->pos; i >= 0; i--) {
            gpio_put(STPER_GP2, clockwise[i][0]);
            gpio_put(STPER_GP3, clockwise[i][1]);
            gpio_put(STPER_GP6, clockwise[i][2]);
            gpio_put(STPER_GP13, clockwise[i][3]);
            sleep_ms(MOTOR_DELAY);
            motorPos->pos--;
            extra--;
            if(motorPos->pos < 0){
                motorPos->pos = 7;
            }
            if(extra== 0){
                break;
            }
        }
    }
    watchdog_feed();

}

void recalib(motor_pos *motorPos){
    sleep_ms(1000);
    run(motorPos, 1,false, false);
    sleep_ms(1000);
    run(motorPos, 0,false, false);
    int step_1 = 0;
    int extra = 0;
    sleep_ms(1000);
    while (step_1 < motorPos->microstep){
        for (int i = motorPos->pos; i < 8; i++) {
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
            if (motorPos->pos == 8) {
                motorPos->pos = 0;
            }
            if (step_1 == motorPos->microstep) {
                break;
            }
        }
    }
    watchdog_feed();
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
    sleep_ms(500);
    watchdog_feed();
}
