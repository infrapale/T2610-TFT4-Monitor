/************************************************************************
    BOX_GROUP_1   BOX_GROUP_12   BOX_DIM4    BOX_GROUP_2   BOX_GROUP_3
    ---------   ---------   --------    ---------   ---------
    |       |   |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |   |       |  
    |       |   |       |   |       |   |       |   |       |  
    |       |   ---------   ---------   |       |   |       |  
    |       |   |       |   |       |   |       |   ---------  
    |       |   ---------   |       |   |       |   |       |  
    |       |   |       |   |       |   |       |   |       |  
    |       |   ---------   ---------   ---------   |       | 
    |       |   |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |   |       |
    |       |   |       |   |       |   |       |   ---------  
    |       |   ---------   ---------   |       |   |       |  
    |       |   |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |   |       |  
    |       |   |       |   |       |   |       |   |       |  
    ---------   ---------   --------    ---------   ---------


***************************************************************************/


#include "main.h"
#include "box.h"
#include "io.h"
#include "atask.h"
#include <SPI.h>
#include <TFT_eSPI.h> 



typedef struct
{
    uint8_t  group;
    bool     visible;
    uint16_t x_pos;
    uint16_t y_pos;
    uint16_t width;
    uint16_t height;
    char     txt[BOX_ROW_LEN];
    uint8_t  font_indx;
    uint8_t  font_size;
    uint16_t fill_color;
    uint16_t border_color;
    uint16_t text_color;
} box_st;


typedef struct 
{
    uint8_t top;
    uint8_t last;
} box_print_area_st;


typedef struct 
{
    uint8_t nbr;
    uint8_t reserve;
    box_print_area_st print_area;
} box_ctrl_st;

SPISettings mySPISettings(2000000, MSBFIRST, SPI_MODE0); // 4 MHz clock
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
//extern TFT_eSPI tft;


// Default color definitions
// #define TFT_BLACK       0x0000      /*   0,   0,   0 */
// #define TFT_NAVY        0x000F      /*   0,   0, 128 */
// #define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
// #define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
// #define TFT_MAROON      0x7800      /* 128,   0,   0 */
// #define TFT_PURPLE      0x780F      /* 128,   0, 128 */
// #define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
// #define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
// #define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
// #define TFT_BLUE        0x001F      /*   0,   0, 255 */
// #define TFT_GREEN       0x07E0      /*   0, 255,   0 */
// #define TFT_CYAN        0x07FF      /*   0, 255, 255 */
// #define TFT_RED         0xF800      /* 255,   0,   0 */
// #define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
// #define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
// #define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
// #define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
// #define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
// #define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F
// #define TFT_BROWN       0x9A60      /* 150,  75,   0 */
// #define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
// #define TFT_SILVER      0xC618      /* 192, 192, 192 */
// #define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
// #define TFT_VIOLET      0x915C      /* 180,  46, 226 */

box_color_st box_color_scheme[BOX_SCHEME_NBR_OF] PROGMEM =
{
    [BOX_SCHEME_UNDEF]          = {.fill_color = TFT_BLACK,         .border_color = TFT_BLACK,          .text_color = TFT_WHITE },
    [BOX_SCHEME_BL_WH]          = {.fill_color = TFT_BLACK,         .border_color = TFT_BLACK,          .text_color = TFT_WHITE },
    [BOX_SCHEME_BL_WH_BRD]      = {.fill_color = TFT_BLUE,          .border_color = TFT_YELLOW,         .text_color = TFT_YELLOW },
    [BOX_SCHEME_BLUE_YELLOW]    = {.fill_color = TFT_BLUE,          .border_color = TFT_YELLOW,         .text_color = TFT_YELLOW },
    [BOX_SCHEME_MENU]           = {.fill_color = TFT_DARKGREY,      .border_color = TFT_DARKCYAN,       .text_color = TFT_WHITE },
    [BOX_SCHEME_TIME]           = {.fill_color = TFT_BLACK,         .border_color = TFT_BLACK,          .text_color = TFT_ORANGE },
    [BOX_SCHEME_SENSOR]         = {.fill_color = TFT_DARKCYAN,      .border_color = TFT_LIGHTGREY,      .text_color = TFT_WHITE },
    [BOX_SCHEME_TIMEOUT]        = {.fill_color = TFT_SILVER,        .border_color = TFT_GOLD,           .text_color = TFT_DARKGREY },
    [BOX_SCHEME_ALARM_HIGH]     = {.fill_color = TFT_RED,           .border_color = TFT_RED,            .text_color = TFT_WHITE },
    [BOX_SCHEME_ALARM_LOW]      = {.fill_color = TFT_WHITE,         .border_color = TFT_NAVY,           .text_color = TFT_RED },
    [BOX_SCHEME_9]              = {.fill_color = TFT_MAROON,        .border_color = TFT_SILVER,         .text_color = TFT_WHITE },
    [BOX_SCHEME_10]             = {.fill_color = TFT_DARKCYAN,      .border_color = TFT_WHITE,          .text_color = TFT_WHITE },
    // [9] = {.fill_color = TFT_CYAN,          .border_color = TFT_MAGENTA,        .text_color = TFT_MAGENTA },
};

box_group_st boxgr[BOX_GROUP_NBR_OF] =
{
    [BOX_GROUP_1]           = {
        .y0=0, 
        .x0=0, 
        .height=TFT_LS_HIGHT,                    
        .width=TFT_LS_WIDTH,    
        .round = 0, 
        .nbr_r=1,             
        .nbr_c=1,  
        .index = 0, 
        .font_indx = BOX_FONT_XXL_75,       
        .font_size = 2,
        .txt_offs_y = 0,
        .txt_offs_x = 0,
        .color_scheme = BOX_SCHEME_UNDEF},
    [BOX_GROUP_16]          = {
        .y0=0, 
        .x0=0, 
        .height=TFT_LS_HIGHT / TFT_LS_ROWS,      
        .width=TFT_LS_WIDTH,    
        .round = 0, 
        .nbr_r=TFT_LS_ROWS,   
        .nbr_c=1,  
        .index = 0, 
        .font_indx = BOX_FONT_SMALL_16,     
        .font_size = 1,
        .txt_offs_y = 1,
        .txt_offs_x = 1,
        .color_scheme = BOX_SCHEME_BLUE_YELLOW },
    [BOX_GROUP_8]           = {
        .y0=0, 
        .x0=0, 
        .height=TFT_LS_HIGHT / TFT_LS_ROWS * 2,  
        .width=TFT_LS_WIDTH,    
        .round = 0, .nbr_r=TFT_LS_ROWS/2, 
        .nbr_c=1,  
        .index = 0, 
        .font_indx = BOX_FONT_MEDIUM_26,    
        .font_size = 1,
        .txt_offs_y = 0,
        .txt_offs_x = 0,
        .color_scheme = BOX_SCHEME_UNDEF  },
    [BOX_GROUP_MID_5X2]      = {
        .y0=TFT_SMALL_ROW_HIGHT * 4,
        .x0=0, 
        .height=TFT_SMALL_ROW_HIGHT * 2,  
        .width=TFT_LS_WIDTH / 2,    
        .round = 2, 
        .nbr_r= BOX_NUMERIC_ROWS, 
        .nbr_c= BOX_NUMERIC_COLS,  
        .index = 0, 
        .font_indx = BOX_FONT_MEDIUM_26,    
        .font_size = 1,
        .txt_offs_y = 12,
        .txt_offs_x = 4,
        .color_scheme = BOX_SCHEME_SENSOR  },
    [BOX_GROUP_4]           = {
        .y0=0, 
        .x0=0, 
        .height=TFT_LS_HIGHT / TFT_LS_ROWS * 4,  
        .width=TFT_LS_WIDTH,    
        .round = 0, 
        .nbr_r=TFT_LS_ROWS/4, 
        .nbr_c=1,  .index = 0, 
        .font_indx = BOX_FONT_XXL_75,       
        .font_size = 1,
        .txt_offs_y = 0,
        .txt_offs_x = 0,
        .color_scheme = BOX_SCHEME_UNDEF  },
    [BOX_GROUP_3]           = {
        .y0=0, 
        .x0=0, 
        .height=TFT_LS_HIGHT / TFT_LS_ROWS * 4,  
        .width=TFT_LS_WIDTH,    
        .round = 0, .nbr_r=3,             
        .nbr_c=1,  .index = 0, 
        .font_indx = BOX_FONT_XXL_75,       
        .font_size = 1,
        .txt_offs_y = 0,
        .txt_offs_x = 0,
        .color_scheme = BOX_SCHEME_UNDEF  },
    [BOX_GROUP_2]           = {
        .y0=0, 
        .x0=0, 
        .height=TFT_LS_HIGHT / TFT_LS_ROWS * 8,  
        .width=TFT_LS_WIDTH,    
        .round = 0, 
        .nbr_r=TFT_LS_ROWS/8, 
        .nbr_c=1,  
        .index = 0, 
        .font_indx = BOX_FONT_7_SEGM_48,    
        .font_size = 2,
        .txt_offs_y = 0,
        .txt_offs_x = 0,
        .color_scheme = BOX_SCHEME_UNDEF  },
    [BOX_GROUP_MENU]        = {
        .y0=TFT_SMALL_ROW_HIGHT *(TFT_LS_ROWS-2) + 4, 
        .x0=0, 
        .height=TFT_SMALL_ROW_HIGHT * 2 - 4,     
        .width=TFT_LS_WIDTH/4,  
        .round = 5, 
        .nbr_r=1,             
        .nbr_c=4,  
        .index = 0, 
        .font_indx = BOX_FONT_MEDIUM_26,    
        .font_size = 1,
        .txt_offs_y = 4,
        .txt_offs_x = 4,
        .color_scheme = BOX_SCHEME_MENU  },
    [BOX_GROUP_HEAD_ROOM]   = {
        .y0=0, 
        .x0=0, 
        .height=TFT_LS_HIGHT / TFT_LS_ROWS * (TFT_LS_ROWS-1), 
        .width=TFT_LS_WIDTH,    
        .round = 0, 
        .nbr_r=1,  
        .nbr_c=1,  
        .index = 0, 
        .font_indx = BOX_FONT_MEDIUM_26,   
        .font_size = 1,
        .txt_offs_y = 0,
        .txt_offs_x = 0,
        .color_scheme = BOX_SCHEME_UNDEF },
}; 


box_ctrl_st box_ctrl = {0};


box_st  box[BOX_MAX_NUMBER];


void box_clear(void)
{
    tft.setRotation(1);
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    //dashboard_draw_box(0);  // clear dashboard
}

void box_initialize(void)
{
    
    SPI.beginTransaction(mySPISettings);
    tft.init();
    Serial.printf("TFT: %d X %d",
        tft.width(),
        tft.height()
    );
    box_clear();
    tft.fillScreen(TFT_MAROON);


    uint8_t bindx = 0;
    uint8_t box_group = (uint8_t) BOX_GROUP_1;


    for (uint8_t box_group = (uint8_t) BOX_GROUP_1; box_group < BOX_GROUP_NBR_OF; box_group++)
    {
        uint16_t    xpos =  boxgr[box_group].x0;
        uint16_t    ypos =  boxgr[box_group].y0;
        uint8_t     nbr_rows = boxgr[box_group].nbr_r;
        uint8_t     nbr_cols = boxgr[box_group].nbr_c;
        
        boxgr[box_group].index = bindx;
        for(uint8_t row = 0; row < nbr_rows; row++)
        {
            for(uint8_t col = 0; col < nbr_cols; col++)
            {
                box[bindx].x_pos        = xpos + col * boxgr[box_group].width;
                box[bindx].y_pos        = ypos + row * boxgr[box_group].height;
                box[bindx].width        = boxgr[box_group].width;
                box[bindx].height       = boxgr[box_group].height;
                box[bindx].fill_color   = TFT_BLUE;
                box[bindx].border_color = TFT_YELLOW;
                box[bindx].text_color   = TFT_WHITE;
                box[bindx].font_indx    = boxgr[box_group].font_indx;
                box[bindx].font_size    = boxgr[box_group].font_size;       
                box[bindx].group        = box_group;
                sprintf(box[bindx].txt, "%d-%d-%d",
                    box_group,
                    bindx,
                    boxgr[box_group].nbr_r
                );
                box[bindx].visible = false;
                bindx++;
                if(bindx >= BOX_MAX_NUMBER) {
                    Serial.println("!!!! ERROR MAX BOX NUMBER EXCEEDED !!!!");
                    box_group =99;
                    break;
                }
             }    
        }
    } 
    box_ctrl.nbr = bindx - 1;
    box_ctrl.print_area.top     = 12;  // scrolling text bindings
    box_ctrl.print_area.last    = 15;

    box_structure_print();
    Serial.println("box_initialize completed");

}

void box_reserve(uint8_t res_bm)
{
    box_ctrl.reserve  |= res_bm;
}
void box_release(uint8_t res_bm)
{
    box_ctrl.reserve  &= ~res_bm;
}

uint8_t box_get_indx(uint8_t box_group, uint8_t row_indx, uint8_t col_indx)
{
    uint8_t box_indx = 255;
    if(( row_indx < boxgr[box_group].nbr_r) && ( col_indx < boxgr[box_group].nbr_c))
        box_indx = boxgr[box_group].index + col_indx + (boxgr[box_group].nbr_c * row_indx);
    else 
        box_indx = boxgr[box_group].index;

    if (box_indx >= BOX_MAX_NUMBER) box_indx = 0;
    return box_indx;
}

uint8_t box_get_nbr_of_rows(uint8_t box_group)
{
    return boxgr[box_group].nbr_r;
}

uint8_t box_get_nbr_of_cols(uint8_t box_group)
{
    return boxgr[box_group].nbr_c;
}
void box_set_visible(uint8_t box_group, uint8_t bindx, boolean visible )
{
    uint8_t indx = box_get_indx(box_group, bindx);
    box[bindx].visible = visible;
}


void box_show_one( uint8_t bindx, box_color_scheme_et color_sch)
{
    uint8_t group = box[bindx].group;

    box_paint(bindx, color_sch);
    //Serial.printf("Font %d Color %d %s \n",box[bindx].font_indx,box[bindx].text_color, box[bindx].txt);
    tft.setTextSize(box[bindx].font_size);
    tft.setTextFont(box[bindx].font_indx);
    tft.setTextColor(box[bindx].text_color, box[bindx].fill_color, false);
    if(boxgr[group].round == 0) {
        tft.fillRect(box[bindx].x_pos, box[bindx].y_pos, box[bindx].width, box[bindx].height, box[bindx].fill_color);
        tft.drawRect(box[bindx].x_pos, box[bindx].y_pos, box[bindx].width, box[bindx].height, box[bindx].border_color);
    }
    else {
        tft.fillRoundRect(box[bindx].x_pos, box[bindx].y_pos, box[bindx].width, box[bindx].height, boxgr[group].round, box[bindx].fill_color);
        tft.drawRoundRect(box[bindx].x_pos, box[bindx].y_pos, box[bindx].width, box[bindx].height, boxgr[group].round, box[bindx].border_color);
    }
    tft.drawString( box[bindx].txt , box[bindx].x_pos+boxgr[group].txt_offs_x, box[bindx].y_pos+boxgr[group].txt_offs_y, box[bindx].font_indx);
}

void box_show_group(uint8_t box_group, box_color_scheme_et color_sch)
{
    for (uint8_t box_indx = 0; box_indx < BOX_MAX_NUMBER; box_indx++){
        if (box[box_indx].group == box_group){
            box_show_one( box_indx, color_sch);
        }
    }
}

void box_show_all(void)
{
    for( uint8_t bindx = 0; bindx < box_ctrl.nbr; bindx++ )
    {
        if(  box[bindx].visible)
        {
            box_show_one( bindx);
            delay(100);
        }
    }
}

void box_print_text(uint8_t bindx, char *txt)
{
    box[bindx].visible = true;
    //Serial.printf("box_print_text: bindx: %d Font %d Color %d %s \n", bindx ,box[bindx].font_indx,box[bindx].text_color, box[bindx].txt);
    Serial.printf("box_print_text: bindx: %d ypos %d\n", bindx ,box[bindx].y_pos);
    strncpy(box[bindx].txt, txt, BOX_ROW_LEN);
    box_show_one(bindx);
}


void box_print_line(char *txt, uint8_t color_sch)
{
    uint8_t bindx = box_get_indx(BOX_GROUP_16, box_ctrl.print_area.top);
    // box_scroll_down(BOX_GROUP_16);
    // box_paint(bindx,color_sch);
    //box_print_text(bindx, txt);
}


void box_hide_all(void)
{
    for( uint8_t bindx = 0; bindx < box_ctrl.nbr; bindx++ )
    {
        box[bindx].visible = false;
    }
}

void box_clone(uint8_t target, uint8_t source)
{
    box[target].visible      = box[source].visible;
    box[target].font_indx    = box[source].font_indx;
    box[target].font_size    = box[source].font_size;
    box[target].fill_color   = box[source].fill_color;
    box[target].border_color = box[source].border_color;
    box[target].text_color   = box[source].text_color;
    strncpy(box[target].txt,   box[source].txt, BOX_ROW_LEN);
}
void box_clear(uint8_t bindx)
{
    uint8_t box_group =  box[bindx].group;
    memset(box[bindx].txt, 0x00, BOX_ROW_LEN);
    box[bindx].font_indx    = boxgr[box_group].font_indx;
    box[bindx].font_size    = boxgr[box_group].font_size;
    box[bindx].fill_color   = box[BOX_DEFAULT_GROUP].fill_color;
    box[bindx].border_color = box[BOX_DEFAULT_GROUP].border_color;
    box[bindx].text_color   = box[BOX_DEFAULT_GROUP].text_color;
}

void box_clear_group(uint8_t box_group)
{
    uint8_t bindx = boxgr[box_group].index;
    uint8_t rows = boxgr[box_group].nbr_r;

    for (uint8_t i = 0; i< rows; i++)
    {
        box_clear(bindx+i);
    }
}
void box_paint(uint8_t bindx,   box_color_scheme_et color_sch)
{
    uint8_t group = box[bindx].group;
    box_color_scheme_et actual_color_sch;
    if (color_sch == BOX_SCHEME_UNDEF) actual_color_sch = boxgr[group].color_scheme;
    else actual_color_sch = color_sch;

    // Serial.printf("box_paint %d %d %d\n", bindx, group, actual_color_sch);
    box[bindx].fill_color   = box_color_scheme[actual_color_sch].fill_color;
    box[bindx].border_color = box_color_scheme[actual_color_sch].border_color;
    box[bindx].text_color   = box_color_scheme[actual_color_sch].text_color;
}


void box_scroll_down(uint8_t box_group)
{
    uint8_t rows = box_ctrl.print_area.last - box_ctrl.print_area.top + 1;
    uint8_t bindx = boxgr[box_group].index + box_ctrl.print_area.last;
    Serial.printf("box_scroll_down group %d: ",box_group);
    for (uint8_t i = 0; i < rows -1; i++)
    {   Serial.printf("%d<-%d, ",bindx,bindx-1);
        box_clone(bindx, bindx - 1);
        box_show_one(bindx);
        bindx--;
    }
    Serial.println();
    bindx--;
    box_clear(bindx);
}

void box_scroll_test(uint8_t group)
{
    box_color_scheme_et color_sch = BOX_SCHEME_UNDEF;
    uint8_t top_row = boxgr[group].index;

    box_clear_group(group);
    box_show_group(group);
    for (uint8_t n = 0; n < 20; n++)
    {
        box_scroll_down(group);
        sprintf(box[top_row].txt, "Group %d - Color %d", group, color_sch);
        box_paint(top_row,color_sch);

        box_show_group(group);
    }
}


void box_draw_line(uint8_t bindx, uint32_t x0, uint32_t y0,uint32_t x1,uint32_t y1)
{
    tft.drawLine(
        box[bindx].x_pos + x0,
        box[bindx].y_pos + y0,
        box[bindx].x_pos + x1,
        box[bindx].y_pos + y1,
        TFT_YELLOW);
}

void box_draw_dot(uint8_t bindx, uint32_t x0, uint32_t y0,uint32_t r,uint32_t color)
{
    tft.fillCircle(
            box[bindx].x_pos + x0,
            box[bindx].y_pos + y0,
            r,
            color);
  
}

//    if(boxgr[group].round == 0) {
//         tft.fillRect(box[bindx].x_pos, box[bindx].y_pos, box[bindx].width, box[bindx].height, box[bindx].fill_color);
//         tft.drawRect(box[bindx].x_pos, box[bindx].y_pos, box[bindx].width, box[bindx].height, box[bindx].border_color);


void box_structure_print(void)
{
    Serial.println("Box Strucure:");

    for (uint8_t box_group = (uint8_t) BOX_GROUP_1; box_group < BOX_GROUP_NBR_OF; box_group++)
    {
        Serial.printf("BoxDim:  %d: %d %d %d %d %d %d %d\n",
            box_group,   
            boxgr[box_group].y0,
            boxgr[box_group].x0,     
            boxgr[box_group].index,        
            boxgr[box_group].nbr_r,        
            boxgr[box_group].nbr_c, 
            boxgr[box_group].height,
            boxgr[box_group].width,       
            boxgr[box_group].round        
        );
    }

    for (uint8_t i = 0; i < box_ctrl.nbr; i++ )
    {
        Serial.printf("%d: %d %d %d %d %d %d %d %d %d %d %d \n",
            i,
            box[i].group,
            box[i].visible,
            box[i].x_pos,
            box[i].y_pos,
            box[i].width,
            box[i].height,
            box[i].font_indx,
            box[i].font_size,
            box[i].fill_color,
            box[i].border_color,
            box[i].text_color
        );
    }
    
}


tft_pin_check_st tft_pin_check[8] =
{
    { "BL  ",  PIN_TFT_LED,  TFT_BL},
    { "CS  ",  PIN_TFT_CS,   TFT_CS}, 
    { "RST ",  PIN_TFT_RST,  TFT_RST},
    { "DC  ",  PIN_TFT_DC,   TFT_DC},
    { "MISO",  PIN_TFT_MISO, TFT_MISO},
    { "CLK ",  PIN_TFT_CLK,  TFT_SCLK},
    { "MOSI",  PIN_TFT_MOSI, TFT_MOSI},
    { "T_CS",  PIN_TOUCH_CS, TOUCH_CS},  
};

void box_run_tft_pin_check(void)
{
    bool tft_setup_is_ok = true;
    bool driver_is_defined = false;

    #if TFT_TARGET_BOARD == BOARD_PICO_TFT_4KEYS
        Serial.println("BOARD_PICO_TFT_4KEYS was defined - OK");
    #elif TFT_TARGET_BOARD == BOARD_TFT_4_QUADCORE_PICO
        Serial.println("BOARD_TFT_4_QUADCORE_PICO was defined - OK");
    #else
        Serial.println("TFT_TARGET_BOARD was NOT defined - ERROR");
        tft_setup_is_ok = false;
    #endif    
    #ifdef ILI9341_DRIVER
        Serial.println("ILI9341_DRIVER was defined - OK");
        driver_is_defined = true;
    #endif    
    #ifdef ILI9488_DRIVER
        Serial.println("ILI9488_DRIVER was defined - OK");
        driver_is_defined = true;
    #endif

    if(!driver_is_defined){
        Serial.println("TFT DRIVER was not defined - ERROR");
        tft_setup_is_ok = false;
    }


    Serial.println("Pin  Design Library");
    for (uint8_t i = 0; i<8;i++)
    {
        Serial.print(tft_pin_check[i].label);
        Serial.print(": ");
        Serial.print(tft_pin_check[i].design_pin);
        Serial.print(" --> ");
        Serial.print(tft_pin_check[i].library_pin);
        if(tft_pin_check[i].design_pin == tft_pin_check[i].library_pin)
            Serial.println(" OK");
        else {
            tft_setup_is_ok = false;
            Serial.println(" ERROR");
        }  
    }

}