
#include "main.h"
#include "io.h"
#include "menu.h"
#include "atask.h"
#include "BtnPinOnOff.h"
#include "dashboard.h"
#include "box.h"
#include "a4key.h"

#define NBR_MENU_KEYS  4

a4key a4_key(PIN_ABTN);

void dummy_cb()
{
   // 
}

void menu_test1(void)
{
   Serial.printf("Test 1\n");
}

void dashboard_show_sensor_print(void);

void dashboard_show_time_sensor(void);


menu_row_st menu[MENU_NBR_OF] =
{
  [MENU_MAIN] =
  {
    "Main Menu", 
    {
      {"Main",MENU_MAIN, dummy_cb },
      {"Time",MENU_TIME, menu_test1 },
      {"Printer", MENU_SENSOR, dashboard_show_sensor_print},
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
      {"Main",MENU_MAIN, dashboard_show_time_sensor },
      {"Prev.",MENU_SENSOR, dashboard_previous_sensor},
      {"Next", MENU_SENSOR, dashboard_next_sensor},
      {"Main", MENU_MAIN, dashboard_show_time_sensor}
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

BtnPinOnOff  menu_btn[NBR_MENU_KEYS];

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
  // menu_draw();

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
    static uint8_t  menu_hr  = box_get_indx(BOX_GROUP_HEAD_ROOM, 0);
    char txt[20];

    switch(menu_th.state)
    {
        case 0:
            menu_th.state = 10;
            break;
        case 10:
            keyc = a4_key.read();
            if(keyc) Serial.printf("Key: %c\n",keyc);
            //if(keyc != 0x00) menu_th.state = 20;
            // else dashboard 
            break;
        case 20:
            menu_btn_pressed(keyc);
            menu_th.state = 20;
            box_reserve(BOX_RESERVE_MENU);
            box_paint(menu_hr, 6);
            sprintf(txt, "key pressed: %c", keyc);
            box_print_text(menu_hr, txt);
            box_show_one(menu_hr);
            menu_timeout = millis() + 10000;
            menu_th.state = 30; 
        case 30:
            keyc = menu_read();
            if(keyc != 0x00) {
                menu_th.state = 20; 
            }
            else {
                if (millis() > menu_timeout){
                    box_print_text(menu_hr, (char*)"menu timeout");
                    box_show_one(menu_hr);
                    menu_th.state = 10; 
                    box_release(BOX_RESERVE_MENU);
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
    uint8_t bindx = box_get_indx(BOX_GROUP_16, 8);
    box_paint(bindx, 0);
    box_print_text(bindx, menu[menu_ctrl.active].row_label);
    box_show_one(bindx);
    bindx++;
    box_clear(bindx);
    box_paint(bindx, 0);

    bindx = box_get_indx(BOX_GROUP_MENU, 0);
    for (uint8_t i = 0; i < 3; i++)
    {
        box_paint(bindx+i, 9);
        box_print_text(bindx+i, menu[menu_ctrl.active].menu_item[i+1].label);
    }
    box_show_group(BOX_GROUP_MENU);

}

void menu_button_scan(void)
{
    a4_key.scan();
}

char menu_read_button(void)
{
    char c = 0x00;
    for( uint8_t i= 0; i < NBR_MENU_KEYS; i++)
    {
        c = menu_btn[i].Read();
        if (c != 0x00) break;
    }
    return c;
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
