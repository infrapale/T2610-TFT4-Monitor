#include    "main.h"
// #include    "secrets.h"
#include    "atask.h"
#include    "io.h"
#include    "msg.h"

#define BUFF_LEN   160
char mbuff[BUFF_LEN];

void print_debug_task(void);
atask_st debug_th       =     {"Debug Task     ", 10000,    0,     0,  255,    0,  1,  print_debug_task };

extern msg_st msg;


void setup() {
  // put your setup code here, to run once:
    Serial1.setTX(PIN_TX0);   
    Serial1.setRX(PIN_RX0);

    Serial.begin(115200);
    Serial1.begin(9600);
    delay(2000);

    atask_initialize();
    //atask_add_new(&debug_th);
    msg_initialize();
    Serial.println("Hello");
}

uint8_t rx_pos = 0;
uint32_t rx_timeout;


void loop() {
    // --- REAL-TIME UART READER ---
    while (Serial1.available()) {
        char c = Serial1.read();

        if (c == '\n' || c == '\r') {
            if (rx_pos > 0) {
                msg.raw[rx_pos] = 0;
                msg.rx_msg_avail = true;   // signal to msg_task()
                rx_pos = 0;
            }
        } else {
            if (rx_pos < MSG_MAX_RAW_MSG_LEN - 1) {
                msg.raw[rx_pos++] = c;
            } else {
                // overflow protection
                msg.raw[MSG_MAX_RAW_MSG_LEN - 1] = 0;
                rx_pos = 0;
            }
        }
    }

    // --- RUN SCHEDULER ---
    atask_run();   // or whatever your scheduler call is
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
