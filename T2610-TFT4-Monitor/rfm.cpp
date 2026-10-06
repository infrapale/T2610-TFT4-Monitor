#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <Arduino.h>
// #include <time.h>
#include "main.h"
#include "msg.h"
#include "rfm.h"
#include "lte.h"
#include "atask.h"
// #include "sensor.h"
// #include "clock.h"

void rfm_rx_task(void);
//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st rfm_rx_th         =     { "RFM RX Task    ", 100,    0,     0,  255,    0,  1,  rfm_rx_task };

rfm_st rfm ={0};

void rfm_initialize(void)
{
    atask_add_new(&rfm_rx_th);
}

void rfm_rx_task(void)
{
    static  char c;
    switch(rfm_rx_th.state)
    {
        case 0:
            rfm_rx_th.state = 5;
            break;
        case 5:
            rfm.rx.pos = 0;
            rfm_rx_th.state = 10;
            break;
        case 10:
            while (SerialRfm.available()) {
                c = SerialRfm.read();
                //Serial.printf("@%d-%c\n",rfm_pos,c);
                if (c == '\n' || c == '\r') {
                    if (rfm.rx.pos > 0) {
                        rfm.rx.buff[rfm.rx.pos] = 0;
                        //msg.rx_msg_avail = true;   // signal to msg_task()
                        rfm_rx_th.state = 20;
                    }
                } else {
                    if (rfm.rx.pos < RFM_MAX_MSG_LEN - 1) {
                        rfm.rx.buff[rfm.rx.pos++] = c;
                    } else {
                        // overflow protection
                        rfm.rx.buff[RFM_MAX_MSG_LEN - 1] = 0;
                        rfm_rx_th.state = 100;
                    }
                }
            }
            break;
        case 20:
            Serial.println(rfm.rx.buff);
            msg_split(&rfm.rx, ';');
            rfm_rx_th.state = 5;
            break;
        case 100:
            rfm.rx.buff[RFM_MAX_MSG_LEN - 1] = 0;
            Serial.print("RX buffer limit:");
            rfm_rx_th.state = 20;
            break;
        case 200:
            rfm_rx_th.state = 10;
            break;
    }



}