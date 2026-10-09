#include "dashboard.h"
#include "io.h"
#include "box.h"
#include "time_func.h"
#include "sensor.h"
#include "atask.h"
#include "main.h"
#include "clock.h"
#include <Arduino.h>

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

extern clock_st clock_mgr;   // from clock.cpp
//extern void tm_to_string(const struct tm *t, char *buff, size_t len);
extern box_group_st boxgr[];

static uint8_t prev_minute = 99;
static uint32_t next_sensor_update = 0;

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

void dashboard_set_mode(dashboard_mode_et new_mode)
{
   dashboard_ctrl.mode = new_mode; 
   dbh.state = 0;
}


static bool box_valid(uint8_t group, uint8_t bindx)
{
    if (group >= BOX_GROUP_NBR_OF) return false;
    if (bindx >= boxgr[group].nbr) return false;
    uint8_t idx = box_get_indx(group, bindx);
    if (idx >= BOX_MAX_NUMBER) return false;
    return true;
}

static void safe_box_print(uint8_t group, uint8_t bindx, uint8_t scheme, const char *txt)
{
    if (!box_valid(group, bindx)) {
        Serial.printf("Invalid box: group=%d bindx=%d\n", group, bindx);
        return;
    }
    uint8_t idx = box_get_indx(group, bindx);
    box_paint(idx, scheme);
    box_print_text(idx, (char*)txt);
}


void dashboard_show_sensor_print(void){
    dashboard_set_mode(DASHBOARD_BASIC_ROWS);
}
void dashboard_show_time_sensor(void){
    dashboard_set_mode(DASHBOARD_TIME_SENSOR);
}

static bool box_index_ok(uint8_t idx)
{
    return idx < BOX_MAX_NUMBER;
}

void dashboard_big_time(void)
{
    if (!box_is_not_reserved()) return;

    uint8_t idx_time_big  = box_get_indx(BOX_GROUP_3, 0);
    uint8_t idx_time_head = box_get_indx(BOX_GROUP_8, 0);

    if (!box_index_ok(idx_time_big) || !box_index_ok(idx_time_head)) {
        Serial.printf("dashboard_big_time(): bad idx %d %d\n",
                      idx_time_big, idx_time_head);
        return;
    }

    // use safe clock, not DateTime/RTC
    const struct tm *t = &clock_mgr.my_time;

    uint8_t minute = t->tm_min;
    if (minute == prev_minute) return;
    prev_minute = minute;

    char buff[80];

    // full date/time
    tm_to_string(t, buff, sizeof(buff));
    box_paint(idx_time_head, BOX_SCHEME_TIME);
    box_print_text(idx_time_head, buff);

    // HH:MM
    snprintf(buff, sizeof(buff), " %02d:%02d", t->tm_hour, minute);
    box_paint(idx_time_big, BOX_SCHEME_TIME);
    box_print_text(idx_time_big, buff);
}

bool dashboard_show_sensor(void)
{
    if (!box_is_not_reserved()) return false;

    uint8_t idx_value = box_get_indx(BOX_GROUP_3, 1);
    uint8_t idx_label = box_get_indx(BOX_GROUP_8, 3);

    if (!box_index_ok(idx_value) || !box_index_ok(idx_label)) {
        Serial.printf("dashboard_show_sensor(): bad idx %d %d\n",
                      idx_value, idx_label);
        return false;
    }

    if (millis() < next_sensor_update) return false;
    next_sensor_update = millis() + 10000;

    char buff[40];

    snprintf(buff, sizeof(buff), "Sensor");
    box_paint(idx_label, BOX_SCHEME_SENSOR);
    box_print_text(idx_label, buff);

    snprintf(buff, sizeof(buff), " %.1f", 3.14);
    box_paint(idx_value, BOX_SCHEME_SENSOR);
    box_print_text(idx_value, buff);

    return true;
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



void dashboard_update_task(void)
{
    static uint32_t next_step_ms = 0;

    if (!box_is_not_reserved()) return;

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
            if (dashboard_show_sensor()) {
                next_step_ms = millis() + 10000;
                dbh.state = 30;
            } else {
                dbh.state = 10;
            }
            break;

        case 30:
            if (millis() > next_step_ms) {
                box_update_area();
                dbh.state = 10;
            }
            break;
    }
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

