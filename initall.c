//
// Created by iamna on 20/12/2023.
//

#include "pico/stdlib.h"
#include "header.h"


void init(){
    // Stepper GPIO
    gpio_init(STPER_GP2);
    gpio_init(STPER_GP3);
    gpio_init(STPER_GP6);
    gpio_init(STPER_GP13);
    gpio_set_dir(STPER_GP2, GPIO_OUT);
    gpio_set_dir(STPER_GP3, GPIO_OUT);
    gpio_set_dir(STPER_GP6, GPIO_OUT);
    gpio_set_dir(STPER_GP13, GPIO_OUT);

    // LED & Button GPIO
    gpio_init(LED_1);
    gpio_init(LED_2);
    gpio_init(LED_3);
    gpio_init(BTN_1);
    gpio_init(BTN_2);
    gpio_init(BTN_3);
    gpio_set_dir(LED_1, GPIO_OUT);
    gpio_set_dir(LED_2, GPIO_OUT);
    gpio_set_dir(LED_3, GPIO_OUT);
    gpio_set_dir(BTN_1, GPIO_IN);
    gpio_set_dir(BTN_2, GPIO_IN);
    gpio_set_dir(BTN_3, GPIO_IN);
    gpio_pull_up(BTN_1);
    gpio_pull_up(BTN_2);
    gpio_pull_up(BTN_3);

    // Prizo & Opto GPIO
    gpio_init(GPIO_Piezo);
    gpio_init(GPIO_Opto);
    gpio_set_dir(GPIO_Piezo, GPIO_IN);
    gpio_set_dir(GPIO_Opto, GPIO_IN);
    gpio_pull_up(GPIO_Opto);
    gpio_pull_up(GPIO_Piezo);

    // I2C
    i2c_init(i2c0, 100000);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);


}