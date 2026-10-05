#ifndef __MAIN_H__
#define __MAIN_H__
#include <Arduino.h>
#include "WString.h"
#include <time.h>
#define   __APP__ ((char*)"T2610-Ruuvi-RFM69-GW")

#define  MY_ADDR_LEN    8

// WiFi Access Point
#define PIRPANA
// #define LILLA_ASTRID
// #define VILLA_ASTRID

// HW Definitions
#define MCU_PICO_PLUS_2

#define DEBUG_PRINT 
#define SEND_TEST_MSG 
#define T2610_RUUVI_RFM69_GW

#define UART_MSG_LEN    80
#define MY_MODULE_TAG   'R'
#define MY_MODULE_ADDR  '1'

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