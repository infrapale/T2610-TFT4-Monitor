#include <Arduino.h>
#include <btstack.h>
#include "main.h"
#include "app.h"
#include "ruuvi.h"
#include "io.h"
#include "atask.h"
// #include "r69.h"
// #include "msg.h"



app_st app[APP_NBR_OF] =
{
    [APP_UNDEFINED] = {.name = "Undefined", .nbr_ruuvi=0},
    [APP_VILLA_ASTRID] = {.name = "VillaAstrid", .nbr_ruuvi=0},
    [APP_PORTIMO_POLKU] = {.name = "Portimopolku", .nbr_ruuvi=0},
    [APP_ARBORETUM] = {.name = "Arboretum", .nbr_ruuvi=0},
    [APP_3] = {.name = "App3", .nbr_ruuvi=0},
    [APP_4] = {.name = "App4", .nbr_ruuvi=0},
    [APP_5] = {.name = "App5", .nbr_ruuvi=0},
    [APP_6] = {.name = "App6", .nbr_ruuvi=0},
    [APP_7] = {.name = "App7", .nbr_ruuvi=0}
};

app_ruuvit_st app_ruuvit[] =
{
    { 
        .app_indx = APP_VILLA_ASTRID,
        .name ="MH1-r", 
        .mac ={0xE6,0x2C,0x8D,0xDB,0x22,0x35}
    },
    { 
        .app_indx = APP_VILLA_ASTRID,
        .name ="KHH-r", 
        .mac ={0xED,0x9A,0xAB,0xC6,0x30,0x72}
    },
    { 
        .app_indx = APP_VILLA_ASTRID,
        .name ="Ulko-r", 
        .mac ={0xF2,0x5B,0x48,0x64,0x65,0x24}
    },
    { 
        .app_indx = APP_VILLA_ASTRID,
        .name ="Jaakaappi", 
        .mac ={0xFC,0xBB,0xC3,0x29,0x93,0x9B}
    },
    { 
        .app_indx = APP_VILLA_ASTRID,
        .name ="Pakastin", 
        .mac ={0xE4,0x35,0x8F,0x17,0x1F,0xEA}
    },
    { 
        .app_indx = APP_VILLA_ASTRID,
        .name ="Tupa-r", 
        .mac ={0xEA,0x78,0xE2,0x12,0x36,0xF8}
    },
    { 
        .app_indx = APP_VILLA_ASTRID,
        .name ="Parveke-r", 
        .mac ={0xF9,0x99,0xC1,0x14,0x14,0x54}
    },
    { 
        .app_indx = APP_PORTIMO_POLKU,
        .name ="Olohuone", 
        .mac ={0xEA,0x78,0xE2,0x12,0x12,0x34}
    },
    { 
        .app_indx = APP_ARBORETUM,  
        .name ="Keittio", 
        .mac ={0xF9,0x99,0xC1,0x14,0x98,0x76}
    },
    { 
        .app_indx = APP_UNDEFINED,  
        .name ="Undefined", 
        .mac ={0x00,0x00,0x00,0x00,0x00,0x00}
    }
};

app_ctrl_st app_ctrl;


void app_initialize(app_indx_et app_indx)
{
    app_ctrl.active = app_indx;
    app_ctrl.nbr_total = sizeof(app_ruuvit) / sizeof(app_ruuvit_st);
    uint8_t cursor = 0;

    for (size_t rindx = 0; rindx < app_ctrl.nbr_total; rindx++)
    {
        if(( app_ruuvit[rindx].app_indx >  APP_UNDEFINED ) &&  app_ruuvit[rindx].app_indx <  APP_NBR_OF )
        {
            if (app_ruuvit[rindx].app_indx  == app_indx){
                app_ctrl.ruuvi[cursor] = rindx;
                cursor++;
                app[app_indx].nbr_ruuvi++;
            } 
        }
    } 
    app_ctrl.nbr_in_app = app[app_indx].nbr_ruuvi;  
    app_print_summary();
}

void app_print_summary(void)
{
    Serial.printf("Active app: %s, nbr of ruuvi: %d\n", 
        app[app_ctrl.active].name,
        app[app_ctrl.active].nbr_ruuvi);

    for (uint8_t indx= 0; indx < app[app_ctrl.active].nbr_ruuvi; indx++)
    {
        uint8_t rindx = app_ctrl.ruuvi[indx];
        Serial.printf("%d - %d: %s\n",
            indx,
            rindx,
            app_ruuvit[rindx].name);
    }
}