#include <SPI.h>
#include "SdFat.h"
#include "main.h"
#include "io.h"

//  --- Start SPI1 ---
// SPI1.begin();     // or SPI1.begin(true) for hardware CS

// // Example transfer
// SPI1.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
// uint8_t rx = SPI1.transfer(0x55);
// SPI1.endTransaction();
#define SD_RETRY_MAX 8

typedef struct
{
    bool enable;
    FsFile file_1;

} sd_ctrl_st;

sd_ctrl_st sd_ctrl = {.enable=false};

SdFat SD;
FsFile myFile;
SdSpiConfig config(PIN_SD_CS , DEDICATED_SPI, SD_SCK_MHZ(16), &SPI1);


bool sd_initialize(void)
{
    SPI1.setRX(PIN_SPI_1_MISO); // or setMISO()
    SPI1.setCS(PIN_SD_CS);
    SPI1.setSCK(PIN_SPI_1_SCK);
    SPI1.setTX(PIN_SPI_1_MOSI); // or setMOSI()
    SPI1.begin(false);

    uint8_t retry_cntr = 0;

    while ( (retry_cntr < SD_RETRY_MAX ) && !sd_ctrl.enable)
    {
        sd_ctrl.enable = SD.begin(config);
        retry_cntr++;
        delay(1000); // Wait for a second before retrying        
    }
    if(sd_ctrl.enable) Serial.println("initialization done.");
    else Serial.println("initialization failed!");
    return sd_ctrl.enable;
}

bool sd_is_enabled(void)
{
    return sd_ctrl.enable;
}

void sd_example(void)
{
    Serial.print("Begin Transaction...");
    SPI1.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    //uint8_t rx = SPI1.transfer(0x55);
    //SPI1.endTransaction();

    Serial.print("SD begin...");
    // Retry mechanism for SD card initialization
    while (!SD.begin(config)) {
      Serial.println("initialization failed! Retrying...");
      delay(1000); // Wait for a second before retrying
    }
    Serial.println("initialization done.");

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    sd_ctrl.file_1 = SD.open("test.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (myFile) {
      Serial.print("Writing to test.txt...");
      sd_ctrl.file_1.println("testing 1, 2, 3.");
      sd_ctrl.file_1.println("hello world!");
      // close the file:
      sd_ctrl.file_1.close();
      Serial.println("done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }

}
