#include    "main.h"
// #include    "secrets.h"
#include    "atask.h"
#include    "io.h"
#include    "msg.h"
#include    "lte.h"
#include    "sensor.h"
#include    "rfm.h"
#include    "super.h"

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

    atask_initialize();
    atask_add_new(&debug_th);
    sensor_initialize();
    lte_initialize();
    msg_initialize();
    rfm_initialize();
    Serial.println("Hello");
}

uint8_t rx_pos = 0;
uint8_t rfm_pos = 0;
uint32_t rx_timeout;


void loop() {
    // // --- REAL-TIME UART READER ---
    // while (Serial1.available()) {
    //     char c = Serial1.read();

    //     if (c == '\n' || c == '\r') {
    //         if (rx_pos > 0) {
    //             msg.raw[rx_pos] = 0;
    //             msg.rx_msg_avail = true;   // signal to msg_task()
    //             rx_pos = 0;
    //         }
    //     } else {
    //         if (rx_pos < MSG_MAX_RAW_MSG_LEN - 1) {
    //             msg.raw[rx_pos++] = c;
    //         } else {
    //             // overflow protection
    //             msg.raw[MSG_MAX_RAW_MSG_LEN - 1] = 0;
    //             rx_pos = 0;
    //         }
    //     }
    // }

    // while (SerialRfm.available()) {
    //     char c = SerialRfm.read();

    //     //Serial.printf("@%d-%c\n",rfm_pos,c);
    //     if (c == '\n' || c == '\r') {
    //         if (rfm_pos > 0) {
    //             msg.rfm[rfm_pos] = 0;
    //             //msg.rx_msg_avail = true;   // signal to msg_task()
    //             Serial.println(msg.rfm);
    //             rfm_pos = 0;
    //         }
    //     } else {
    //         if (rfm_pos < MSG_MAX_RFM_MSG_LEN - 1) {
    //             msg.rfm[rfm_pos++] = c;
    //         } else {
    //             // overflow protection
    //             msg.rfm[MSG_MAX_RFM_MSG_LEN - 1] = 0;
    //             rfm_pos = 0;
    //         }
    //     }
    // }



    // --- RUN SCHEDULER ---
    atask_run();   // or whatever your scheduler call is
    lte_fast_read();
    super_clear_cntr(SUPER_CNTR_LOOP);
}


// void loop() {
//     atask_run();
//     // Read from Serial1 (UART) and buffer until newline
//     // while (Serial1.available()) {
//     //     char c = Serial1.read();

//     //     // Print raw characters for debugging
//     //     // Serial.write(c);

//     //     if (c == '\n' || c == '\r') {
//     //         // End of line → terminate and print full buffer
//     //         mbuff[mpos] = 0;      // null‑terminate
//     //         if (mpos > 0) {
//     //             // Serial.print("UART LINE: ");
//     //             Serial.println(mbuff);
//     //         }
//     //         mpos = 0;             // reset buffer
//     //     } else {
//     //         // Add to buffer if space remains
//     //         if (mpos < BUFF_LEN - 1) {
//     //             mbuff[mpos++] = c;
//     //         } else {
//     //             // Overflow protection
//     //             mbuff[BUFF_LEN - 1] = 0;
//     //             Serial.println("UART buffer overflow");
//     //             mpos = 0;
//     //         }
//     //     }
//     // }

//     // Optional: also read from USB Serial and forward to Serial1
//     // while (Serial.available()) {
//     //     char c = Serial.read();
//     //     Serial1.write(c);
//     // }

// }


void print_debug_task(void)
{
    atask_print_status(true);
}
