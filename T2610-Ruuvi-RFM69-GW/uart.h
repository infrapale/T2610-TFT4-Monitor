#ifndef __UART_H__
#define __UART_H__

#define UART_BUFF_LEN   80

typedef struct
{
    char        buff[UART_BUFF_LEN];
    uint8_t     field_count;
    uint16_t    pos;
    uint16_t    len;
    bool        avail;
}  rxtx_st;

typedef struct
{
    rxtx_st tx;
    rxtx_st rx;
} uart_st;

void uart_initialize(void);

void uart_tft_send(char *buff);


#endif