#include <I2S.h>
#include <SPI.h>
#include "SdFat.h"
#include "sd.h"

#include "main.h"
#include "dingdong.h"
#include "io.h"

#define SD_RETRY_MAX 8


typedef struct
{
    bool sd_ok;
    bool wav_ok;
    bool i2s_ok;
    bool more_data;
} dingdong_ctrl_st;

dingdong_ctrl_st dingdong = {.wav_ok=false, .i2s_ok=false, .more_data=false};


FsFile wavFile;

// WAV header structure
struct WAVHeader {
  char riff[4];
  uint32_t chunkSize;
  char wave[4];
  char fmt[4];
  uint32_t subchunk1Size;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
  char dataHeader[4];
  uint32_t dataSize;
};

char wav_files[WAV_NBR_OF][40] =
{

    [WAV_BIG_BEN]           = {"bigben.wav"},
    [WAV_SENSOR_BEEP]       = {"censor_beep1.wav"},
    [WAV_DIAL_CALL]         = {"dial_call.wav"},
    [WAV_OLD_CHURCH]        = {"old_chrch.wav"},
    [WAV_PHONE_RINGING]     = {"phone_ringing.wav"},
    [WAV_ROTARY_PHONE_HIGH] = {"rotary_phone_high.wav"},
    [WAV_ROTARY_PHONE_LOW]  = {"rotary_phone_low.wav"},
    [WAW_1]                 = {"typewriter_slow.wav"},
    [WAV_2]                 = {"typewriter_slow.wav"},
};
    


extern SdFat SD;
//extern SdSpiConfig config(PIN_SD_CS, DEDICATED_SPI, SD_SCK_MHZ(12), &SPI1);
WAVHeader header;
I2S i2s(OUTPUT, PIN_I2S_BCLK, PIN_I2S_DOUT);


void dingdong_initialize() {
    dingdong.sd_ok = sd_initialize();
}  

void dingdong_play_wav(wav_files_et wav_indx)
{
    if (dingdong.sd_ok)
    {
        //wavFile = SD.open("bigben.wav");
        wavFile = SD.open(wav_files[wav_indx]);
        if (!wavFile) {
            Serial.println("Failed to open WAV file");
            dingdong.wav_ok = false;
        }
        else dingdong.wav_ok = true;

        if(dingdong.wav_ok)
        {


            // Read WAV header
            wavFile.read(&header, sizeof(WAVHeader));

            Serial.print("Sample rate: ");
            Serial.println(header.sampleRate);
            Serial.print("Bits: ");
            Serial.println(header.bitsPerSample);
            Serial.print("Channels: ");
            Serial.println(header.numChannels);
            Serial.print("audioFormat: "); Serial.println(header.audioFormat);
            Serial.print("bitsPerSample: "); Serial.println(header.bitsPerSample);
            Serial.print("numChannels: "); Serial.println(header.numChannels);
            Serial.print("dataHeader: "); Serial.write(header.dataHeader, 4); Serial.println();
            Serial.write(header.riff, 4);      Serial.println();
            Serial.write(header.wave, 4);      Serial.println();
            Serial.write(header.fmt, 4);       Serial.println();
            Serial.write(header.dataHeader, 4);Serial.println();



            i2s.setBitsPerSample(header.bitsPerSample);
            //i2s.setFrequency(header.sampleRate);

            if (i2s.begin(header.sampleRate)) {
                dingdong.i2s_ok = true;  
            }  
            else{  
                Serial.println("I2S begin failed!");
                dingdong.i2s_ok = false;
            }
            if(dingdong.i2s_ok)
            {
                Serial.println("Playing...");
                static uint8_t buffer[512];
                dingdong.more_data = true;
                while(dingdong.more_data)
                {
                    if (!wavFile.available()) {
                      Serial.println("Done.");
                      wavFile.close();
                      dingdong.more_data = false;;
                    } else {
                        int bytesRead = wavFile.read(buffer, sizeof(buffer));
                        if (bytesRead <= 0) dingdong.more_data = false;
                        else{
                            int bytesPerSample = header.bitsPerSample / 8;
                            int frameSize = bytesPerSample * header.numChannels;

                            for (int i = 0; i + frameSize <= bytesRead; i += frameSize) {

                                int16_t sample16 = 0;

                                if (header.numChannels == 1) {
                                    sample16 = (int16_t)(buffer[i] | (buffer[i+1] << 8));
                                } else {
                                    int16_t left  = (int16_t)(buffer[i] | (buffer[i+1] << 8));
                                    int16_t right = (int16_t)(buffer[i+2] | (buffer[i+3] << 8));
                                    sample16 = (left / 2) + (right / 2);
                                }

                                // ⭐ Write 16‑bit samples, exactly like your tone sketch
                                i2s.write(sample16); // Left
                                i2s.write(sample16); // Right
                            }
                        }
                    }
                }
            }
            i2s.end();
        }
    }
    else 
        Serial.println("SD initialization failed!");

}

void dingdong_play_all(void)
{
      for(uint8_t windx = WAV_BIG_BEN; windx < WAV_NBR_OF; windx++)
    {
        Serial.println(wav_files[windx]);
        dingdong_play_wav((wav_files_et)windx);
    }
    delay(2000);

}


