/**
\brief Mercator firmware, see https://github.com/openwsn-berkeley/mercator/.

\author Constanza Perez Garcia <constanza.perezgarcia@gmail.com>, November 2014.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, November 2014.
*/

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
#define RF_BUF_LEN           128

#define TASK_PRIO_SERIAL     TASKPRIO_MAX
#define TASK_PRIO_WIRELESS   TASKPRIO_SIXTOP_TIMEOUT

#define TYPE_REQ_ST          1
#define TYPE_RESP_ST         2
#define TYPE_REQ_IDLE        3
#define TYPE_REQ_TX          4
#define TYPE_IND_TXDONE      5
#define TYPE_REQ_RX          6 
#define TYPE_IND_RX          7

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
   uint8_t         mac[8];
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
   uint8_t         transctr;
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
   uint8_t         srcmac[8];
   uint8_t         transctr;
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

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  sendTimerId;             ///< Each time expires, a packet is sent.
   
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
   uint8_t         uartbufrxfill;
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
   uint8_t         txpk_totalnumpk;
   uint8_t         mac[8];
   // rx
   uint8_t         rxpk_buf[RF_BUF_LEN];
   uint8_t         rxpk_len;
   uint8_t         rxpk_num;
    int8_t         rxpk_rssi;
   uint8_t         rxpk_lqi;
      bool         rxpk_crc;
   uint8_t         rxpk_txfillbyte;
   uint8_t         rxpk_srcmac[8];
   uint8_t         rxpk_transctr;
   
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

void isr_openserial_tx_mod(void);
void isr_openserial_rx_mod(void);

uint16_t htons(uint16_t val);

void cb_endFrame(uint16_t timestamp);
void cb_sendPacket(void);
void cb_finishTx(void);

//=========================== initialization ==================================

int mote_main(void) {
   
   memset(&mercator_vars,0,sizeof(mercator_vars_t));
   mercator_vars.status = ST_IDLE;
   
   board_init();
   scheduler_init();
   opentimers_init();
   radio_init();
   leds_init();

   // get mac
   eui64_get(mercator_vars.mac);
   
   // initial radio
   radio_setEndFrameCb(cb_endFrame);

   // initial UART
   uart_setCallbacks(
      isr_openserial_tx_mod,
      isr_openserial_rx_mod
   );
   serial_enable();
   
   scheduler_start();
   return 0; // this line should never be reached
}

//=========================== private =========================================

//===== serial

void serial_enable(void) {
   uart_clearTxInterrupts();
   uart_clearRxInterrupts();      // clear possible pending interrupts
   uart_enableInterrupts();       // Enable USCI_A1 TX & RX interrupt
}

void serial_disable(void) {
   uart_disableInterrupts();      // disable USCI_A1 TX & RX interrupt
}

void serial_flushtx(void) {
   
   // abort if nothing to print
   if (mercator_vars.uartbuftxfill==0) {
      return;
   }
   
   // update stats
   mercator_vars.uartNumTx++;
   
   // initialize HDLC variables
   mercator_vars.uarttxcrc             = HDLC_CRCINIT;
   mercator_vars.uartbufrdidx          = 0;
   mercator_vars.uarttxcrcAdded        = 0;
   mercator_vars.uarttxclosingSent     = 0;
   
   // start sending over UART
   uart_writeByte(HDLC_FLAG);
}

void serial_rx_all(void) {
   do {
      if (mercator_vars.uartbufrxfill<1){
         // update stats
         mercator_vars.serialNumRxWrongLength++;
         break;
      }
      
      switch(mercator_vars.uartbufrx[0]) {
         case TYPE_REQ_ST:
            serial_rx_REQ_ST();
            break;
         case TYPE_REQ_IDLE:
            serial_rx_REQ_IDLE();
            break;
         case TYPE_REQ_TX:
            serial_rx_REQ_TX();
            break;
         case TYPE_REQ_RX:
            serial_rx_REQ_RX();
            break;
         default:
            // update stats
            mercator_vars.serialNumRxUnknownRequest++;
            break;
      }
      
   } while(0);
   
   serial_enable();
}

void serial_rx_REQ_ST(void) {
   if (mercator_vars.uartbufrxfill!=sizeof(REQ_ST_ht)){
      // update stats
      mercator_vars.serialNumRxWrongLength++;
      return;
   }
   
   // trigger a RESP_ST transmission
   scheduler_push_task(serial_tx_RESP_ST,TASK_PRIO_SERIAL);
}

void serial_tx_RESP_ST(void) {
   RESP_ST_ht* resp;
   
   resp = (RESP_ST_ht*)mercator_vars.uartbuftx;
   
   resp->type                     = TYPE_RESP_ST;
   resp->status                   = mercator_vars.status;
   resp->numnotifications         = htons(mercator_vars.numnotifications);

   memcpy(resp->mac, mercator_vars.mac, 8);

   mercator_vars.uartbuftxfill    = sizeof(RESP_ST_ht);
   
   serial_flushtx();
}

void serial_rx_REQ_IDLE(void) {
   if (mercator_vars.uartbufrxfill!=sizeof(REQ_IDLE_ht)){
      // update stats
      mercator_vars.serialNumRxWrongLength++;
      return;
   }
   if (mercator_vars.status == ST_TX){
      opentimers_stop(mercator_vars.sendTimerId);
   } else if (mercator_vars.status == ST_RX){
      leds_radio_off();
   }
   radio_rfOff();
   mercator_vars.status = ST_IDLE;
}

void serial_rx_REQ_TX(void) {
   uint16_t pkctr;
   REQ_TX_ht* req;

   if (mercator_vars.uartbufrxfill!=sizeof(REQ_TX_ht)){
      // update stats
      mercator_vars.serialNumRxWrongLength++;
      return;
   }
   mercator_vars.status = ST_TX;
   mercator_vars.numnotifications = 0;

   req = (REQ_TX_ht*)mercator_vars.uartbufrx;

   mercator_vars.txpk_numpk         = 0;
   mercator_vars.txpk_len           = req->txlength;
   mercator_vars.txpk_totalnumpk    = htons(req->txnumpk);

   //prepare packet
   memcpy(mercator_vars.rfbuftx, mercator_vars.mac, 8);
   memcpy(&mercator_vars.rfbuftx[8], &req->transctr, 1);
   pkctr = htons(mercator_vars.txpk_numpk);
   memcpy(mercator_vars.rfbuftx + 9, &pkctr, 2);
   memset(mercator_vars.rfbuftx + 11, req->txfillbyte, mercator_vars.txpk_len - 11);

   // prepare radio
   radio_rfOn();
   radio_setFrequency(req->frequency);
   
   // TODO set TX Power

   // init opentimers to send packets periodically
   mercator_vars.sendTimerId  = opentimers_start(
      htons(req->txifdur),
      TIMER_PERIODIC,
      TIME_MS,
      cb_sendPacket
   );

   return;
}

void serial_rx_REQ_RX(void) {
   REQ_RX_ht* req;
   if (mercator_vars.uartbufrxfill!=sizeof(REQ_RX_ht)){
      // update stats
      mercator_vars.serialNumRxWrongLength++;
      return;
   }

   req = (REQ_RX_ht*)mercator_vars.uartbufrx;
   mercator_vars.rxpk_transctr = req->transctr;
   mercator_vars.rxpk_txfillbyte = req->txfillbyte;
   memcpy(mercator_vars.rxpk_srcmac, req->srcmac, 8);


   // reset notifications counter
   mercator_vars.numnotifications = 0;

   // turn on radio leds
   leds_radio_on();

   // prepare radio
   radio_rfOn();
   radio_setFrequency(req->frequency);

   // switch in RX
   radio_rxEnable();

   // change status to RX
   mercator_vars.status = ST_RX;

   return;
}

//===== helpers

uint16_t htons(uint16_t val) {
   return (((uint16_t)(val>>0)&0xff)<<8) | (((uint16_t)(val>>8)&0xff)<<0);
}

//===== interrupt handlers

//executed in ISR, called from scheduler.c
void isr_openserial_tx_mod(void) {
   uint8_t  b;
   uint16_t finalCrc;
   
   // led
   leds_sync_on();
   
   // write next byte, if any
   if (mercator_vars.uartbuftxfill>0) {
      b = mercator_vars.uartbuftx[mercator_vars.uartbufrdidx];
      if (mercator_vars.uarttxescape==0 && (b==HDLC_FLAG || b==HDLC_ESCAPE)) {
         uart_writeByte(HDLC_ESCAPE);
         mercator_vars.uarttxescape=1;
      } else {
         mercator_vars.uarttxcrc = crcIteration(mercator_vars.uarttxcrc,b);
         if (mercator_vars.uarttxescape==1) {
             b = b^HDLC_ESCAPE_MASK;
             mercator_vars.uarttxescape = 0;
         }
         mercator_vars.uartbufrdidx++;
         mercator_vars.uartbuftxfill--;
         uart_writeByte(b);
      }
      if (mercator_vars.uartbuftxfill==0 && mercator_vars.uarttxcrcAdded==0) {
          // finalize the calculation of the CRC
          finalCrc   = ~mercator_vars.uarttxcrc;
          
          // write the CRC value
          mercator_vars.uartbuftx[mercator_vars.uartbufrdidx]   = (finalCrc>>0)&0xff;
          mercator_vars.uartbuftx[mercator_vars.uartbufrdidx+1] = (finalCrc>>8)&0xff;
          
          mercator_vars.uartbuftxfill += 2;
          mercator_vars.uarttxcrcAdded = 1;
      }
   } else {
      if (mercator_vars.uarttxclosingSent==0){
         mercator_vars.uarttxclosingSent = 1;
         uart_writeByte(HDLC_FLAG);
      }
   }
   
   // led
   leds_sync_off();
}

void inputHdlcWriteMod(uint8_t b) {
   if (b==HDLC_ESCAPE) {
      mercator_vars.uartrxescaping = TRUE;
   } else {
      if (mercator_vars.uartrxescaping==TRUE) {
         b                             = b^HDLC_ESCAPE_MASK;
         mercator_vars.uartrxescaping = FALSE;
      }
      
      // add byte to input buffer
      mercator_vars.uartbufrx[mercator_vars.uartbufrxfill] = b;
      mercator_vars.uartbufrxfill++;
      
      // iterate through CRC calculator
      mercator_vars.uartrxcrc = crcIteration(mercator_vars.uartrxcrc,b);
   }
}

// executed in ISR, called from scheduler.c
void isr_openserial_rx_mod(void) {
   uint8_t rxbyte;
   
   // led
   leds_sync_on();
   
   // read byte just received
   rxbyte = uart_readByte();
   
   // handle byte
   if (
         mercator_vars.uartrxbusy==FALSE  &&
         mercator_vars.uartlastRxByte==HDLC_FLAG &&
         rxbyte!=HDLC_FLAG
      ) {
      // start of frame
      
      // I'm now receiving
      mercator_vars.uartrxbusy           = TRUE;
      
      // reset the input buffer index
      mercator_vars.uartbufrxfill            = 0;
      
      // initialize the value of the CRC
      mercator_vars.uartrxcrc                = HDLC_CRCINIT;
      
      // add the byte just received
      inputHdlcWriteMod(rxbyte);
   } else if (
         mercator_vars.uartrxbusy==TRUE   &&
         rxbyte!=HDLC_FLAG
      ) {
      // middle of frame
      
      // add the byte just received
      inputHdlcWriteMod(rxbyte);
      if (mercator_vars.uartbufrxfill+1>UART_BUF_LEN){
         mercator_vars.uartbufrxfill         = 0;
         mercator_vars.uartrxbusy        = FALSE;
      }
   } else if (
         mercator_vars.uartrxbusy==TRUE   &&
         rxbyte==HDLC_FLAG
      ) {
         // end of frame
         
         // verify the validity of the frame
         if (mercator_vars.uartrxcrc==HDLC_CRCGOOD) {
            // the CRC is correct
            
            // update stats
            mercator_vars.uartNumRxCrcOk++;
            
            // remove the CRC from the input buffer
            mercator_vars.uartbufrxfill    -= 2;
            
            // stop receiving (until frame handled)
            serial_disable();
            
            // schedule task to handle frame
            scheduler_push_task(serial_rx_all,TASK_PRIO_SERIAL);
            
            // wakeup the scheduler
            SCHEDULER_WAKEUP();
         } else {
            // the CRC is incorrect
            
            // update stats
            mercator_vars.uartNumRxCrcWrong++;
         }
         
         mercator_vars.uartrxbusy      = FALSE;
   }
   
   // store byte
   mercator_vars.uartlastRxByte = rxbyte;
   
   // led
   leds_sync_off();
}

//=========================== callbacks =======================================



//=========================== callbacks =======================================

//===== radiotimer

void cb_endFrame(uint16_t timestamp) {
   // local vars
      uint8_t  srcmac[8];
      uint8_t  transctr;
     uint16_t  pkctr;
      uint8_t  txfillbyte;
         bool  is_expected = TRUE;
   IND_RX_ht*  resp;
   
   if (mercator_vars.status == ST_RX){
      
      // get packet from radio
      radio_getReceivedFrame(
         mercator_vars.rxpk_buf,
         &mercator_vars.rxpk_len,
         sizeof(mercator_vars.rxpk_buf),
         &mercator_vars.rxpk_rssi,
         &mercator_vars.rxpk_lqi,
         &mercator_vars.rxpk_crc
      );

      memcpy(srcmac,       mercator_vars.rxpk_buf     , 8);
      memcpy(&transctr,    &mercator_vars.rxpk_buf[8] , 1);
      memcpy(&pkctr,       mercator_vars.rxpk_buf + 9 , 2);
      pkctr = htons(pkctr);
      memcpy(&txfillbyte,  &mercator_vars.rxpk_buf[11], 1);

      // check srcmac
      if (memcmp(srcmac, mercator_vars.rxpk_srcmac, 8) != 0){
         is_expected = FALSE;
      }

      // check transctr
      if (transctr != mercator_vars.rxpk_transctr){
         is_expected = FALSE;
      }

      // check txfillbyte
      if (txfillbyte != mercator_vars.rxpk_txfillbyte){
         is_expected = FALSE;
      }

      resp = (IND_RX_ht*)mercator_vars.uartbuftx;

      resp->type     =  TYPE_IND_RX;
      resp->length   =  mercator_vars.rxpk_len;
      resp->rssi     =  mercator_vars.rxpk_rssi;
      resp->flags    =  mercator_vars.rxpk_crc << 7 | is_expected << 6;
      resp->pkctr    =  htons(pkctr);

      mercator_vars.uartbuftxfill = sizeof(IND_RX_ht);

      serial_flushtx();

      mercator_vars.numnotifications++;
   }
}

void cb_sendPacket(void){
   IND_TXDONE_ht* resp;
   uint16_t pkctr;
   // send packet
   leds_error_on();

   radio_loadPacket(mercator_vars.rfbuftx, mercator_vars.txpk_len);
   radio_txEnable();
   radio_txNow();

   leds_error_off();

   if (mercator_vars.txpk_numpk == mercator_vars.txpk_totalnumpk) {
      opentimers_stop(mercator_vars.sendTimerId);

      // finishing TX
      radio_rfOff();
      mercator_vars.status = ST_TXDONE;

      // send IND_TXDONE
      resp = (IND_TXDONE_ht*)mercator_vars.uartbuftx;
      resp->type = TYPE_IND_TXDONE;

      mercator_vars.uartbuftxfill = sizeof(IND_TXDONE_ht);

      serial_flushtx();

      mercator_vars.numnotifications++;

      return;
   }

   // update pkctr
   mercator_vars.txpk_numpk++;
   pkctr = htons(mercator_vars.txpk_numpk);
   memcpy(mercator_vars.rfbuftx + 9, &pkctr, 2);
   return;
}
