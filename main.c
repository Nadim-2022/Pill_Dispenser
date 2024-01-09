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
    motorPos->address = 0;
    motorPos->recaliberate = false;
}
typedef struct boot_state{
    bool state;
    bool not_state;
} boot_state;

void set_boot_state(boot_state *boot_state, bool state){
   boot_state->state = state;
    boot_state->not_state = !state;

}
bool get_boot_state(boot_state *boot_state){
    return boot_state->state == !boot_state->not_state;
}
int main() {
    stdio_init_all();
    init();
    motor_pos motorPos;
    log_entry le;
    //init_motor_pos(&motorPos);
    pil_state pilState = {.state = DisStart};
    boot_state bootState;
    bool loraConn = false;
    /*
    while(!loraConn){
        loraConn = lora();
    }
    loraMsg("Boot");
    */
    eeprom_read_bytes(EEPROM_SIZE-1, (uint8_t*)&bootState, sizeof(boot_state));
    if(!get_boot_state(&bootState)){
        printf("Boot state: %d\n", bootState.state);
        set_boot_state(&bootState, true);
        eeprom_write_bytes(EEPROM_SIZE-1, (uint8_t*)&bootState, sizeof(boot_state));
    }
    read_log();
    erase_log(0);
    eeprom_read_bytes(EEPROM_motorPos-5, (uint8_t*)&motorPos, sizeof(motor_pos));
    //init_motor_pos(&motorPos);
    if(motorPos.currentPillnum == 7){
        motorPos.currentPillnum = 0;
        printf("currentPillnum: %d\n", motorPos.currentPillnum);
    }
    if (bootState.state && motorPos.currentPillnum > 0){
        pilState.state = DisreCalib;
    }
    printf("Current state: %d\n", pilState.state);
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
                    motorPos.currentPillnum = 0;
                    ledToggle = false;
                    gpio_put(LED_1, 0);
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
                    motorPos.currentPillnum = 0;
                    gpio_put(LED_1, 0);
                    dispensepills(&motorPos, &le);
                    calibration = false;
                    ledToggle = true;
                    pilState.state = DisStart;
                }
                break;
            case DisreCalib:
                recalib(&motorPos);
                dispensepills(&motorPos, &le);
                pilState.state = DisStart;
        }
    }
    return 0;
}
