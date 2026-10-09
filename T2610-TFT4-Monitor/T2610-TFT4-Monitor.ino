#include    "main.h"
// #include    "secrets.h"
#include    "atask.h"
#include    "io.h"
#include    "menu.h"
#include    "time_func.h"
#include    "msg.h"
#include    "clock.h"
#include    "lte.h"
#include    "sensor.h"
#include    "rfm.h"
#include    "super.h"
#include    "box.h"
#include    "dashboard.h"


#define BUFF_LEN   160
char mbuff[BUFF_LEN];

main_ctrl_st main_ctrl = {
    .next_io_tick = 0,
    .my_addr = "SMS1",
};


void print_debug_task(void);
atask_st debug_th       =     {"Debug Task     ", 10000,    0,     0,  255,    0,  1,  print_debug_task };

extern msg_st msg;


void setup() {
  // put your setup code here, to run once:
    Serial1.setTX(PIN_TX0);   //SerialLte connected to Clipper 4G
    Serial1.setRX(PIN_RX0);   
    Serial2.setTX(PIN_TX1);   //SerialRfm connected to RFM MCU 
    Serial2.setRX(PIN_RX1);   // RFM69 and RuuviTag


    Serial.begin(115200);
    Serial1.begin(115200);
    Serial2.begin(9600);
    delay(2000);
    Serial.println(__APP__);
    Serial.printf(" Compiled: %s %s\n",__DATE__, __TIME__);

    atask_initialize();
    atask_add_new(&debug_th);
    sensor_initialize();
    lte_initialize();
    msg_initialize();
    rfm_initialize();
    clock_initialize();

    box_run_tft_pin_check();
    box_initialize();
    box_structure_print();
    dashboard_initialize();   // start dashboard task
    menu_initialize();        // starting scan and read tasks
    // dingdong_initialize();
    // dingdong_play_all();

}

uint8_t rx_pos = 0;
uint8_t rfm_pos = 0;
uint32_t rx_timeout;


void loop() {
     // --- RUN SCHEDULER ---
    atask_run();   // or whatever your scheduler call is
    lte_fast_read();
    super_clear_cntr(SUPER_CNTR_LOOP);
}




void print_debug_task(void)
{
    atask_print_status(true);
    io_debug_print();

}
