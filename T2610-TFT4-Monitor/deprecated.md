

void dashboard_big_time(void)
{
    if (!box_is_not_reserved()) return;

    uint8_t idx_time_big = box_get_indx(BOX_GROUP_3, 0);
    uint8_t idx_time_head = box_get_indx(BOX_GROUP_8, 0);

    if (!box_index_ok(idx_time_big) || !box_index_ok(idx_time_head)) {
        Serial.printf("dashboard_big_time(): bad idx %d %d\n",
                      idx_time_big, idx_time_head);
        return;
    }

    DateTime *now = time_get_time_now();
    if (!now) {
        Serial.println("RTC ERROR: now == NULL");
        return;
    }

    uint8_t minute = now->minute();
    if (minute == prev_minute) return;
    prev_minute = minute;

    char buff[80];

    time_to_string(buff);
    box_paint(idx_time_head, BOX_SCHEME_TIME);
    box_print_text(idx_time_head, buff);

    snprintf(buff, sizeof(buff), " %02d:%02d", now->hour(), minute);
    box_paint(idx_time_big, BOX_SCHEME_TIME);
    box_print_text(idx_time_big, buff);
}


--------------------------------------------
bool dashboard_show_sensor(void)
{
    if (!box_is_not_reserved()) return false;

    if (!box_valid(BOX_GROUP_3, 1) || !box_valid(BOX_GROUP_8, 3)) {
        Serial.println("dashboard_show_sensor(): boxes not ready");
        return false;
    }

    if (millis() < next_sensor_update) return false;
    next_sensor_update = millis() + 10000;

    char buff[40];

    // Sensor label
    snprintf(buff, sizeof(buff), "Sensor");
    safe_box_print(BOX_GROUP_8, 3, BOX_SCHEME_SENSOR, buff);

    // Sensor value (demo)
    snprintf(buff, sizeof(buff), " %.1f", 3.14);
    safe_box_print(BOX_GROUP_3, 1, BOX_SCHEME_SENSOR, buff);

    return true;
}

void dashboard_show_sensor_print(void){
    dashboard_set_mode(DASHBOARD_BASIC_ROWS);
}
void dashboard_show_time_sensor(void){
    dashboard_set_mode(DASHBOARD_TIME_SENSOR);
}



void dashboard_big_time(void)
{
    if (!box_is_not_reserved()) return;

    if (!box_valid(BOX_GROUP_3, 0) || !box_valid(BOX_GROUP_8, 0)) {
        Serial.println("dashboard_big_time(): boxes not ready");
        return;
    }

    DateTime *now = time_get_time_now();
    if (!now) {
        Serial.println("RTC ERROR: now == NULL");
        return;
    }

    uint8_t minute = now->minute();
    if (minute == prev_minute) return;
    prev_minute = minute;

    char buff[80];

    // Full date/time
    time_to_string(buff);
    safe_box_print(BOX_GROUP_8, 0, BOX_SCHEME_TIME, buff);

    // HH:MM
    snprintf(buff, sizeof(buff), " %02d:%02d", now->hour(), minute);
    safe_box_print(BOX_GROUP_3, 0, BOX_SCHEME_TIME, buff);
}


/******************************************************************************
    dashboard.cpp  Show Time and measurements on the TFT display
*******************************************************************************
    BOX_H0      BOX_H1      BOX_H2      BOX_H4
    ---------   ---------   --------    ---------
    |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |  
    |       |   |       |   |       |   |       |  
    |       |   ---------   ---------   |       |  
    |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |  
    |       |   |       |   |       |   |       |  
    |       |   ---------   ---------   ---------  
    |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |
    |       |   |       |   |       |   |       |  
    |       |   ---------   ---------   |       |  
    |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |  
    |       |   |       |   |       |   |       |  
    ---------   ---------   --------    ---------


******************************************************************************/

//  This sketch uses the GLCD (font 1) and fonts 2, 4, 6, 7, 8

#include  "main.h"
#include  "io.h"
#include  "dashboard.h"
#include  "time_func.h"
#include  "menu.h"
#include  "atask.h"
#include  "box.h"
#include  "sensor.h"

typedef struct
{
    dashboard_mode_et mode;
    bool show_sensor_value;
    bool force_show_big_time;
    bool fast_forward;
    //bool show_basic_rows;
    uint8_t sensor_indx;
    uint8_t menu_sensor_indx;
    uint8_t basic_row_indx;
    bool    basic_row_updated;
} dashboard_ctrl_st;

typedef struct
{
    //uint16_t    state;
    uint16_t    bl_pwm;
    uint32_t    timeout;
    uint16_t    light_state;
    uint16_t    ldr_value;
    uint8_t     pir_value;
} dashboard_backlight_st;

dashboard_ctrl_st dashboard_ctrl    = {DASHBOARD_TIME_SENSOR, false, true, false, 1, 0, 0, false};

dashboard_backlight_st backlight = {0};

//extern value_st subs_data[AIO_SUBS_NBR_OF];

char unit_label[UNIT_NBR_OF][UNIT_LABEL_LEN] =
{
  // 012345678
    "Celsius ",
    "%       ",
    "kPa     ",
    "Light   ",
    "LDR     ",
    "V       ",
    "Time    ",
    "CO2     ",
    "LUX     "
};

char measure_label[UNIT_NBR_OF][MEASURE_LABEL_LEN] =
{
  // 0123456789012345
    "Temperature    ",
    "Humidity       ",
    "Air Pressure   ",
    "Light          ",
    "LDR Value      ",
    "Voltage        "
};

void dashboard_backlight_task(void);

//                      123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st dbh      =   {"Dashboard SM   ", 1000,   0,     0,  255,    0,   1,  dashboard_update_task };
atask_st blh      =   {"Backlight task ", 1000,   0,     0,  255,    0,   1,  dashboard_backlight_task };

void dashboard_clear(void)
{
    if (box_is_not_reserved()){
        uint8_t bindx = box_get_indx(BOX_GROUP_1, 0);
        box_paint(bindx, 0);
        box_show_one(bindx);
    } 
}

void dashboard_initialize(void)
{
    atask_add_new(&dbh);
    atask_add_new(&blh);
    dashboard_clear();
}

void dashboard_big_time(void)
{
    static uint8_t prev_minute = 99;
    char buff[80];

    // Get box indices
    uint8_t bindx = box_get_indx(BOX_GROUP_3, 0);
    uint8_t hindx = box_get_indx(BOX_GROUP_8, 0);

    // Validate indices
    if (bindx == 255 || hindx == 255) {
        Serial.printf("dashboard_big_time(): invalid box index bindx=%d hindx=%d\n",
                      bindx, hindx);
        return;
    }

    // Get current time
    DateTime *now = time_get_time_now();
    if (!now) {
        Serial.println("RTC ERROR: now == NULL");
        return;
    }

    // Update only when minute changes
    uint8_t minute = now->minute();
    if (minute == prev_minute)
        return;

    prev_minute = minute;

    // Print full date/time
    time_to_string(buff);
    box_paint(hindx, BOX_SCHEME_TIME);
    box_print_text(hindx, buff);

    // Print HH:MM
    snprintf(buff, sizeof(buff), " %02d:%02d", now->hour(), minute);
    Serial.printf("dashboard_big_time(): %s\n", buff);

    box_paint(bindx, BOX_SCHEME_TIME);
    box_print_text(bindx, buff);
}


void dashboard_set_mode(dashboard_mode_et new_mode)
{
   dashboard_ctrl.mode = new_mode; 
   dbh.state = 0;
}


void dashboard_show_app_info(void)
{
    // String Str_info = APP_NAME;
    // Str_info += "\n";
    // Str_info += __DATE__;
    // Str_info += __TIME__;

    /// TODO

}
void dashboard_show_info(uint8_t sindx)
{
    char            buff[40];
    uint8_t         bindx = box_get_indx(BOX_GROUP_8, 4);

    time_to_string(buff);
    box_paint(bindx,BOX_SCHEME_TIME);
    box_print_text(bindx, buff);

    bindx++;
    sprintf(buff,"%s %s", "info", "test"
        // subs_data[sindx].location,
        // unit_label[subs_data[sindx].unit_index]
    );
    box_paint(bindx, BOX_SCHEME_SENSOR);
    box_print_text(bindx, buff);

}

bool dasboard_show_sensor(void)
{
    //static uint8_t  middle_big_box = box_get_indx(BOX_GROUP_3, 1);
    static uint32_t next_update = 0;
    uint8_t bindx = box_get_indx(BOX_GROUP_3, 1);
    uint8_t hindx = box_get_indx(BOX_GROUP_8, 3);



    char    buff[40];
    bool    update_box = false;
    int     value_indx = -1;    
    //Serial.printf("middle_big_box: %d\n",middle_big_box);

    if (millis() > next_update)
    {
        next_update = millis() + 10000;
        value_indx = 1;  //sensor_get_next_updated_str(buff, 40);
        if (value_indx >= 0)
        {
            update_box = true;
            box_paint(hindx, BOX_SCHEME_SENSOR);
            box_print_text(hindx, buff);
            sprintf(buff," %.1f", 3.14);
            //sensor_get_value_str(buff,value_indx);
            box_paint(bindx, BOX_SCHEME_SENSOR);
            box_print_text(bindx, buff);


        }
        // if ( subs_data[sindx].updated)
        // {
        //     dashboard_ctrl.show_sensor_value = true;
        //     Serial.print("aio index: "); Serial.print(sindx); 
        //     Serial.println(" = Updated ");
        //     subs_data[sindx].updated = false;

        //     sprintf(buff," %.1f", subs_data[sindx].value);
        //     if(strlen(buff) > 5) sprintf(buff,"%.1f", subs_data[sindx].value);
        //     else if(strlen(buff) > 6) sprintf(buff,"%.0f", subs_data[sindx].value); 
        //     Serial.println(buff);
        //     box_paint(middle_big_box, BOX_SCHEME_SENSOR);
        //     box_print_text(middle_big_box, buff);
        //     update_box = true;

        // }
        // subs_data[sindx].show_next_ms = millis() + subs_data[sindx].show_interval_ms;
        // if (millis() > subs_data[sindx].next_update_limit) subs_data[sindx].state = SENSOR_TIMEOUT;
    }
    return update_box;
}

// bool dasboard_show_sensor(uint8_t sindx)
// {
//     static uint8_t  middle_big_box = box_get_indx(BOX_GROUP_3, 1);
//     char            buff[40];
//     bool            update_box = false;
//     Serial.printf("middle_big_box: %d\n",middle_big_box);
//     if (millis() > subs_data[sindx].show_next_ms)
//     {
//         if ( subs_data[sindx].updated)
//         {
//             dashboard_ctrl.show_sensor_value = true;
//             Serial.print("aio index: "); Serial.print(sindx); 
//             Serial.println(" = Updated ");
//             subs_data[sindx].updated = false;

//             sprintf(buff," %.1f", subs_data[sindx].value);
//             if(strlen(buff) > 5) sprintf(buff,"%.1f", subs_data[sindx].value);
//             else if(strlen(buff) > 6) sprintf(buff,"%.0f", subs_data[sindx].value); 
//             Serial.println(buff);
//             box_paint(middle_big_box, BOX_SCHEME_SENSOR);
//             box_print_text(middle_big_box, buff);
//             update_box = true;

//         }
//         subs_data[sindx].show_next_ms = millis() + subs_data[sindx].show_interval_ms;
//         if (millis() > subs_data[sindx].next_update_limit) subs_data[sindx].state = SENSOR_TIMEOUT;
//     }
//     return update_box;
// }


void dashboard_update_task(void)
{
    static uint32_t next_step_ms;
    bool            update_box;

    switch (dbh.state)
    {
        case 0:
            box_clear_group(BOX_GROUP_1);
            dbh.state = 10;
            break;
        case 10: 
            dashboard_big_time();
            dbh.state = 20;
            break;
        case 20:
            update_box = dasboard_show_sensor();

            if (update_box )
            {
                dbh.state = 30;
                //dashboard_show_info((uint8_t)dashboard_ctrl.sensor_indx);
                next_step_ms = millis() + 10000;
            }
            else
            {
               dbh.state = 10;
            }           
            break;  
        case 30:

            if (millis() > next_step_ms)
            {
                dbh.state = 10;
                box_update_area();
            } 
            break;
    }
    //Serial.printf("db %d -> %d\n", dbh.prev_state, dbh.state);
}



void dashboard_show_sensor_print(void){
    dashboard_set_mode(DASHBOARD_BASIC_ROWS);
}
void dashboard_show_time_sensor(void){
    dashboard_set_mode(DASHBOARD_TIME_SENSOR);
}


void dashboard_next_sensor(void)
{
    // dashboard_ctrl.menu_sensor_indx++;
    // if(dashboard_ctrl.menu_sensor_indx >= AIO_SUBS_NBR_OF) dashboard_ctrl.menu_sensor_indx = AIO_SUBS_FIRST;
    // subs_data[dashboard_ctrl.menu_sensor_indx].show_next_ms = 0              ;
    // dashboard_ctrl.sensor_indx = dashboard_ctrl.menu_sensor_indx;
    // Serial.printf("dashboard_ctrl.menu_sensor_indx=%d\n",dashboard_ctrl.menu_sensor_indx);
    // dashboard_ctrl.fast_forward = true;
}

void dashboard_previous_sensor(void)
{
    // if(dashboard_ctrl.menu_sensor_indx <= 1 ) dashboard_ctrl.menu_sensor_indx = AIO_SUBS_NBR_OF -1;
    // else dashboard_ctrl.menu_sensor_indx--;
    // subs_data[dashboard_ctrl.menu_sensor_indx].show_next_ms = 0;
    // dashboard_ctrl.sensor_indx = dashboard_ctrl.menu_sensor_indx;
    // Serial.printf("dashboard_ctrl.menu_sensor_indx=%d\n",dashboard_ctrl.menu_sensor_indx);
    // dashboard_ctrl.fast_forward = true;
}

void dashboard_debug_print(void)
{
    Serial.printf("LDR: %d PIR %d PWM %d\n", backlight.ldr_value, backlight.pir_value, backlight.bl_pwm);
}

void dashboard_backlight_task(void)
{
    backlight.ldr_value = analogRead(PIN_LDR_AN);
    backlight.pir_value = digitalRead(PIN_PIR);

    switch(blh.state)
    {
        case 0:
            blh.state = 10; //dark
            break;
        case 10:
            if(backlight.ldr_value > 3400 ) blh.state = 100;
            else if(backlight.ldr_value > 3000 ) blh.state = 100;
            else if(backlight.ldr_value > 2000 ) blh.state = 200;
            else blh.state = 100;
            break;
        case 100: 
            if (backlight.pir_value){
                backlight.bl_pwm = 50;
                backlight.timeout = millis() + 10000; 
                blh.state = 105;
            } 
            else {
                backlight.bl_pwm = 30;           
                blh.state = 10;
            }
            break;
        case 105:
            if (millis() > backlight.timeout) blh.state = 10;
            break;
        case 200: 
            if (backlight.pir_value){
                backlight.bl_pwm = 120;
                backlight.timeout = millis() + 10000; 
                blh.state = 205;
            } 
            else {
                backlight.bl_pwm = 80;           
                blh.state = 10;
            }
            break;
        case 205:
            if (millis() > backlight.timeout) blh.state = 10;
            break;
        case 300: 
            if (backlight.pir_value){
                backlight.bl_pwm = 1023;
                backlight.timeout = millis() + 10000; 
                blh.state = 305;
            } 
            else {
                backlight.bl_pwm = 200;           
                blh.state = 10;
            }
            break;
        case 305:
            if (millis() > backlight.timeout) blh.state = 10;
            break;
    }
    analogWrite(PIN_TFT_LED, backlight.bl_pwm);
    //analogWrite(PIN_TFT_LED, 200);

}

----------------------------------

void dashboard_big_time(void)
{
    static uint8_t prev_minute = 99;
    char buff[80];
    uint8_t bindx = box_get_indx(BOX_GROUP_3, 0);
    uint8_t hindx = box_get_indx(BOX_GROUP_8, 0);

    Serial.printf("Big TIme: %d %d \n", bindx, hindx);

    DateTime *now = time_get_time_now();
    if (!now) {
        Serial.println("RTC ERROR: now == NULL");
    }
    else{
        if (now->minute() != prev_minute)
        {
            time_to_string(buff);
            box_paint(hindx, BOX_SCHEME_TIME);
            box_print_text(hindx, buff);

            sprintf(buff," %02d:%02d", now->hour(), now->minute());
            //sprintf(buff," %02d:%02d", 12, 34);
            Serial.printf("dashboard_big_time(): %s\n",buff);
            prev_minute = now->minute();
            box_paint(bindx, BOX_SCHEME_TIME);
            box_print_text(bindx, buff);
        }
    }
}


/*******************************************************************************
clock.cpp
********************************************************************************
Radio message:
  <T;#;PING;2026;07;10;05;55>

********************************************************************************

*******************************************************************************/

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "clock.h"
#include "main.h"
#include "sensor.h"
#include "msg.h"
#include "rfm.h"

#include "atask.h"

extern rfm_st rfm;
extern main_ctrl_st main_ctrl;

void clock_task(void);

clock_st clock_mgr;


// atask_st:            = {"Label          ", ival, next, state, prev, cntr, run, task_ptr };
atask_st clock_th       = {"Clock Task     ", 1000,    0,     0,  255,    0,   1, clock_task};

void clock_initialize(void)
{
    atask_add_new(&clock_th);
    clock_mgr.time_epoch = 0;  
    memset(&clock_mgr.my_time, 0, sizeof(clock_mgr.my_time));

    clock_mgr.next_minute = millis() + 60000;
    clock_mgr.last_hour = 0;
    clock_mgr.new_hour_event = false;
}

void clock_print_date_time(const struct tm *t)
{ 
    char buff[40];
    //strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S (%Z)", t);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", t);
    Serial.println(buff);
    
}
void clock_print_my_time(void)
{
    clock_print_date_time(&clock_mgr.my_time);
}

void clock_set_date_time(void)
{
    struct tm tmp_time = {0};
    Serial.println("clock_set_date_time");

    uint8_t errors = 0;
    Serial.printf("Time: %s\n", rfm.rx.buff);

    if ((rfm.rx.field_count == 8) && (rfm.rx.field[1][0] == '#'))
    {
        // Year: 2026 → tm_year = 126
        tmp_time.tm_year = (uint16_t)msg_robust_atoi(rfm.rx.field[3], &errors, 2000, 2100) - 1900;

        // Month: 1–12 in message → 0–11 in tm
        uint8_t month = (uint8_t)msg_robust_atoi(rfm.rx.field[4], &errors, 1, 12);
        tmp_time.tm_mon = month - 1;

        tmp_time.tm_mday = (uint8_t)msg_robust_atoi(rfm.rx.field[5], &errors, 1, 31);
        tmp_time.tm_hour = (uint8_t)msg_robust_atoi(rfm.rx.field[6], &errors, 0, 23);
        tmp_time.tm_min  = (uint8_t)msg_robust_atoi(rfm.rx.field[7], &errors, 0, 59);
        tmp_time.tm_sec  = 0;
        tmp_time.tm_isdst = -1;   // let mktime figure it out

        Serial.printf("time errors %d\n", errors);
        clock_print_date_time(&tmp_time);

        if (errors == 0) {
            time_t t = mktime(&tmp_time);   // normalize into time_t

            if (t == (time_t)-1) {
                Serial.println("mktime failed (out of range)");
            } else {
                // Store both time_t and normalized tm
                clock_mgr.time_epoch = t;
                clock_mgr.my_time    = tmp_time;
                clock_print_date_time(&clock_mgr.my_time);
            }
        } else {
            Serial.println("!!!msg_time_action: Integer conversion Error");
        }
    }
    else {
        Serial.println("Incorrect Time message");
    }

    clock_mgr.next_minute = millis() + 60000;
}


void clock_task(void)
{
    if (millis() > clock_mgr.next_minute)
    {
        clock_mgr.next_minute += 60000;   // real minute; use 6000 only for testing

        // Advance canonical epoch time by one minute
        clock_mgr.time_epoch += 60;

        if (clock_mgr.time_epoch < 0 || clock_mgr.time_epoch > 4102444800) {
            // 2100-01-01 safety range
            return;
        }
        // Derive broken-down time from epoch
        struct tm *lt = localtime(&clock_mgr.time_epoch);
        if (lt != nullptr) {
            clock_mgr.my_time = *lt;
            //clock_print_date_time(&clock_mgr.my_time);
        }

    }

    if (clock_mgr.last_hour != clock_mgr.my_time.tm_hour)
    {
        clock_mgr.last_hour = clock_mgr.my_time.tm_hour;
        Serial.println("@ @ @ New Hour @ @ @");
        clock_print_my_time();
        switch(clock_mgr.my_time.tm_hour)
        {
            case 8:
                Serial.println("* * * Send Report * * *");
                msg_send_all_temp();
                sensor_clear_all();
                break;
        }
    }
}


---------------------------------------------------------------------------------------------------------------


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



