
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "msg.h"
#include  "app.h"
#include "rfm.h"

#include "atask.h"


extern rfm_st rfm;

app_conf_st app_conf = {
    .name = "Undefined",
    .indx = APP_UNDEFINED,
    .rfm_sw_version = 0,
    .wd_enabled = false,
    .lte_enabled = false
};

void app_initialize(void)
{

}

void app_print_conf(app_conf_st *conf)
{
    Serial.printf("%s indx=%d ver=%d WD=%d LTE=%d\n",
        conf->name, conf->indx, conf->rfm_sw_version, conf->wd_enabled, conf->lte_enabled);
}

void app_process_conf_msg(msg_st  *msgp)
{
    uint8_t err_cntr = 0;
    app_conf_st temp_conf;

    // msg_sub_print(msgp);
    if ((msgp->field[0][0] == 'C') && (msgp->field_count))
    {
        strncpy(temp_conf.name, rfm.rx.field[1], APP_NAME_LEN );
        for(uint8_t findx = 2; findx < msgp->field_count - 2; findx +=2)
        {
            switch(msgp->field[findx][0])
            {
                case 'I':
                    temp_conf.indx = (app_indx_et)msg_robust_atoi(
                        msgp->field[findx+1], 
                        &err_cntr, 
                        APP_UNDEFINED, APP_NBR_OF);
                  break;
                case 'V':
                    temp_conf.rfm_sw_version = (uint16_t)msg_robust_atoi(
                        msgp->field[findx+1], 
                        &err_cntr, 
                        0, 9999);
                    break;
                case 'W':
                    temp_conf.wd_enabled = (bool)msg_robust_atoi(
                        msgp->field[findx+1], 
                        &err_cntr, 
                        0, 1);
                    break;
                case 'L':
                    temp_conf.lte_enabled = (bool)msg_robust_atoi(
                        msgp->field[findx+1], 
                        &err_cntr, 
                        0, 1);
                    break;
            }
        }
        if(err_cntr == 0) {
            memcpy(&app_conf, &temp_conf, sizeof(app_conf));
            Serial.print("Configuration accepted: ");
            app_print_conf(&app_conf);
        } else {
            Serial.print("Incorrect Configuration: ");
            app_print_conf(&temp_conf);
        }


    }    
}

