#ifndef __MSG_H__
#define __MSG_H__

#define MSG_MAX_FIELDS          20
#define MSG_MAX_FIELD_LEN       16
#define MSG_MAX_RAW_MSG_LEN     200
#define MSG_MAX_SMS_CMD_LEN     8
#define MSG_MAX_RFM_MSG_LEN     64


typedef enum 
{
    MSG_FROM_UNDEFINED = 0,
    MSG_FROM_UART,
    MSG_FROM_RFM,
    MSG_FROM_SMS,
    MSG_FROM_NBR_OF
} msg_from_et;

typedef enum
{
    SMS_CMD_HOME = 0,
    SMS_CMD_RELAY_PUMP,
    SMS_CMD_RELAY_PEER,
    SMS_CMD_SENSOR_PIHA1,
    SMS_CMD_SENSOR_REPO1,
    SMS_CMD_SENSOR_REPO2,
    SMS_CMD_ALL_TEMPERATURE,
    SMS_CMD_NBR_OF
} sms_cmd_type_et;


typedef struct
{
    char  field[MSG_MAX_FIELDS][MSG_MAX_FIELD_LEN];
    uint8_t count;
} fields_st;

typedef struct 
{
    char        buff[MSG_MAX_RAW_MSG_LEN];
    uint16_t    pos;
    uint8_t     from;
    char        field[MSG_MAX_FIELDS][MSG_MAX_FIELD_LEN];
    uint8_t     field_count;
    bool        avail;
} msg_st;



// typedef struct 
// {
//     char        raw[MSG_MAX_RAW_MSG_LEN];
//     char        rfm[MSG_MAX_RFM_MSG_LEN];
//     msg_from_et from;
//     fields_et   fields;
//     // char        fields[MSG_MAX_FIELDS][MSG_MAX_FIELD_LEN];
//     // uint8_t     field_count;
//     bool        rx_msg_avail;
// } msg_st;



void msg_initialize(void);

void str_to_upper(char str[]);

bool msg_is_valid_char(char c);

uint32_t msg_robust_atoi(const char *s, uint8_t *err_cntr, int min, int max );

uint8_t msg_split(msg_st *msg,  char separator);

size_t msg_set_sms_string(char *sms_str);

void msg_process_sms_cmd(void);

void msg_process(msg_from_et from, msg_st *msg);

void msg_sub_print(void);

void msg_time_action(void);

void msg_send_repo1(void);

void msg_send_all_temp(void);

#endif