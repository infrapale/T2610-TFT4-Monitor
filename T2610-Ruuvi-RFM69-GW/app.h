#ifndef __APP_H__
#define __APP_H__

#include <stdint.h>
#include "ruuvi.h"

#define APP_NAME_LEN                20
#define APP_MAX_TOTAL_NBR_OF_RUUVI  42
#define APP_BUFF_LEN                80

typedef enum 
{
    APP_UNDEFINED = 0,
    APP_VILLA_ASTRID ,
    APP_PORTIMO_POLKU,
    APP_ARBORETUM,
    APP_3,
    APP_4,
    APP_5,
    APP_6,
    APP_7,
    APP_NBR_OF
} app_indx_et;

typedef struct
{
    char name[APP_NAME_LEN];
    uint8_t nbr_ruuvi;
} app_st;

typedef struct 
{
    app_indx_et app_indx;
    char    name[RUUVI_NAME_LEN];
    uint8_t mac[RUUVI_MAC_LEN];
} app_ruuvit_st;

typedef struct
{
    app_indx_et active;
    size_t      nbr_total;
    size_t      nbr_in_app;
    uint8_t     ruuvi[APP_MAX_TOTAL_NBR_OF_RUUVI];
    char        buff[APP_BUFF_LEN];
} app_ctrl_st;

void app_initialize(app_indx_et app_indx);

void app_print_summary(void);

#endif