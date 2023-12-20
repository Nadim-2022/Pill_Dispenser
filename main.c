#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "header.h"



void init_motor_pos(motor_pos *motorPos) {
    motorPos->pos = 0;
    motorPos->revol = 0;
    motorPos->microstep = 0;
    motorPos->currentPillnum = 0;// 0 = no pill, 1 = pill
}
typedef struct boot_state{
    bool state;
} boot_state;

int main() {
    stdio_init_all();
    init();
    motor_pos motorPos;
    pil_state pilState = {.state = DisStart};
    boot_state bootState = {.state = true};
    eeprom_write_bytes(EEPROM_SIZE-1, (uint8_t*)&bootState, sizeof(boot_state));
    init_motor_pos(&motorPos);
    gpio_set_irq_enabled_with_callback(GPIO_Piezo, GPIO_IRQ_EDGE_FALL, true, &piezoHandler);
    uint64_t lastToggleTime = time_us_64();
    bool ledToggle = true;
    bool calibration = false;
    while (true){
        switch (pilState.state) {
            case DisStart:
                if (ledToggle && (time_us_32() - lastToggleTime >= TOGGLE_DELAY)) {
                    gpio_put(LED_1, !gpio_get(LED_1));
                    lastToggleTime = time_us_64();
                }
                if (!gpio_get(BTN_1)) {
                    motorPos.revol = 0;
                    printf("Current revolution %d\n", motorPos.revol);
                    ledToggle = false;
                    gpio_put(LED_1, 0);
                    printf("Button 1 pressed\n");
                    sleep_ms(100);
                    pilState.state = DisCalib;
                }
                break;
            case DisCalib:
                if (!calibration) {
                    calib(&motorPos);
                    calibration = true;
                    gpio_put(LED_1, 1);
                    pilState.state = DisRun;
                }
                break;
            case DisRun:
                if(!gpio_get(BTN_2)){
                    gpio_put(LED_1, 0);
                    dispensepills(&motorPos);
                    calibration = false;
                    ledToggle = true;
                    pilState.state = DisStart;
                }
                break;
        }
    }
    return 0;
}
