#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <Arduino.h>
#include <time.h>
#include "main.h"
#include "msg.h"
#include "r69.h"
#include "io.h"
#include "atask.h"
//#include "sensor.h"
#include "clock.h"


extern main_ctrl_st main_ctrl;

msg_st msg = {0};


void msg_initialize(void)
{
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

// <S;PIHA1;T;25.3;H;43;L;9>
// Split nbr 8
// Message fields; 8
// [0] = S
// [1] = PIHA1
// [2] = T
// [3] = 25.3
// [4] = H
// [5] = 43
// [6] = L
// [7] = 9




void str_to_upper(char str[]) {
    uint16_t i = 0;
    while (str[i] != '\0') { 
        str[i] = toupper(str[i]); // Convert character to uppercase
        i++;
    }
}


uint32_t msg_robust_atoi(const char *s, uint8_t *err_cntr, int min, int max)
{
    char *endptr;
    long val = 0;

    if (s == NULL) *err_cntr++;
    if (*err_cntr == 0) val = strtol(s, &endptr, 10);
    if (endptr == s) *err_cntr++;
    if (val > max)  *err_cntr++;
    if (val < min)  *err_cntr++;
    if (*err_cntr > 0) val = 0;
    Serial.printf("%s %d !%d! ",s,val, *err_cntr );
    return val;
}



void  msg_time_action(void)
{
    // uint8_t errors = 0;
    // Serial.printf("Time: %s\n", msg.raw);
    // if((msg.field_count == 8) && (msg.fields[1][0] == '#'))
    // {
    //     main_ctrl.timeinfo.tm_year  = (uint16_t)msg_robust_atoi(msg.fields[3],&errors,2000,2100) -1900;    
    //     main_ctrl.timeinfo.tm_mon   = (uint8_t)msg_robust_atoi(msg.fields[4],&errors,1,12);    
    //     main_ctrl.timeinfo.tm_mday  = (uint8_t)msg_robust_atoi(msg.fields[5],&errors,1,31);
    //     main_ctrl.timeinfo.tm_hour  = (uint8_t)msg_robust_atoi(msg.fields[6],&errors,0,24);
    //     main_ctrl.timeinfo.tm_min   = (uint8_t)msg_robust_atoi(msg.fields[7],&errors,0,60);
    //     Serial.printf("time errors %d\n",errors);
    //     if (errors==0) clock_set_date_time();
    //     else Serial.println("!!!msg_time_action: Integer conversion Error");
    // }
    // else {
    //     Serial.println("Incorrect Time message");
    // }

    clock_set_date_time();
}


