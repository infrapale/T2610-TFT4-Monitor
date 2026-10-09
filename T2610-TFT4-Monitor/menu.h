#ifndef __MENU_H__
#define __MENU_H__

#include <stdint.h>
#include <Arduino.h>

typedef struct
{
    uint16_t x_pos;
    uint16_t y_pos;
    uint16_t width;
    uint16_t height;
    char     *txt;
} menu_box_st;

typedef struct
{
    uint8_t atask;
    uint8_t active;
} menu_ctrl_st;

typedef void (*cb_func)(void);
typedef enum  
{
    MENU_MAIN = 0,
    MENU_TIME,
    MENU_SENSOR,
    MENU_OUTDOOR,
    MENU_NBR_OF
} menu_row_et;

typedef struct
{
    char label[8];
    menu_row_et next;
    void (*cb_func)(void);
} menu_item_st;

typedef struct 
{
    char          row_label[20];
    menu_item_st menu_item[4];
}  menu_row_st;

void menu_initialize(void);
void menu_draw(void);
void menu_button_scan(void);
char menu_read_button(void);
void menu_btn_pressed(char pressed);

#endif