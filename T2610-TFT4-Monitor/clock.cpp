

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




// Days per month for non-leap and leap years
static const uint8_t days_in_month[2][12] = {
    {31,28,31,30,31,30,31,31,30,31,30,31},
    {31,29,31,30,31,30,31,31,30,31,30,31}
};

void clock_initialize(void)
{
    atask_add_new(&clock_th);
    clock_mgr.time_epoch = 0;  
    memset(&clock_mgr.my_time, 0, sizeof(clock_mgr.my_time));

    clock_mgr.next_minute = millis() + 60000;
    clock_mgr.last_hour = 0;
    clock_mgr.new_hour_event = false;
}

static bool is_leap(uint32_t year)
{
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

// Convert epoch seconds → struct tm (UTC)
void epoch_to_tm(uint32_t epoch, struct tm *out)
{
    uint32_t secs = epoch;

    out->tm_sec  = secs % 60;
    secs /= 60;
    out->tm_min  = secs % 60;
    secs /= 60;
    out->tm_hour = secs % 24;

    uint32_t days = secs / 24;

    // Epoch starts at 1970-01-01
    uint32_t year = 1970;
    while (true) {
        uint32_t days_in_year = is_leap(year) ? 366 : 365;
        if (days < days_in_year) break;
        days -= days_in_year;
        year++;
    }
    out->tm_year = year - 1900;

    uint8_t leap = is_leap(year) ? 1 : 0;
    uint8_t month = 0;
    while (days >= days_in_month[leap][month]) {
        days -= days_in_month[leap][month];
        month++;
    }

    out->tm_mon  = month;
    out->tm_mday = days + 1;
    out->tm_isdst = 0;
}

uint32_t tm_to_epoch(const struct tm *t)
{
    uint32_t year = t->tm_year + 1900;
    uint32_t month = t->tm_mon;
    uint32_t day = t->tm_mday - 1;

    uint32_t days = 0;

    // Add days for all previous years
    for (uint32_t y = 1970; y < year; y++) {
        days += is_leap(y) ? 366 : 365;
    }

    // Add days for previous months in this year
    uint8_t leap = is_leap(year) ? 1 : 0;
    for (uint32_t m = 0; m < month; m++) {
        days += days_in_month[leap][m];
    }

    // Add days in this month
    days += day;

    uint32_t epoch = days * 86400;
    epoch += t->tm_hour * 3600;
    epoch += t->tm_min  * 60;
    epoch += t->tm_sec;

    return epoch;
}

void tm_to_string(const struct tm *t, char *buff, size_t len)
{
    snprintf(buff, len,
             "%04d-%02d-%02d %02d:%02d:%02d",
             t->tm_year + 1900,
             t->tm_mon + 1,
             t->tm_mday,
             t->tm_hour,
             t->tm_min,
             t->tm_sec);
}

void clock_set_date_time(void)
{
    struct tm tmp = {0};
    uint8_t errors = 0;

    if ((rfm.rx.field_count == 8) && (rfm.rx.field[1][0] == '#'))
    {
        tmp.tm_year = msg_robust_atoi(rfm.rx.field[3], &errors, 2000, 2100) - 1900;
        tmp.tm_mon  = msg_robust_atoi(rfm.rx.field[4], &errors, 1, 12) - 1;
        tmp.tm_mday = msg_robust_atoi(rfm.rx.field[5], &errors, 1, 31);
        tmp.tm_hour = msg_robust_atoi(rfm.rx.field[6], &errors, 0, 23);
        tmp.tm_min  = msg_robust_atoi(rfm.rx.field[7], &errors, 0, 59);
        tmp.tm_sec  = 0;

        if (errors == 0) {
            clock_mgr.time_epoch = tm_to_epoch(&tmp);
            clock_mgr.my_time    = tmp;
        }
    }

    clock_mgr.next_minute = millis() + 60000;
}

void clock_task(void)
{
    if (millis() > clock_mgr.next_minute)
    {
        clock_mgr.next_minute += 60000;
        clock_mgr.time_epoch += 60;

        epoch_to_tm(clock_mgr.time_epoch, &clock_mgr.my_time);
    }

    if (clock_mgr.last_hour != clock_mgr.my_time.tm_hour)
    {
        clock_mgr.last_hour = clock_mgr.my_time.tm_hour;

        char buff[40];
        tm_to_string(&clock_mgr.my_time, buff, sizeof(buff));
        Serial.println(buff);

        if (clock_mgr.my_time.tm_hour == 8)
        {
            msg_send_all_temp();
            sensor_clear_all();
        }
    }
}

