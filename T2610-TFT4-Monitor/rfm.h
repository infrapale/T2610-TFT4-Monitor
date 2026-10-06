#ifndef __RFM_H__
#define __RFM_H__

#define RFM_MAX_MSG_LEN     200


typedef struct 
{
    msg_st  rx;
    msg_st  tx;
    // char        rxbuff[RFM_MAX_MSG_LEN];
    // char        txbuff[RFM_MAX_MSG_LEN];
    // fields_et   fields;
    // uint16_t    rxpos;
    // uint16_t    txpos;
    // bool        rx_msg_avail;
} rfm_st;

void rfm_initialize(void);

#endif