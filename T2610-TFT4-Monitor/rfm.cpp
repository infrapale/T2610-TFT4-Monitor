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
#include "sensor.h"
// #include "clock.h"

void rfm_rx_task(void);
//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st rfm_rx_th         =     { "RFM RX Task    ", 100,    0,     0,  255,    0,  1,  rfm_rx_task };

rfm_st rfm ={0};

void rfm_initialize(void)
{
    // atask_add_new(&rfm_rx_th);   call directly in loop
}

void rfm_rx_task(void)
{
    while (SerialRfm.available()) {
        char c = SerialRfm.read();
        //Serial.printf("%d: %02X %c\n", rfm.rx.pos, c, c);

        switch(c)
        {
            case '\r':
                break;
            case '\n':
                //terminate = true;
                rfm.rx.avail = true;
                rfm.rx.buff[rfm.rx.pos] = 0;
                break;
            default:
                if (rfm.rx.pos < RFM_MAX_MSG_LEN - 1) {
                    rfm.rx.buff[rfm.rx.pos++] = c;
                } else {
                    // overflow protection
                    rfm.rx.buff[RFM_MAX_MSG_LEN - 1] = 0;
                    Serial.println("Overflow!");
                    //rfm_rx_th.state = 100;
                }
                break;
        }

    }
    if(rfm.rx.avail)
    {
        Serial.println(rfm.rx.buff);
        rfm.rx.field_count = msg_split(&rfm.rx, ';');
        // msg_sub_print(&rfm.rx);
        sensor_process_msg(rfm.rx.field_count);
        rfm.rx.avail = false;
        rfm.rx.pos = 0;
        rfm.rx.buff[0] = 0;
    }
}

