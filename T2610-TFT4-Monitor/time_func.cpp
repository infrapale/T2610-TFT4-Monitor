// https://github.com/adafruit/RTClib/blob/master/src/RTClib.h

#include <Wire.h>
#include "time_func.h"
#include <TimeLib.h>
#include "RTC_NXP.h"
#include "RTClib.h"
#include "comm.h"


typedef struct 
{
    tm        new_time;
    DateTime  now;
    uint32_t cloud_time;
} rtc_ctrl_st;


// https://github.com/XbergCode/DateTimeFunctions

// Convert seconds to date time, from year X.  
// char* conSec2DT(const uint32_t _SEC, const uint16_t _YEAR, const bool _USEMONTHNAME = false);
// Returns: Date string


//extern RTC_PCF8563 rtc;
//RTC_PCF8563 rtc;
RTC_DS3231  rtc;
rtc_ctrl_st rtc_ctrl;


char week_day[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

DateTime time_now;
DateTime cloud_time;


bool time_begin(void)
{
  bool ret = false;
  if (!rtc.begin(&Wire1)) 
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    //while (1) delay(10);
  } 
  else 
  {
    ret = true;
    time_lost_power(true);
    time_print();
  }
  return ret;
}

void time_lost_power(bool force_new_time)
{
  if (rtc.lostPower() || force_new_time) 
  {
      Serial.println("RTC is NOT initialized, let's set the time!");
      // When time needs to be set on a new device, or after a power loss, the
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      //rtc.adjust(DateTime(2023, 12, 13, 3, 0, 0));
      //
      // Note: allow 2 seconds after inserting battery or applying external power
      // without battery before calling adjust(). This gives the PCF8523's
      // crystal oscillator time to stabilize. If you call adjust() very quickly
      // after the RTC is powered, lostPower() may still return true.


    // When the RTC was stopped and stays connected to the battery, it has
    // to be restarted by clearing the STOP bit. Let's do this to ensure
    // the RTC is running.
  }

}

void time_set_comm_date_time(comm_data_st *msg)
{
    rtc.adjust(DateTime(
            atoi(msg->field[1]),
            atoi(msg->field[2]),
            atoi(msg->field[3]),
            atoi(msg->field[4]),
            atoi(msg->field[5]),
            0));
}

DateTime *time_get_time_now(void)
{
    rtc_ctrl.now = rtc.now();

    return &rtc_ctrl.now;
}


void time_to_string(char *buff )
{
    DateTime now = rtc.now();
    sprintf(buff, "%s %d-%d-%d",
        week_day[now.dayOfTheWeek()],
        now.year(),
        now.month(),
        now.day()
    );   
}

void time_to_encode(char *buff )
{
    DateTime now = rtc.now();
    sprintf(buff, "<##C0T0=%d;%d;%d;%d;%d;%d>",
        now.year(),
        now.month(),
        now.day(),
        now.dayOfTheWeek(),
        now.hour(),
        now.minute()
    );   
}


uint32_t time_get_epoc_time(void)
{
    DateTime now = rtc.now();
    return now.unixtime();
}

void time_set_epoc_time(uint32_t epoc_time)
{
    DateTime now =epoc_time;
    Serial.printf("Time Set: %4d-%02d-%02d %02d:%02d:%02d\n", 
        year(epoc_time), month(epoc_time), day(epoc_time), 
        hour(epoc_time), minute(epoc_time), second(epoc_time));

    //rtc_ctrl.now.year()
    //const uint32_t epoc = epoc_time;
    // rtc_ctrl.new_time.tm_year = year(epoc);
    // rtc_ctrl.new_time.tm_mon = month(epoc);  
    // rtc_ctrl.new_time.tm_mday = day(epoc);
    // rtc_ctrl.new_time.tm_hour = hour(epoc);
    // rtc_ctrl.new_time.tm_min = minute(epoc);
    // rtc_ctrl.new_time.tm_sec = second(epoc);

    rtc.adjust(now);
    Serial.printf("time_set_epoc_time(%d)\n",epoc_time);
    Serial.println("time_print():");
    time_print();
}
void time_print(void)
{
    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(week_day[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
}