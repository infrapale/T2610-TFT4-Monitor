#include <stdint.h>
#ifndef __APP_H__
#define __APP_H__

#define APP_NAME_LEN                20

// App index must be same as on the RFM MCU
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
    char        name[APP_NAME_LEN];
    app_indx_et indx;
    uint16_t    rfm_sw_version;
    bool        wd_enabled;
    bool        lte_enabled;
} app_conf_st;

void app_process_conf_msg(msg_st  *msgp);

#endif