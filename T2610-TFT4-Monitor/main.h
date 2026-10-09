#ifndef __MAIN_H__
#define __MAIN_H__
#include "WString.h"
#include <time.h>
#define   __APP__ ((char*)"T2610_TFT4_Monitor")

#define  MY_ADDR_LEN    8
#define LABEL_LEN           12
#define TXT_LEN             40
#define TIME_ZONE_OFFS      2
#define UNIT_LABEL_LEN      10
#define MEASURE_LABEL_LEN   16


// WiFi Access Point
#define PIRPANA
// #define LILLA_ASTRID
// #define VILLA_ASTRID

// HW Definitions
#define MCU_PICO_PLUS_2
#define DEBUG_PRINT 
#define SEND_TEST_MSG 
#include <Arduino.h>

typedef enum
{
    UNIT_TEMPERATURE = 0,
    UNIT_HUMIDITY,
    UNIT_AIR_PRESSURE,
    UNIT_LIGHT,
    UNIT_LDR,
    UNIT_VOLTAGE,
    UNIT_TIME,
    UNIT_CO2,
    UNIT_LUX,
    UNIT_NBR_OF
} unit_et;


typedef struct
{
    uint32_t next_io_tick;
    uint32_t next_super_tick;
    char my_addr[MY_ADDR_LEN];
    struct tm timeinfo;
} main_ctrl_st;


typedef struct
{
    char            tag;
    char            addr;         
} modem_data_st;





#endif