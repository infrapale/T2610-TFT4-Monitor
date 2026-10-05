
#ifndef __MSG_H__
#define __MSG_H__

#define MSG_MAX_FIELDS          20
#define MSG_MAX_FIELD_LEN       16
#define MSG_MAX_RAW_MSG_LEN     200
#define MSG_MAX_SMS_CMD_LEN     8
#define MSG_MAX_RFM_MSG_LEN     64

typedef enum 
{
    MSG_FROM_UNDEFINED = 0,
    MSG_FROM_UART,
    MSG_FROM_RFM,
    MSG_FROM_SMS,
    MSG_FROM_NBR_OF
} msg_from_et;

typedef enum
{
    SMS_CMD_HOME = 0,
    SMS_CMD_RELAY_PUMP,
    SMS_CMD_RELAY_PEER,
    SMS_CMD_SENSOR_PIHA1,
    SMS_CMD_SENSOR_REPO1,
    SMS_CMD_SENSOR_REPO2,
    SMS_CMD_ALL_TEMPERATURE,
    SMS_CMD_NBR_OF
} sms_cmd_type_et;


typedef struct 
{
    char        raw[MSG_MAX_RAW_MSG_LEN];
    char        rfm[MSG_MAX_RFM_MSG_LEN];
    msg_from_et from;
    char        fields[MSG_MAX_FIELDS][MSG_MAX_FIELD_LEN];
    uint8_t     field_count;
    bool        rx_msg_avail;
} msg_st;


typedef struct
{
    char cmd[MSG_MAX_SMS_CMD_LEN];
    sms_cmd_type_et type;
} sms_cmd_st;


void msg_initialize(void);

bool msg_is_valid_char(char c);

uint32_t msg_robust_atoi(const char *s, uint8_t *err_cntr, int min, int max );

uint8_t msg_split(char *msg_inp,  char separator ); 



void msg_sub_print(void);

void msg_time_action(void);



#endif

------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <Arduino.h>
#include <time.h>
#include "main.h"
#include "msg.h"
//#include "r69.h"
//#include "lte.h"
#include "atask.h"
//#include "sensor.h"
//#include "clock.h"

void msg_task(void);
//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st msg_th              =    {"Message Task   ", 100,    0,     0,  255,    0,  1,  msg_task };


msg_st msg = {0};

void msg_mod_test(void);

void msg_initialize(void)
{
    atask_add_new(&msg_th);
}

bool msg_is_valid_char(char c) {
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= 'a' && c <= 'z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == ';' || c == '#' || c == '.' || c == '-') return true;
    return false;
}

int msg_strip_to_raw(char *msg_inp)
{
    int len = strlen(msg_inp);
    // Serial.printf("len1: %d ",len);
    if(len == 0 ) return 0;
    int indx = len-1;
    while(msg_inp[indx] == '\n' || msg_inp[indx] == '\r'){
        msg_inp[indx--] = 0x00;
        if(indx < 3) break;
    }
    len = strlen(msg_inp);
    int i = 0;
    while(msg_inp[i] == '\n' || msg_inp[i] == '\r'){
        i++;
        if(i > len -3) break;
    }
    len -= i;
    strncpy(msg.raw, &msg_inp[i],MSG_MAX_RAW_MSG_LEN);
    return len;
}

uint8_t msg_split(char *msg_inp,  char separator = ';') 
{
    // Serial.print("sg_split() ");
    // Must start with '<' and end with '>'

    int len = msg_strip_to_raw(msg_inp);
    // Serial.printf("len1: %d ",len);
    if(len < 3) return 0;
    
    int f = 0;   // field index
    int i = 0;   // 
    int p = 0;   // position inside field

    // Serial.printf("len3: %d ",len);
    // Serial.printf("Start - End: %c %c ",raw_msg[i], raw_msg[len - 1] );
    if (len < 2 || msg.raw[i] != '<' || msg.raw[len - 1] != '>') return 0;
    i++;
    while (i < len - 1 && f < MSG_MAX_FIELDS) {
        char c = msg.raw[i];

        if (c == separator) {
            // End of field
            msg.fields[f][p] = '\0';
            f++;
            p = 0;
        }
        else {
            if (p < MSG_MAX_FIELD_LEN - 1) {
                msg.fields[f][p++] = c;
            }
        }
        i++;
    }

    // Final field
    if (f < MSG_MAX_FIELDS) {
        msg.fields[f][p] = '\0';
        f++;
    }

    // Serial.printf("..end return %d\n",f);
    msg.field_count = f;
    return f;
}

void msg_sub_print(void)
{
    Serial.printf("Message fields; %d\n", msg.field_count);
    for (uint8_t i = 0; i < msg.field_count; i++) {
        Serial.printf("[%d] = %s\n", i, msg.fields[i]);
    }
}


uint32_t timeout;
uint16_t prev_state = 0;

void msg_task(void)
{

    switch(msg_th.state)
    {
        case 0:
            msg_th.state = 10;
            break;
        case 10:
            if (msg.rx_msg_avail) {
                msg_th.state = 20;
            }
            break;
        case 20:
            msg.rx_msg_avail = false;
            Serial.print("UART LINE: ");
            Serial.println(msg.raw);
            msg_split(msg.raw);
            msg_sub_print();
            msg_th.state = 10;
            break;
    }
}



