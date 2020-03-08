/**
\brief Mercator firmware, see https://github.com/openwsn-berkeley/mercator/.

\author Constanza Perez Garcia <constanza.perezgarcia@gmail.com>, November 2014.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, November 2014.
*/

#include "03oos_mercator.h"
#include <stddef.h>

//=========================== initialization ==================================

int mote_main(void) {

   memset(&mercator_vars,0,sizeof(mercator_vars_t));
   mercator_vars.status = ST_IDLE;

   board_init();
   scheduler_init();
   opentimers_init();
   mercator_vars.sendTimerId = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_NONE);

   leds_all_off();

   // get mac
   eui64_get(mercator_vars.mac);

   // initial radio
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);

   // initial UART
   uart_setCallbacks(
      isr_openserial_tx_mod,
      isr_openserial_rx_mod
   );
   serial_enable();
   scheduler_push_task(serial_tx_IND_UP,TASK_PRIO_SERIAL);

   scheduler_start();
   return 0; // this line should never be reached
}

//=========================== private =========================================

//===== serial

void serial_enable(void) {
   uart_disableInterrupts();      // clear possible pending interrupts
   uart_enableInterrupts();       // Enable USCI_A1 TX & RX interrupt
   mercator_vars.uartlastRxByte = 0x00;
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

    if (mercator_vars.uartbufrxindex<1){
       // update stats
       mercator_vars.serialNumRxWrongLength++;
    } else {
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
    }
    serial_enable();
}

void serial_rx_REQ_ST(void) {
   if (mercator_vars.uartbufrxindex!=sizeof(REQ_ST_ht)){
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

   memcpy(resp->mac, mercator_vars.mac, MAC_LEN);

   mercator_vars.uartbuftxfill    = sizeof(RESP_ST_ht);

   serial_flushtx();
}

void serial_rx_REQ_IDLE(void) {
   if (mercator_vars.uartbufrxindex!=sizeof(REQ_IDLE_ht)){
      // update stats
      mercator_vars.serialNumRxWrongLength++;
      return;
   }
   // schedule a response
   scheduler_push_task(serial_tx_RESP_IDLE,TASK_PRIO_SERIAL);
   if (mercator_vars.status == ST_TX){
      opentimers_cancel(mercator_vars.sendTimerId);
   } else if (mercator_vars.status == ST_RX){
      leds_radio_off();
   }
   radio_rfOff();
   mercator_vars.status = ST_IDLE;
}

void serial_tx_RESP_IDLE(void) {
  RESP_IDLE_ht* resp;

  resp = (RESP_IDLE_ht*)mercator_vars.uartbuftx;

  resp->type                     = TYPE_RESP_IDLE;
  mercator_vars.uartbuftxfill    = sizeof(RESP_IDLE_ht);

  serial_flushtx();
}

void serial_rx_REQ_TX(void) {
   uint16_t transctr;
   uint16_t pkctr;
   REQ_TX_ht* req;

   if (mercator_vars.uartbufrxindex!=sizeof(REQ_TX_ht)){
      // update stats
      mercator_vars.serialNumRxWrongLength++;
      return;
   }

   // schedule a response
   scheduler_push_task(serial_tx_RESP_TX,TASK_PRIO_SERIAL);

   if (mercator_vars.status == ST_TX){
      // we've already in ST_TX; just return a response
      return;
   }

   mercator_vars.status = ST_TX;
   mercator_vars.numnotifications = 0;

   req = (REQ_TX_ht*)mercator_vars.uartbufrx;

   mercator_vars.txpk_numpk         = 0;
   mercator_vars.txpk_len           = req->txlength;
   mercator_vars.txpk_totalnumpk    = htons(req->txnumpk);

   // prepare packet
   memcpy(mercator_vars.rfbuftx, mercator_vars.mac, MAC_LEN);
   transctr = htons(req->transctr);
   memcpy(&mercator_vars.rfbuftx[offsetof(RF_PACKET_ht, transctr)],
          &transctr,
          sizeof(((REQ_TX_ht){0}).transctr)
   );
   pkctr = htons(mercator_vars.txpk_numpk);
   memcpy(&mercator_vars.rfbuftx[offsetof(RF_PACKET_ht, pkctr)],
          &pkctr, sizeof(((mercator_vars_t){0}).txpk_numpk));

   // fill remaining packet space
   memset(&mercator_vars.rfbuftx[offsetof(RF_PACKET_ht, txfillbyte)], req->txfillbyte,
          mercator_vars.txpk_len - offsetof(RF_PACKET_ht, txfillbyte) - LENGTH_CRC);

   // prepare radio
   radio_rfOn();
   // freq type only effects on scum port
   radio_setFrequency(req->frequency, FREQ_TX);

   // TODO set TX Power

   // init opentimers to send packets periodically
   opentimers_scheduleIn(
      mercator_vars.sendTimerId,
      htons(req->txifdur),
      TIME_MS,
      TIMER_PERIODIC,
      cb_sendPacket
   );

   return;
}

void serial_tx_RESP_TX(void) {
  RESP_TX_ht* resp;

  resp = (RESP_TX_ht*)mercator_vars.uartbuftx;

  resp->type                     = TYPE_RESP_TX;
  mercator_vars.uartbuftxfill    = sizeof(RESP_TX_ht);

  serial_flushtx();
}

void serial_rx_REQ_RX(void) {
   REQ_RX_ht* req;

   if (mercator_vars.uartbufrxindex!=sizeof(REQ_RX_ht)){
      // update stats
      mercator_vars.serialNumRxWrongLength++;
      return;
   }

   // schedule a response
   scheduler_push_task(serial_tx_RESP_RX,TASK_PRIO_SERIAL);

   req = (REQ_RX_ht*)mercator_vars.uartbufrx;
   // save the expected values
   mercator_vars.txpk_len = req->txlength;
   mercator_vars.txpk_transctr = htons(req->transctr);
   mercator_vars.txpk_txfillbyte = req->txfillbyte;
   memcpy(mercator_vars.txpk_srcmac, req->srcmac, 8);


   // reset notifications counter
   mercator_vars.numnotifications = 0;

   // turn on radio leds
   leds_radio_on();

   // prepare radio
   radio_rfOn();
   // freq type only effects on scum port
   radio_setFrequency(req->frequency, FREQ_RX);

   // switch in RX
   radio_rxEnable();
   radio_rxNow();

   // change status to RX
   mercator_vars.status = ST_RX;

   return;
}

void serial_tx_RESP_RX(void) {
  RESP_RX_ht* resp;

  resp = (RESP_RX_ht*)mercator_vars.uartbuftx;

  resp->type                     = TYPE_RESP_RX;
  mercator_vars.uartbuftxfill    = sizeof(RESP_RX_ht);

  serial_flushtx();
}

void serial_tx_IND_UP(void) {
   IND_UP_ht* resp;

   resp = (IND_UP_ht*)mercator_vars.uartbuftx;

   resp->type                     = TYPE_IND_UP;

   mercator_vars.uartbuftxfill    = sizeof(IND_UP_ht);

   serial_flushtx();
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
      mercator_vars.uartbufrx[mercator_vars.uartbufrxindex] = b;
      mercator_vars.uartbufrxindex++;

      // iterate through CRC calculator
      mercator_vars.uartrxcrc = crcIteration(mercator_vars.uartrxcrc,b);
   }
}

// executed in ISR, called from scheduler.c
uint8_t isr_openserial_rx_mod(void) {
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
      mercator_vars.uartbufrxindex            = 0;

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

      // reset buffer if frame too long
      if (mercator_vars.uartbufrxindex+1>UART_BUF_LEN){
         mercator_vars.uartbufrxindex         = 0;
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
            mercator_vars.uartbufrxindex    -= 2;

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

   return 0;
}

//=========================== callbacks =======================================



//=========================== callbacks =======================================

//===== radiotimer

void cb_startFrame(PORT_TIMER_WIDTH timestamp){
    // nothing
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
   // local vars
   RF_PACKET_ht* rx_temp;
   bool         is_expected = TRUE;
   IND_RX_ht*   resp;
   int i;

   radio_rfOff();

   if (mercator_vars.status == ST_RX){

      // get packet from radio
      memset(mercator_vars.rxpk_buf, 0, sizeof(mercator_vars.rxpk_buf));
      radio_getReceivedFrame(
         mercator_vars.rxpk_buf,
         &mercator_vars.rxpk_len,
         sizeof(mercator_vars.rxpk_buf),
         &mercator_vars.rxpk_rssi,
         &mercator_vars.rxpk_lqi,
         &mercator_vars.rxpk_crc
      );

      // unpack rx buffer
      rx_temp = (RF_PACKET_ht *)mercator_vars.rxpk_buf;

      // check srcmac
      if (memcmp(rx_temp->srcmac, mercator_vars.txpk_srcmac, MAC_LEN) != 0){
         is_expected = FALSE;
      }

      // check length
      if (mercator_vars.rxpk_len != mercator_vars.txpk_len){
         is_expected = FALSE;
      }

      // check transctr
      if (rx_temp->transctr != mercator_vars.txpk_transctr){
         is_expected = FALSE;
      }

      // check txfillbyte
      for (i = offsetof(RF_PACKET_ht, txfillbyte);
           i < mercator_vars.rxpk_len - LENGTH_CRC; i++){
         if(mercator_vars.rxpk_buf[i] != mercator_vars.txpk_txfillbyte){
            is_expected = FALSE;
            break;
         }
      }

      if (is_expected == TRUE && mercator_vars.rxpk_crc == TRUE){
        resp = (IND_RX_ht*)mercator_vars.uartbuftx;

        resp->type     =  TYPE_IND_RX;
        resp->length   =  mercator_vars.rxpk_len;
        resp->rssi     =  mercator_vars.rxpk_rssi;
        // resp->flags should have always the same value, 0xc0, but
        // keep this attribute for backward compatibility
        resp->flags    =  mercator_vars.rxpk_crc << 7 | is_expected << 6;
        resp->pkctr    =  rx_temp->pkctr;

        mercator_vars.uartbuftxfill = sizeof(IND_RX_ht);

        serial_flushtx();

        mercator_vars.numnotifications++;
      }

      // get ready for a following frame
      radio_rfOn();
      radio_rxEnable();
      radio_rxNow();
   }
}

void cb_sendPacket(opentimers_id_t id){
   IND_TXDONE_ht* resp;
   uint16_t pkctr;
   uint8_t pkctr_offset = 10; // srcmac[8] + transctr[2]

   if (mercator_vars.txpk_numpk == mercator_vars.txpk_totalnumpk) {
     opentimers_cancel(mercator_vars.sendTimerId);

     // finishing TX
     radio_rfOff();
     mercator_vars.status = ST_TXDONE;

     // send IND_TXDONE
     resp = (IND_TXDONE_ht*)mercator_vars.uartbuftx;
     resp->type = TYPE_IND_TXDONE;

     mercator_vars.uartbuftxfill = sizeof(IND_TXDONE_ht);

     serial_flushtx();

     mercator_vars.numnotifications++;

     // turn off the radio just in case
     radio_rfOff();
     // we keep the current status, ST_TX, which will be changed to
     // ST_IDLE by the user
     return;
   }

   // send packet
   leds_error_on();

   radio_loadPacket(mercator_vars.rfbuftx, mercator_vars.txpk_len);
   radio_txEnable();
   radio_txNow();

   leds_error_off();

   // update pkctr
   mercator_vars.txpk_numpk++;
   pkctr = htons(mercator_vars.txpk_numpk);
   memcpy(mercator_vars.rfbuftx + pkctr_offset, &pkctr, sizeof(pkctr));
   return;
}
