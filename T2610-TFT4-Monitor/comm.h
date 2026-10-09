#ifndef __COMM_H__
#define __COMM_H__

#define MSG_TAG_LEN     8
#define CMD_MAX_VALUES  8
#define CMD_FIELD_LEN   8
#define COMM_BUFF_LEN   64

typedef enum
{
    COMM_INDX_0   = 0,
    COMM_INDX_1,
    COMM_INDX_2,
    COMM_INDX_NBR_OF,
} comm_indx_et;


typedef enum 
{
    COMM_MSG_UNDEFINED = 0,
    COMM_MSG_DATE_TIME,
    COMM_MSG_RUUVI,
    COMM_MSG_RFM_SENSOR,
    COMM_MSG_X,
    COMM_MSG_NBR_OF,
} comm_msg_type_et;

typedef struct
{
    char tx_buff[COMM_BUFF_LEN];
    char rx_buff[COMM_BUFF_LEN];
    bool rx_avail;
} comm_st;


typedef struct
{
    uint32_t    send_time_upate_at;
    uint8_t     nbr_values;
    char        field[CMD_MAX_VALUES][CMD_FIELD_LEN];
    comm_msg_type_et msg_type;
} comm_data_st;

// void comm_initialize(void);

// void comm_1_read(void);

#endif
