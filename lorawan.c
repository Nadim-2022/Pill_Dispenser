//
// Created by iamna on 08/01/2024.
//
#include "header.h"

typedef enum{
    at,
    lwotaa,
    appkey,
    class,
    port,
    join,
}loraState;
bool lora(){
    const char command0[] = "AT\r\n";
    const char command1[] = "AT+MODE=LWOTAA\r\n";
    const char command2[] = "AT+KEY=APPKEY,\"AD50014F5E3B35A03EF7CF0909B5B975\"\r\n";
    const char command3[] = "AT+CLASS=A\r\n";
    const char command4[] = "AT+PORT=8\r\n";
    const char command5[] = "AT+JOIN\r\n";
    char str[STRLEN];
    loraState loarsState = at;
    int pos = 0;
    switch (loarsState) {
        case at:
            uart_send(UART_NR, command0);
            sleep_ms(500);
            uint32_t t = time_us_32();
            pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);

            printf("Version: %s\n", str);
            if(pos > 0){
                str[pos] = '\0';
                if(strstr(str, "OK") != NULL){
                    loarsState = lwotaa;
                }
            }
         case lwotaa:
            printf("Lwotaa\n");
            uart_send(UART_NR, command1);
            sleep_ms(1000);
             t = time_us_32();
             pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
            printf("Lwotaa: %s\n", str);
            if(pos > 0){
                str[pos] = '\0';
                if(strstr(str, "+MODE: LWOTAA") != NULL){
                    loarsState = appkey;
                }
            }
        case appkey:
            uart_send(UART_NR, command2);
            sleep_ms(500);
            t = time_us_32();
            pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
            if(pos > 0){
                str[pos] = '\0';
                if(strstr(str, "+KEY: APPKEY,AD50014F5E3B35A03EF7CF0909B5B975") != NULL){
                    loarsState = class;
                }

            }
            printf("Appkey: %s\n", str);
        case class:
            uart_send(UART_NR, command3);
            sleep_ms(500);
            t = time_us_32();
            pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
            if(pos > 0){
                str[pos] = '\0';
                if(strstr(str, "+CLASS: A") != NULL){
                    loarsState = port;
                }

            }
            printf("Class: %s\n", str);
        case port:
            uart_send(UART_NR, command4);
            sleep_ms(500);
            pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
            if(pos > 0){
                str[pos] = '\0';
                if(strstr(str, "+PORT: 8") != NULL){
                    loarsState = join;
                }

            }
            printf("Port: %s\n", str);
        case join:
            uart_send(UART_NR, command5);
            sleep_ms(10000);
            pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
            if(pos > 0){
                str[pos] = '\0';
                if(strstr(str, "+JOIN: Network joined") != NULL){
                    printf("Join: %s\n", str);
                    printf("Network joined\n");
                    return true;
                } else{
                    return false;
                }
            }

    }
}

bool loraMsg(const char *message){
    char preMsg[STRLEN];
    strcpy(preMsg,"AT+MSG=\"");
    strcat(preMsg, message);
    strcat(preMsg, "\"\r\n");
    printf("%s",preMsg);

    char str[STRLEN];
    int pos = 0;
    uart_send(UART_NR, preMsg);
    sleep_ms(5000);
    pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
    if(pos){
        str[pos] = '\0';
        if(strstr(str, "+MSG: Done") != NULL){
            return true;
        }
    }
    return false;
}