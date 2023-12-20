//
// Created by iamna on 20/12/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "header.h"
/*
void piezoHandler(uint gpio, uint32_t events){
    uint64_t start = time_us_64();
    while(time_us_64() - start < 85600){
        if(gpio_get(gpio) == 0){
            printf("Pill dispensed\n");
            break;
        }
    }
}
 */