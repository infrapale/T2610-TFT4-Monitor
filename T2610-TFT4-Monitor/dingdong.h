#ifndef __DINGDONG_H__
#define __DINGDONG_H__

typedef enum
{
    WAV_BIG_BEN = 0,
    WAV_SENSOR_BEEP,
    WAV_DIAL_CALL,
    WAV_OLD_CHURCH,
    WAV_PHONE_RINGING,
    WAV_ROTARY_PHONE_HIGH,
    WAV_ROTARY_PHONE_LOW,
    WAW_1,
    WAV_2,
    WAV_NBR_OF
} wav_files_et;


void dingdong_initialize();
void dingdong_play_wav(wav_files_et wav_indx);
void dingdong_play_all(void);


#endif