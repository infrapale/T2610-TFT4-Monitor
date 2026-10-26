#ifndef __APP_H__
#define __APP_H__


typedef enum 
{
    APP_VILLA_ASTRID = 0,
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
    char label[APP_LABEL_LEN];
} app_st;

#endif