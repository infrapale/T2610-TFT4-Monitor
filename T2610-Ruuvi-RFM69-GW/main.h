#ifndef __MAIN_H__
#define __MAIN_H__
#include <Arduino.h>
#include "WString.h"
#include <time.h>

#define   __APP__ ((char*)"T2610-Ruuvi-RFM69-GW")
#define   __VER__ (100)
#define DEBUG_PRINT 
#define T2610_RUUVI_RFM69_GW


// WiFi Access Point
#define PIRPANA
// #define LILLA_ASTRID
// #define VILLA_ASTRID


typedef struct
{
    uint8_t     app_indx;
    uint32_t    next_io_tick;
    uint32_t    next_super_tick;
    struct      tm timeinfo;
} main_ctrl_st;

#endif