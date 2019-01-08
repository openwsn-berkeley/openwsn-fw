#include "board.h"
#include "scheduler.h"
#include "opentimers.h"
#include "openhdlc.h"
#include "leds.h"
#include "uart.h"
#include "radio.h"
#include "eui64.h"

//=========================== defines ==========================================

#define UART_BUF_LEN         30
#define RF_BUF_LEN           125+LENGTH_CRC // maximum length is 127 bytes
#define MAC_LEN              8

#define TASK_PRIO_SERIAL     TASKPRIO_MAX
#define TASK_PRIO_WIRELESS   TASKPRIO_SIXTOP_TIMEOUT

#define TYPE_REQ_ST          1
#define TYPE_RESP_ST         2
#define TYPE_REQ_IDLE        3
#define TYPE_REQ_TX          4
#define TYPE_IND_TXDONE      5
#define TYPE_REQ_RX          6
#define TYPE_IND_RX          7
#define TYPE_IND_UP          8

#define ST_IDLE              1
#define ST_TX                2
#define ST_TXDONE            3
#define ST_RX                4

//=========================== frame formats ===================================

BEGIN_PACK
typedef struct {
   uint8_t         type;
} REQ_ST_ht;
END_PACK

BEGIN_PACK
typedef struct {
   uint8_t         type;
   uint8_t         status;
   uint16_t        numnotifications;
   uint8_t         mac[MAC_LEN];
} RESP_ST_ht;
END_PACK

BEGIN_PACK
typedef struct {
   uint8_t         type;
} REQ_IDLE_ht;
END_PACK

BEGIN_PACK
typedef struct {
   uint8_t         type;
   uint8_t         frequency;
    int8_t         txpower;
   uint16_t        transctr;
   uint16_t        txnumpk;
   uint16_t        txifdur;
   uint8_t         txlength;
   uint8_t         txfillbyte;
} REQ_TX_ht;
END_PACK

BEGIN_PACK
typedef struct {
   uint8_t         type;
} IND_TXDONE_ht;
END_PACK

BEGIN_PACK
typedef struct {
   uint8_t         type;
   uint8_t         frequency;
   uint8_t         srcmac[MAC_LEN];
   uint16_t        transctr;
   uint8_t         txlength;
   uint8_t         txfillbyte;
} REQ_RX_ht;
END_PACK

BEGIN_PACK
typedef struct {
   uint8_t         type;
   uint8_t         length;
   int8_t          rssi;
   uint8_t         flags;
   uint16_t        pkctr;
} IND_RX_ht;
END_PACK

BEGIN_PACK
typedef struct {
   uint8_t         type;
} IND_UP_ht;
END_PACK

BEGIN_PACK
typedef struct {
   uint8_t   srcmac[MAC_LEN];
   uint16_t  transctr;
   uint16_t  pkctr;
   uint8_t   txfillbyte;
   uint8_t   padding[RF_BUF_LEN-sizeof(uint8_t)-2*sizeof(uint16_t)];
} RF_PACKET_ht;
END_PACK

//=========================== variables =======================================

typedef struct {
   opentimers_id_t  sendTimerId;             ///< Each time expires, a packet is sent.

   //=== state machine
   uint8_t         status;
   uint16_t        numnotifications;

   //=== UART
   // tx
   uint8_t         uartbuftx[UART_BUF_LEN];
   uint8_t         uartbufrdidx;
   uint8_t         uartbuftxfill;
   uint8_t         uarttxescape;
   uint16_t        uarttxcrc;
   uint8_t         uarttxcrcAdded;
   uint8_t         uarttxclosingSent;
   // rx
   uint8_t         uartbufrx[UART_BUF_LEN];
   uint8_t         uartbufrxindex;
   uint8_t         uartlastRxByte;
   uint8_t         uartrxescaping;
   uint16_t        uartrxcrc;
   uint8_t         uartrxbusy;

   //=== stats
   uint16_t        uartNumRxCrcOk;
   uint16_t        uartNumRxCrcWrong;
   uint16_t        uartNumTx;
   uint16_t        serialNumRxOk;
   uint16_t        serialNumRxWrongLength;
   uint16_t        serialNumRxUnknownRequest;

   //=== RF
   // tx
   uint8_t         rfbuftx[RF_BUF_LEN];
   uint16_t        txpk_numpk;
   uint8_t         txpk_len;
   uint16_t        txpk_totalnumpk;
   uint8_t         mac[MAC_LEN];
   // rx
   uint8_t         rxpk_buf[RF_BUF_LEN];
   uint8_t         rxpk_len;
   uint8_t         rxpk_num;
    int8_t         rxpk_rssi;
   uint8_t         rxpk_lqi;
      bool         rxpk_crc;
   uint8_t         rxpk_txfillbyte;
   uint8_t         rxpk_srcmac[MAC_LEN];
   uint16_t        rxpk_transctr;
} mercator_vars_t;

mercator_vars_t mercator_vars;

//=========================== prototypes ======================================

void serial_enable(void);
void serial_disable(void);
void serial_flushtx(void);

void serial_rx_all(void);
void serial_rx_REQ_ST(void);
void serial_tx_RESP_ST(void);
void serial_rx_REQ_IDLE(void);
void serial_rx_REQ_TX(void);
void serial_rx_REQ_RX(void);
void serial_tx_IND_UP(void);

void isr_openserial_tx_mod(void);
uint8_t isr_openserial_rx_mod(void);

uint16_t htons(uint16_t val);

   // initial radio
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);
void cb_sendPacket(opentimers_id_t id);
void cb_finishTx(void);