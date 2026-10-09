#ifndef __IO_H__
#define __IO_H__

#define T2601_PICO_RFM69

//#define SERIAL_TFT      Serial2
#define SerialLte       Serial1
#define SerialRfm       Serial2


#ifdef  MCU_PICO_PLUS_2
    #define PIN_TX0     (32u)
    #define PIN_RX0     (33u)
    #define PIN_PWRKEY  (36u)
    #define PIN_RESET   (35u)
#else
    #define PIN_TX0     (0u)
    #define PIN_RX0     (1u)
#endif

#define PIN_I2C1_SDA    (2u)
#define PIN_I2C1_SCL    (3u)

#define PIN_TX1         (4u)
#define PIN_RX1         (5u)

#define PIN_LED_RED     (6u)
#define PIN_LED_BLUE    (7u)

// I2S Audio Out
#define PIN_I2S_BCLK    (6u)
#define PIN_I2S_LRCLK   (7u)
#define PIN_I2S_DOUT    (8u)
// SD Card SPI
#define PIN_SPI_1_SCK   (10u)
#define PIN_SPI_1_MOSI  (11u)
#define PIN_SPI_1_MISO  (12u)
#define PIN_SD_CS       (13u)
// TFT SPI
#define PIN_TFT_RST     (9u)
#define PIN_TFT_LED     (14u)
#define PIN_TFT_CS      (17u)
#define PIN_TFT_DC      (15u)
#define PIN_TFT_MISO    (16u)
#define PIN_TFT_CLK     (18u)
#define PIN_TFT_MOSI    (19u)
#define PIN_TOUCH_CS    (-1)

#define PIN_PIR         (20u)
#define PIN_LED_YELLOW  (21u)
#define PIN_RUN_RFM     (22u)
#define PIN_LDR_AN      (26u)
#define PIN_ABTN        (A1)

//#define PIN_WD_ENABLE   PIN_DIP_SW1


#define BLINK_DISABLE  (9998)
#define BLINK_FOREVER  (9999)
#define IO_DIP_SW_NBR_OF    8

typedef enum
{
    LED_INDX_YELLOW =0,
    LED_INDX_NBR_OF
} led_index_et;

// TFT Library Check
typedef struct 
{
    char        label[5];
    int8_t      design_pin;
    int8_t      library_pin;
} tft_pin_check_st;

typedef enum
{
  BLINK_OFF = 0,
  BLINK_ON,
  BLINK_1_FLASH,
  BLINK_2_FLASH,
  BLINK_4_FLASH,
  BLINK_SLOW,
  BLINK_NORMAL,
  BLINK_FAST,
  BLINK_SOS,
  BLINK_JITTER_1,
  BLINK_JITTER_2,
  BLINK_JITTER_3,
  BLINK_NBR_OF
} blink_et;

void io_initialize(void);

void io_task_initialize(void);

void io_led_flash(led_index_et color, blink_et bindx, uint16_t tick_nbr);

void io_task(void);

bool io_wd_is_enabled(void);

void io_debug_print(void);


#endif
