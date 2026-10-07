#include    <time.h>
#include    "main.h"
#include    "secrets.h"
#include    "io.h"
#include    "atask.h"
#include    "lte.h"
#include    "msg.h"
#include    "rfm.h"
#include    "sensor.h"

typedef struct
{
    uint16_t prev_state;
    uint16_t retry_cntr;
    uint32_t next_try;
    uint32_t wait_until;
    uint8_t   task_indx;
    time_t   now;
    struct tm timeinfo;
    char    buffer[64];
} lte_st;

lte_st lte ={0};

lte_msg_st lte_msg ={};

extern main_ctrl_st main_ctrl;
extern rfm_st rfm;
extern sensor_st sensor[SENSOR_NBR_OF];
extern sensor_value_st value_array[];


sms_cmd_st sms_cmd[SMS_CMD_NBR_OF] =
{
    {"HOME",    SMS_CMD_HOME},
    {"PUMP",    SMS_CMD_RELAY_PUMP},
    {"PEER",    SMS_CMD_RELAY_PEER},
    {"PIHA1",   SMS_CMD_SENSOR_PIHA1},
    {"REPO1",   SMS_CMD_SENSOR_REPO1},
    {"REPO2",   SMS_CMD_SENSOR_REPO2},
    {"TEMP",    SMS_CMD_ALL_TEMPERATURE},
};


contact_st contact_list[NAME_NBR_OF] =
{
    [NAME_UNKNOWN]= {"+358000000000", "Unknown"},
    [NAME_TOM]    = {"+358405056630", "Tom"},
    [NAME_HESSU]  = {"+358400454270", "Hessu"},
    [NAME_SAULI]  = {"+358400737682", "Sauli"},
};


// Function prototypes
bool lte_parse_msg_header(lte_msg_st *lte_msg, const char *msg);
void lte_task(void);
//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st lte_th              =    {"LTE Task       ", 100,    0,     0,  255,    0,  1,  lte_task };

void lte_initialize(void)
{
    LteSerial.begin(115200);
    lte.task_indx =  atask_add_new(&lte_th);
}

void lte_fast_read(void)
{
    // if (!lte_msg.available){
    if (LteSerial.available()){
        uint16_t len = lte_read_line(lte_msg.message, MSG_LEN, 1000);
        if (len > 0){
            Serial.printf("Message len: %d body: %s\n", len, lte_msg.body);
        }
    }
}

uint16_t lte_read_line(char *lp, uint16_t max_len, uint32_t timeout = 1000)
{
    uint32_t timeout_at = millis() + timeout;
    uint16_t cursor = 0;
    bool      do_continue = true;
    char    c;

    while ((millis() < timeout_at) && do_continue) {
        if (LteSerial.available()) {
            c = LteSerial.read();
            if( (c != '\r') && (c != '\n')){
                lp[cursor] = c;
                timeout_at = millis() + 10;
                //if (lp[cursor] == '\n') do_continue = false;
                if (cursor >= max_len-2) do_continue = false;
                cursor++;
            }
        } 
        else delay(2);
    }
    lp[cursor] = 0x00;
    // Serial.printf("cursor=%d - %s\n", cursor, lp);
    if (cursor > 0) lte_msg.available = true;
    return cursor;
}

// ------------------------------------------------------------
// Utility: Flush leftover UART data
// ------------------------------------------------------------
void lte_flush_serial(uint32_t timeout = 300) {
  uint32_t start = millis();
  while (millis() - start < timeout) {
    while (LteSerial.available()) {
      LteSerial.read();
      // start = millis();  !!CoPilot
    }
    delay(2);
  }
}

// ------------------------------------------------------------
// Send AT command and print response
// ------------------------------------------------------------
void lte_send_at(const char *cmd, uint32_t wait = 500) {
  lte_flush_serial();
  Serial.print(">> ");
  Serial.println(cmd);
  LteSerial.println(cmd);

  uint32_t start = millis();
  while (millis() - start < wait) {
    if (LteSerial.available()) {
      Serial.write(LteSerial.read());
    }
  }
  Serial.println();
}

void lte_clear_msg(lte_msg_st *lte_msg)
{
    lte_msg->complete = false;
    memset(lte_msg->sender,0x00,sizeof(lte_msg->sender));
    memset(lte_msg->timestamp,0x00,sizeof(lte_msg->timestamp));
    memset(lte_msg->message,0x00,sizeof(lte_msg->message));
    lte_msg->date_time.year = 0;
    lte_msg->date_time.month = 0;
    lte_msg->date_time.day = 0;
    lte_msg->date_time.hour = 0;
    lte_msg->date_time.minute = 0;
    lte_msg->date_time.second = 0;

}

void lte_print_msg(lte_msg_st *lte_msg)
{
    if(lte_msg->complete) Serial.print("Complete: ");
    else Serial.print("Draft: ");
    Serial.printf("From: %s %s\n", 
        lte_msg->sender, 
        contact_list[lte_msg->contact_indx].name);
    Serial.printf(" %d-%d-%d %d:%d:%d\n",
        lte_msg->date_time.year,
        lte_msg->date_time.month,
        lte_msg->date_time.day,
        lte_msg->date_time.hour,
        lte_msg->date_time.minute,
        lte_msg->date_time.second);
    Serial.print("Body: ");
    Serial.println(lte_msg->body);

}

uint8_t lte_get_contact_index(char *nbr)
{
    uint8_t contact = NAME_UNKNOWN;
    uint8_t cindx = NAME_UNKNOWN + 1;

    while((contact == NAME_UNKNOWN) && (cindx < NAME_NBR_OF))
    {
        //Serial.printf("Compare -%s-%s-\n",nbr,contact_list[cindx].sender);
        if (strcmp(nbr, contact_list[cindx].sender) == 0) contact = cindx;
        else cindx++;
    }
    return contact;
}
bool lte_parse_message(void)
{
    // +CMT: "+358401234567","","24/03/27,12:45:10+08"
    bool        parse_res = true;
    const char  *p = lte_msg.message;
    const char  *q;
    char        *end_ptr;
    char        ch_nbr[8];
    uint16_t    u16;
    size_t      len;

    if (strncmp(lte_msg.message, "+CMT", 4) == 0) {


        // char sender[32] = {0};
        // char timestamp[32] = {0};

        // Find first quoted field (sender)
        p = strchr(p, '"');
        if (!p) return false;
        q = strchr(p + 1, '"');
        if (!q) return false;
        size_t len = q - (p + 1);
        if (len < sizeof(lte_msg.sender))
            memcpy(lte_msg.sender, p + 1, len);

        Serial.printf("[INFO] Sender: %s\n", lte_msg.sender);

        // Timestamp: 26/04/22,11:08:12+12
        // Find timestamp (4th quoted field)
        p = strchr(q + 1, '"'); if (!p) return false;
        p = strchr(p + 1, '"'); if (!p) return false;
        p = strchr(p + 1, '"'); if (!p) return false;
        q = strchr(p + 1, '"'); if (!q) return false;
        len = q - (p + 1);
        if (len < sizeof(lte_msg.timestamp))
        {
            //Serial.print("[parse header] 110: ");
            memcpy(lte_msg.timestamp, p+1, len);
            lte_msg.timestamp[len] = '\0';
            p = lte_msg.timestamp;
            q = strchr(p + 1, '/');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg.date_time.year = u16;

            p = q+1;
            q = strchr(p + 1, '/');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg.date_time.month = (uint8_t)u16;

            p = q+1;
            q = strchr(p + 1, ',');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg.date_time.day = (uint8_t)u16;

            p = q+1;
            q = strchr(p + 1, ':');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg.date_time.hour = (uint8_t)u16;
            p = q+1;
            q = strchr(p + 1, ':');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg.date_time.minute = (uint8_t)u16;
            p = q+1;
            q = strchr(p + 1, '+');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg.date_time.second = (uint8_t)u16;

            p = lte_msg.message;
            q = strchr(p, '+');
            p = q+1;
            q = strchr(p, '+');
            p = q+1;
            q = strchr(p, '+');
            p = q+1;
            q = strchr(p, '\"');
            p = q+1;

            strncpy(lte_msg.body,p,SMS_LEN);       
        } else parse_res = false;
        //Serial.print("[INFO] Timestamp: "); Serial.println(lte_msg.timestamp);

        //strncpy(lte_msg.sender, sender, sizeof(lte_msg->sender));
        // strncpy(lte_msg->timestamp, timestamp, sizeof(lte_msg->timestamp));
    } else {
        parse_res = false;
        lte_clear_msg(&lte_msg);
        lte_msg.available = false;
    }
    return parse_res;
}

char *lte_get_sender_nbr(void)
{
    return lte_msg.sender;
}

void lte_reply_msg(lte_msg_st *lte_msg)
{ 
    char reply[SMS_LEN] = {0};
    char at_cmd[40];

    uint16_t ch_avail = SMS_LEN;
    char *cp;
    uint16_t  len;

    strncpy(reply,"Terve ", ch_avail);
    ch_avail = SMS_LEN - strlen(reply);

    cp = contact_list[lte_msg->contact_indx].name;
    len = strlen(cp);
    if (ch_avail > len) {
        strncat(reply, cp, ch_avail);
        ch_avail = SMS_LEN - strlen(reply);
    }

    cp = (char*)"  Tupa Temp = 22.3C";
    len = strlen(cp);
    if (ch_avail > len) {
        strncat(reply, cp, ch_avail);
        ch_avail = SMS_LEN - strlen(reply);
    }

    sprintf(at_cmd, "AT+CMGS=\"%s\"", lte_msg->sender);
    lte_send_at(at_cmd);
    LteSerial.print(reply);
    LteSerial.write(26);
    Serial.printf("Reply: %s\n", reply);
}


void lte_send_msg(char *to_nbr, char *sms_msg)
{
    char at_cmd[40];

    // Serial.printf("To nbr: %s\n", to_nbr);
    sprintf(at_cmd, "AT+CMGS=\"%s\"", to_nbr);
    lte_send_at(at_cmd);
    LteSerial.print(sms_msg);
    LteSerial.write(26);
    Serial.printf("SMS Sent: %s to %s\n", sms_msg, to_nbr);
}

// ------------------------------------------------------------
// Modem Boot Sequence
// ------------------------------------------------------------
void lte_modem_boot() {
  Serial.println("[ACTION] Resetting modem…");
  digitalWrite(PIN_RESET, LOW);
  delay(500);
  digitalWrite(PIN_RESET, HIGH);
  delay(1500);

  Serial.println("[ACTION] Powering modem ON…");
  digitalWrite(PIN_PWRKEY, LOW);
  delay(800);
  digitalWrite(PIN_PWRKEY, HIGH);
  delay(3000);
}

// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------
void lte_setup() {
    lte_clear_msg(&lte_msg);
    lte_send_at("AT");
    lte_send_at("ATE0");
    lte_send_at("AT+CPIN=\"1234\"");
    lte_send_at("AT+CMGF=1");          // SMS text mode
    lte_send_at("AT+CNMI=2,2,0,0,0");  // Push SMS immediately 
}



size_t lte_set_sms_string(char *sms_str, msg_st *msg)
{
    str_to_upper(sms_str);
    Serial.printf("msg_set_sms_string: %s\n", sms_str);
    msg->buff[0] = '<';
    strncpy(&msg->buff[1],sms_str, MSG_MAX_RAW_MSG_LEN-1);
    size_t len = strnlen(msg->buff,MSG_MAX_RAW_MSG_LEN);
    msg->buff[len++] = '>';
    msg->buff[len++] = 0x00;
    msg_split(msg, ' ');
    Serial.printf(" ... %s\n", msg->buff);
    // msg_sub_print();
    return len;
} 



void msg_send_repo1(void)
{
    char    buff[SMS_LEN];
    uint8_t arr_indx[4]; 
    arr_indx[0] = sensor[SENSOR_KHH].value_indx[VALUE_TEMPERATURE];
    arr_indx[1] = sensor[SENSOR_KHH].value_indx[VALUE_HUMIDITY];

    sprintf(buff,"KHH: %0.1fC, Min: %0.1fC, Max: %0.1fC, Avg: %0.1fC, Nbr: %d, Hum: %d%",
        value_array[arr_indx[0]].last,
        value_array[arr_indx[0]].min,
        value_array[arr_indx[0]].max,
        value_array[arr_indx[0]].average,
        value_array[arr_indx[0]].daily_cntr,
        value_array[arr_indx[1]].last
    );
    Serial.println(buff);
    // lte_send_msg(lte_get_sender_nbr(), buff);
}

void msg_send_ruuvi_repo(uint8_t sindx)
{
    char    buff[SMS_LEN];
    uint8_t arr_indx[4]; 
    arr_indx[0] = sensor[sindx].value_indx[VALUE_TEMPERATURE];
    arr_indx[1] = sensor[sindx].value_indx[VALUE_HUMIDITY];
    arr_indx[2] = sensor[sindx].value_indx[VALUE_BAT];

    sprintf(buff,"%s: %0.1fC, Min: %0.1fC, Max: %0.1fC, Avg: %0.1fC, Nbr: %d, Hum: %0.1f, Bat: %0.1fV",
        sensor[sindx].label,
        value_array[arr_indx[0]].last,
        value_array[arr_indx[0]].min,
        value_array[arr_indx[0]].max,
        value_array[arr_indx[0]].average,
        value_array[arr_indx[0]].daily_cntr,
        value_array[arr_indx[1]].last,
        value_array[arr_indx[2]].last
    );
    Serial.println(buff);
    lte_send_msg(lte_get_sender_nbr(), buff);
}

void safe_append(char *dst, size_t dst_size,  const char *src)
{
    size_t len_dst = strnlen(dst, dst_size);
    size_t len_src = strnlen(src, dst_size);

    if (len_dst + len_src + 1 > dst_size) {
        // Not enough space — truncate safely
        size_t copy_len = dst_size - len_dst - 1;
        memcpy(dst + len_dst, src, copy_len);
        dst[dst_size - 1] = '\0';
        return;
    }

    memcpy(dst + len_dst, src, len_src);
    dst[len_dst + len_src] = '\0';
}

#define ONE_SENSOR_LEN  32
void msg_send_all_temp(void)
{
    char    buff[SMS_LEN] = {0};
    char    one_buff[ONE_SENSOR_LEN];

    for(uint8_t sindx = SENSOR_UNDEFINED + 1; sindx < SENSOR_NBR_OF; sindx++)
    {
        sprintf(one_buff,"%s: %0.1fC,",
            sensor[sindx].label,
            value_array[sensor[sindx].value_indx[VALUE_TEMPERATURE]].last
        );       
        safe_append(buff, SMS_LEN, one_buff);
    }

    Serial.println(buff);
    lte_send_msg(lte_get_sender_nbr(), buff);
}




void lte_process_sms_cmd(msg_st *msg)
{
    int cmd_indx = -1; 
    int16_t param;
    char    buff[SMS_LEN];

    Serial.println("msg_process_sms_cmd");
    for(uint8_t i = 0; ((i < SMS_CMD_NBR_OF) && (cmd_indx == -1)); i++)
    {
        if(strncmp(msg->field[0], sms_cmd[i].cmd, MSG_MAX_SMS_CMD_LEN) == 0) cmd_indx = i;
        Serial.printf("sms %s - %s: %d\n", msg->field[0], sms_cmd[i].cmd, cmd_indx);
    }
    Serial.printf("cmd_indx= %d\n", cmd_indx);

    if (cmd_indx != -1)
    {
        param = atoi(msg->field[1]);
        switch(cmd_indx)
        {
            case SMS_CMD_HOME:
                break;
            case SMS_CMD_RELAY_PUMP:
                sprintf(buff,"<R;RANTA;%s;PUMP;%d>", main_ctrl.my_addr, param);
                //r69_send(buff);
                Serial.println(buff);
                break;
            case SMS_CMD_RELAY_PEER:
                sprintf(buff,"<R;RANTA;%s;PEER;%d>", main_ctrl.my_addr, param);
                //r69_send(buff);
                Serial.println(buff);
                break;
            case SMS_CMD_SENSOR_PIHA1:
                sprintf(buff,"<S;#;PIHA1;T;-12.3;H;44;L;2344>");
                Serial.println(buff);
                break;
            case SMS_CMD_SENSOR_REPO1:
                msg_send_repo1();
                break;
            case SMS_CMD_SENSOR_REPO2:
                msg_send_ruuvi_repo(SENSOR_PARVEKE);
                //Serial.println(buff);
                break;
            case SMS_CMD_ALL_TEMPERATURE:
                msg_send_all_temp();
                //Serial.println(buff);
                break;
            default:
                break;
        }

    }

}



void lte_task(void)
{
    static lte_msg_et header_status = LTE_MSG_UNDEF;
    static lte_msg_et body_status = LTE_MSG_UNDEF;
    static uint32_t body_timeout;
    
    switch(lte_th.state)
    {
        case 0:
            lte_th.state = 10;
            lte.wait_until = millis() + 3200;
            Serial.println("=== Pico 2 W + A7683E SMS Framework ===");

            io_led_flash(LED_INDX_YELLOW, BLINK_JITTER_1, 40);            
            break;
        case 10:
            Serial.println("[ACTION] Resetting modem…");
            digitalWrite(PIN_RESET, LOW);
            lte.wait_until = millis() + 500;
            lte_th.state = 15;
            break;
        case 15:
            if (millis() > lte.wait_until)
            {
                digitalWrite(PIN_RESET, HIGH);
                lte.wait_until = millis() + 1500;
                lte_th.state = 20;
            }
            break;
        case 20:
            if (millis() > lte.wait_until)
            {
                Serial.println("[ACTION] Powering modem ON…");
                digitalWrite(PIN_PWRKEY, LOW);
                lte.wait_until = millis() + 800;
                lte_th.state = 25;
            }
            break;
        case 25:
            if (millis() > lte.wait_until)
            {
                digitalWrite(PIN_PWRKEY, HIGH);
                lte.wait_until = millis() + 3000;
                lte_th.state = 30;
            }
            break;
        case 30:    
            if (millis() > lte.wait_until) lte_th.state = 50;
            break;
        case 50:
            lte_setup();
            Serial.println("[INFO] Setup complete. Waiting for SMS…");
            lte_th.state = 100;
            break;
        case 100:
            if (lte_msg.available){
                io_led_flash(LED_INDX_YELLOW, BLINK_NORMAL, 80); 
                lte_th.state = 110; 
            } 
            break;
        case 110:
            lte_msg.complete = false;
            Serial.println(lte_msg.message);
            if (lte_parse_message()) {
                lte_th.state = 120; 
                lte_msg.complete = true;
                lte_set_sms_string( lte_msg.body, &rfm.tx);
                lte_process_sms_cmd(&rfm.tx);

            }
            else {
                Serial.println("Incorrect message");
                io_led_flash(LED_INDX_YELLOW, BLINK_SOS, 100); 
                lte_clear_msg(&lte_msg);
                lte_msg.available = false;
                lte_th.state = 100;
            }
            break;
        case 120:
            lte_msg.contact_indx = (contact_indx_st)lte_get_contact_index(lte_msg.sender);
            lte_print_msg(&lte_msg);
            lte_th.state = 200;
            break;

        case 200:
            // lte_reply_msg(&lte_msg);
            lte_clear_msg(&lte_msg);
            lte_msg.available = false;
            lte_th.state = 100;
            break;

    }
    if(lte_th.state != lte.prev_state){
        Serial.printf("LTE State %d -> %d\n", lte.prev_state, lte_th.state);
        lte.prev_state = lte_th.state;
    }
}


