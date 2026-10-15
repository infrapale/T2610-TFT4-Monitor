
#include "main.h"
#include "io.h"
#include "menu.h"
#include "atask.h"
// #include "BtnPinOnOff.h"
#include "dashboard.h"
#include "box.h"
#include "a4key.h"
#include "sensor.h"

#define NBR_MENU_KEYS  4

a4key a4_key(PIN_ABTN);
extern box_group_st boxgr[];

void dummy_cb()
{
   // 
}

void menu_test1(void)
{
   Serial.printf("Test 1\n");
}

void dashboard_show_temperature(void) { dashboard_set_mode(DASHBOARD_TIME_TEMPERATURE); dashboard_set_update_now();}
void dashboard_show_humidity(void) { dashboard_set_mode(DASHBOARD_TIME_HUMIDITY); dashboard_set_update_now();}
void dashboard_show_battery(void) { dashboard_set_mode(DASHBOARD_TIME_BATTERY); dashboard_set_update_now();}

void dashboard_show_time_sensor(void);


menu_row_st menu[MENU_NBR_OF] =
{
  [MENU_MAIN] =
  {
    "Main Menu", 
    {
      {"Main",MENU_MAIN, dummy_cb },
      {"Time",MENU_TIME, menu_test1 },
      {"Sensor", MENU_SENSOR, dummy_cb},
      {"Outd", MENU_OUTDOOR, dummy_cb}
    }
  },
  [MENU_TIME] =
  {
    "Time", 
    {
      {"Main",MENU_MAIN, dummy_cb },
      {"Time",MENU_TIME, dummy_cb},
      {"Set", MENU_TIME, dummy_cb},
      {"Main", MENU_MAIN, dummy_cb}
    }
  },
  [MENU_SENSOR] =   // this menu is not visible
  {
    "Sensors", 
    {
      {"Main",MENU_MAIN, dummy_cb },
      {"Temp",MENU_SENSOR, dashboard_show_temperature},
      {"Hum", MENU_SENSOR, dashboard_show_humidity},
      {"Bat ",MENU_SENSOR, dashboard_show_battery}
     }
  },
  [MENU_OUTDOOR] =
  {
    "Outdoor Sensors", 
    {
      {"Temp",MENU_MAIN, dummy_cb },
      {"Trend",MENU_SENSOR, dummy_cb},
      {"Limit", MENU_SENSOR, dummy_cb},
      {"Main", MENU_MAIN, dummy_cb}
    }
  }
};


menu_ctrl_st menu_ctrl = {ATASK_NOT_DEFINED,  MENU_MAIN};

//BtnPinOnOff  menu_btn[NBR_MENU_KEYS];

void menu_state_machine(void);

//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st menu_th =                {"Menu State     ", 100,    0,     0,  255,    0,   1, menu_state_machine };
atask_st kscan_h =                {"Key Scan       ", 10,     0,     0,  255,    0,   1, menu_button_scan };


void menu_initialize(void)
{
  // menu_btn[0].Init(PIN_KEY1,'3', true);
  // menu_btn[1].Init(PIN_KEY2,'2', true);
  // menu_btn[2].Init(PIN_KEY3,'1', true);
  // menu_btn[3].Init(PIN_KEY_STATUS,'0', false);
  a4_key.begin();
  atask_add_new(&menu_th);
  atask_add_new(&kscan_h);
  menu_draw();

}

char menu_read(void)
{
  char c = 0x00;    //menu_read_button();
  if (c != 0x00) 
  {
      if ((c & 0b10000000) == 0) 
          Serial.printf("On");
      else 
          Serial.printf("Off");
      Serial.printf(" %c\n",c & 0b01111111);
      c &=  0b01111111;
      //menu_btn_pressed(c);
  }
  return c;
}

void menu_state_machine(void)
{
    static char     keyc = 0x00;
    static uint32_t menu_timeout;
    //static uint8_t  menu_hr  = box_get_indx(BOX_GROUP_HEAD_ROOM, 0, 0);
    char txt[20];

    switch(menu_th.state)
    {
        case 0:
            menu_th.state = 10;
            break;
        case 10:
            keyc = a4_key.read();
            if(keyc) Serial.printf("Key: %c\n",keyc);
            if(keyc != 0x00) menu_th.state = 20;
            // else dashboard 
            break;
        case 20:
            menu_btn_pressed(keyc);
            menu_th.state = 20;
            // box_reserve(BOX_RESERVE_MENU);
            //box_paint(menu_hr, 6);
            sprintf(txt, "key pressed: %c", keyc);
            // box_print_text(menu_hr, txt);
            // box_show_one(menu_hr);
            menu_timeout = millis() + 10000;
            menu_th.state = 30; 
        case 30:
            keyc = menu_read();
            if(keyc != 0x00) {
                menu_th.state = 20; 
            }
            else {
                if (millis() > menu_timeout){
                    // box_print_text(menu_hr, (char*)"menu timeout");
                    // box_show_one(menu_hr);
                    menu_th.state = 10; 
                    // box_release(BOX_RESERVE_MENU);
                }              
            }
            break;
        case 40:
            menu_th.state = 10;
            break;
    }
}



void menu_draw(void)
{
    for (uint8_t i = 0; i < MENU_BTN_NBR_OF; i++){
        uint8_t bindx = box_get_indx(BOX_GROUP_MENU, 0, i);
        box_print_text(bindx, menu[menu_ctrl.active].menu_item[i].label);
        box_paint(bindx+i, BOX_SCHEME_MENU);
    }
    box_show_group(BOX_GROUP_MENU);
}

void menu_button_scan(void)
{
    a4_key.scan();
}

void menu_btn_pressed(char pressed)
{
    uint8_t bindx = pressed - '0';
    if ((bindx >= 0) && (bindx <=3))
    {
        Serial.printf("active %d - pressed %d\n", menu_ctrl.active, bindx);
        menu[menu_ctrl.active].menu_item[bindx].cb_func();
        menu_ctrl.active = menu[menu_ctrl.active].menu_item[bindx].next;
        menu_draw();
    }
}
