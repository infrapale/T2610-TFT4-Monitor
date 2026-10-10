#ifndef __BOX_H__
#define __BOX_H__
#include "main.h"
#include "box.h"
// #include <TFT_eSPI.h> 

#define BOX_MAX_NUMBER      48
#define BOX_DEFAULT_GROUP   0

#define BOX_RESLOUTION_320X240  1
#define BOX_RESLOUTION_480X320  2


#if TFT_TARGET_BOARD == BOARD_PICO_TFT_4KEYS
    #define BOX_RESLOUTION  BOX_RESLOUTION_320X240
    #define TFT_LS_WIDTH        320
    #define TFT_LS_HIGHT        240
    #define TFT_LS_ROWS         12
    #define BOX_ROW_LEN       40
#elif TFT_TARGET_BOARD == BOARD_TFT_4_QUADCORE_PICO
    #define BOX_RESLOUTION  BOX_RESLOUTION_480X320
    #define TFT_LS_WIDTH        480
    #define TFT_LS_HIGHT        320
    #define TFT_LS_ROWS         16
    #define BOX_ROW_LEN         42
#else
	#pragma message("ERROR!!! No board selected")
	#include <User_Setup.h>           // Default setup is root library folder
#endif

typedef enum 
{
    BOX_GROUP_1 = 0,
    BOX_GROUP_16,
    BOX_GROUP_8,
    BOX_GROUP_4,
    BOX_GROUP_3,
    BOX_GROUP_2,
    BOX_GROUP_MENU,
    BOX_GROUP_HEAD_ROOM,
    BOX_GROUP_NBR_OF
} box_group_et;

typedef enum
{
    BOX_FONT_GLCD   = 1,
    BOX_FONT_SMALL_16  = 2,
    BOX_FONT_MEDIUM_26 = 4,
    BOX_FONT_LARGE_48  = 6,            
    BOX_FONT_7_SEGM_48  = 7,
    BOX_FONT_XXL_75     = 8,
} box_font_et;

typedef enum 
{
    BOX_RESERVE_MENU    = 0b00000001,
    BOX_RESERVE_NONE    = 0b00000000,
} box_reserve_et;

typedef enum
{
    BOX_SCHEME_BL_WH     = 0,
    BOX_SCHEME_BL_WH_BRD,
    BOX_SCHEME_BLUE_YELLOW,
    BOX_SCHEME_3,
    BOX_SCHEME_TIME,
    BOX_SCHEME_SENSOR,
    BOX_SCHEME_TIMEOUT,
    BOX_SCHEME_ALARM_HIGH,
    BOX_SCHEME_ALARM_LOW,
    BOX_SCHEME_9,
	BOX_SCHEME_10,
	BOX_SCHEME_NBR_OF,
} box_color_scheme_et;

typedef struct 
{
    uint16_t height;
    uint16_t width; 
    uint16_t round;
    uint16_t nbr;
    uint8_t  index;
    uint8_t  font_indx;
    uint8_t  font_size;
} box_group_st;


void box_initialize(void);

void box_reserve(uint8_t res_bm);

void box_release(uint8_t res_bm);

bool box_is_not_reserved(void);


uint8_t box_get_indx(uint8_t box_group, uint8_t bindx);

void box_set_visible(uint8_t box_group, uint8_t bindx, boolean visible );

void box_print_text(uint8_t bindx, char *txt);

uint8_t box_get_nbr_of_rows(uint8_t box_group);

void box_print_line(char *txt, uint8_t color_sch);

void box_show_one( uint8_t bindx);

void box_show_group(uint8_t box_group);

void box_update_area(void);

void box_show_all(void);

void box_hide_all(void);

void box_clone(uint8_t target, uint8_t source);

void box_clear(uint8_t bindx);

void box_clear_group(uint8_t box_group);

void box_paint(uint8_t bindx, uint8_t color_sch);

void box_scroll_down(uint8_t box_group);

void box_scroll_test(uint8_t group);

void box_structure_print(void);

void box_run_tft_pin_check(void);

#endif