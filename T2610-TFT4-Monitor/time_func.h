#ifndef __TIME_FUNC_H__
#define __TIME_FUNC_H__

#include "Arduino.h"
#include "RTClib.h"
#include "main.h"
#include "comm.h"

#define TIME_ZONE_OFSET_SEC   (TIME_ZONE_OFFS * 3600)

typedef enum
{
    TIME_SRC_UNDEF = 0,
    TIME_SRC_CLOUD,
    TIME_SRC_RTC
} time_source_et;


bool time_begin(void);
void time_lost_power(bool force_new_time);
DateTime *time_get_time_now(void);
void time_to_string(char *buff);
void time_to_encode(char *buff);
uint32_t time_get_epoc_time(void);
void time_set_epoc_time(uint32_t epoc_time);
void time_print(void);
void time_set_comm_date_time(comm_data_st *msg);

#endif