#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <Arduino.h>
// #include <time.h>
#include "main.h"
#include "uart.h"
#include "msg.h"
#include "io.h"
#include "atask.h"


uart_st uart_tft = {0};

void uart_tft_rx_task(void);
//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st tft_rx_th         =     { "TFT UART RX    ", 100,    0,     0,  255,    0,  1,  uart_tft_rx_task };


void uart_initialize(void)
{
    uart_tft.tx.avail = true;
    uart_tft.rx.avail = false;
    atask_add_new(&tft_rx_th);

}

void uart_tft_send(char *buff)
{
    strncpy(uart_tft.tx.buff, buff, UART_BUFF_LEN);
    SerialTft.println(uart_tft.tx.buff);
    Serial.printf("->TFT: %s\n",uart_tft.tx.buff);
}

bool uart_tft_rx_avail(void){
    return uart_tft.rx.avail;
}

bool uart_tft_read(char *buff){
    if(uart_tft.rx.avail){
        uart_tft.rx.avail = false;
        strncpy(buff, uart_tft.rx.buff,UART_BUFF_LEN);
        return true;
    } else {
        return false;
    }
}

void uart_tft_rx_task(void)
{
    static  char c;
    switch(tft_rx_th.state)
    {
        case 0:
            tft_rx_th.state = 5;
            break;
        case 5:
            uart_tft.rx.pos = 0;
            uart_tft.rx.avail = false;
            tft_rx_th.state = 10;
            break;
        case 10:
            while (SerialTft.available()) {
                c = SerialTft.read();
                //Serial.printf("@%d-%c\n",rfm_pos,c);
                if (c == '\n' || c == '\r') {
                    if (uart_tft.rx.pos > 0) {
                        uart_tft.rx.buff[uart_tft.rx.pos] = 0;
                        uart_tft.rx.avail = true;   // signal to msg_task()
                        tft_rx_th.state = 20;
                        //Serial.println(uart_tft.rx.buff);
                    }
                } else {
                    if (uart_tft.rx.pos < UART_BUFF_LEN - 1) {
                        uart_tft.rx.buff[uart_tft.rx.pos++] = c;
                    } else {
                        // overflow protection
                        uart_tft.rx.buff[UART_BUFF_LEN - 1] = 0;
                        tft_rx_th.state = 100;
                    }
                }
            }
            break;
        case 20:
            Serial.println(uart_tft.rx.buff); Serial.flush();
            uart_tft.rx.field_count = msg_split(uart_tft.rx.buff, ';');
            //sensor_process_msg(uart_tft.rx.field_count);
            tft_rx_th.state = 5;
            break;
        case 100:
            uart_tft.rx.buff[UART_BUFF_LEN - 1] = 0;
            Serial.print("RX buffer limit:");
            tft_rx_th.state = 20;
            break;
        case 200:
            tft_rx_th.state = 10;
            break;
    }

}