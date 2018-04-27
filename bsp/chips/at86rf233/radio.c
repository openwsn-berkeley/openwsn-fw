/**
* Copyright (c) 2014 Atmel Corporation. All rights reserved. 
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution.
* 
* 3. The name of Atmel may not be used to endorse or promote products derived 
* from this software without specific prior written permission.  
* 
* 4. This software may only be redistributed and used in connection with an 
* Atmel microcontroller product.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* 
* 
*/

#include "board.h"
#include "radio.h"
#include "at86rf233.h"
#include "spi.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   radiotimer_capture_cbt    startFrame_cb;
   radiotimer_capture_cbt    endFrame_cb;
   radio_state_t             state; 
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

void    radio_spiWriteReg(uint8_t reg_addr, uint8_t reg_setting);
uint8_t radio_spiReadReg(uint8_t reg_addr);
void    radio_spiWriteTxFifo(uint8_t* bufToWrite, uint8_t lenToWrite);
void    radio_spiReadRxFifo(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                            uint8_t* pLqi);
uint8_t radio_spiReadRadioInfo();

//=========================== public ==========================================

//===== admin
extern uint8_t trx_reg_read(uint8_t addr);

void radio_init(void) {
   // clear variables
   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   
   // change state
   radio_vars.state          = RADIOSTATE_STOPPED;
  
  //Check the radio part number. If the H/W interface in not initialized it statys here */
  while (trx_reg_read(RG_PART_NUM) != AT86RF233_PART_NUM);
  
   // configure the radio
   radio_spiWriteReg(RG_TRX_STATE, CMD_FORCE_TRX_OFF);    // turn radio off
  
   radio_spiWriteReg(RG_IRQ_MASK,
                     (AT_IRQ_RX_START| AT_IRQ_TRX_END));  // tell radio to fire interrupt on TRX_END and RX_START
   radio_spiReadReg(RG_IRQ_STATUS);                       // deassert the interrupt pin in case is high
   radio_spiWriteReg(RG_ANT_DIV, RADIO_CHIP_ANTENNA);     // use chip antenna
   radio_spiWriteReg(RG_TRX_CTRL_1, 0x20);                // have the radio calculate CRC
   //busy wait until radio status is TRX_OFF
  
   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != TRX_OFF);
   
   // change state
   radio_vars.state          = RADIOSTATE_RFOFF;
}

void radio_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_setOverflowCb(cb);
}

void radio_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_setCompareCb(cb);
}

void radio_setStartFrameCb(radiotimer_capture_cbt cb) {
   radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
   radio_vars.endFrame_cb    = cb;
}

//===== reset

void radio_reset(void) {
   PORT_PIN_RADIO_RESET_LOW();
}

//===== timer

void radio_startTimer(PORT_TIMER_WIDTH period) {
   radiotimer_start(period);
}

PORT_TIMER_WIDTH radio_getTimerValue(void) {
   return radiotimer_getValue();
}

void radio_setTimerPeriod(PORT_TIMER_WIDTH period) {
   radiotimer_setPeriod(period);
}

PORT_TIMER_WIDTH radio_getTimerPeriod(void) {
   return radiotimer_getPeriod();
}

//===== RF admin

void radio_setFrequency(uint8_t frequency) {
   // change state
   radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;
   
   // configure the radio to the right frequency
   radio_spiWriteReg(RG_PHY_CC_CCA,0x20+frequency);
   
   // change state
   radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn(void) {
   PORT_PIN_RADIO_RESET_LOW();
}

void radio_rfOff(void) {
   // change state
   radio_vars.state = RADIOSTATE_TURNING_OFF;
   radio_spiReadReg(RG_TRX_STATUS);
   // turn radio off
   radio_spiWriteReg(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
   //radio_spiWriteReg(RG_TRX_STATE, CMD_TRX_OFF);
   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != TRX_OFF); // busy wait until done
   
   // wiggle debug pin
   debugpins_radio_clr();
   leds_radio_off();
   
   // change state
   radio_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint16_t len) {
   // change state
   radio_vars.state = RADIOSTATE_LOADING_PACKET;
   
   // load packet in TXFIFO
   radio_spiWriteTxFifo(packet,len);
   
   // change state
   radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable(void) {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_TX;
   
   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
   
   // turn on radio's PLL
   radio_spiWriteReg(RG_TRX_STATE, CMD_PLL_ON);
   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != PLL_ON); // busy wait until done
   
   // change state
   radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow(void) {
   PORT_TIMER_WIDTH val;
   // change state
   radio_vars.state = RADIOSTATE_TRANSMITTING;
   
   // send packet by pulsing the SLP_TR_CNTL pin
   PORT_PIN_RADIO_SLP_TR_CNTL_HIGH();
   PORT_PIN_RADIO_SLP_TR_CNTL_LOW();
   
   // The AT86RF231 does not generate an interrupt when the radio transmits the
   // SFD, which messes up the MAC state machine. The danger is that, if we leave
   // this funtion like this, any radio watchdog timer will expire.
   // Instead, we cheat an mimick a start of frame event by calling
   // ieee154e_startOfFrame from here. This also means that software can never catch
   // a radio glitch by which #radio_txEnable would not be followed by a packet being
   // transmitted (I've never seen that).
   if (radio_vars.startFrame_cb!=NULL) {
      // call the callback
      val=radiotimer_getCapturedTime();
      radio_vars.startFrame_cb(val);
   }
}

//===== RX

void radio_rxEnable(void) {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_RX;
   
   // put radio in reception mode
   radio_spiWriteReg(RG_TRX_STATE, CMD_RX_ON);
   
   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
   
   // busy wait until radio really listening
   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != RX_ON);
   
   // change state
   radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow(void) {
   // nothing to do
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                            uint8_t* pCrc) {
   uint8_t temp_reg_value;
   
   //===== crc
   temp_reg_value  = radio_spiReadReg(RG_PHY_RSSI);
   *pCrc           = (temp_reg_value & 0x80)>>7;  // msb is whether packet passed CRC
   
   //===== rssi
   // as per section 8.5.3 of the AT86RF233, the RSSI is calculate as:
   // -91 + ED [dBm]
   temp_reg_value  = radio_spiReadReg(RG_PHY_ED_LEVEL);
   *pRssi          = -91 + temp_reg_value;
   
   //===== packet
   radio_spiReadRxFifo(pBufRead,
                       pLenRead,
                       maxBufLen,
                       pLqi);
}

//=========================== private =========================================


uint8_t radio_spiReadRadioInfo(void) {
   uint8_t              spi_tx_buffer[3];
   uint8_t              spi_rx_buffer[3];

   // prepare buffer to send over SPI
   spi_tx_buffer[0]     =  (0x80 | 0x1E);        // [b7]    Read/Write:    1    (read)
   // [b6]    RAM/Register : 1    (register)
   // [b5-0]  address:       0x1E (Manufacturer ID, Lower 16 Bit)
   spi_tx_buffer[1]     =  0x00;                 // send a SNOP strobe just to get the reg value
   spi_tx_buffer[2]     =  0x00;                 // send a SNOP strobe just to get the reg value

   // retrieve radio manufacturer ID over SPI
   spi_txrx(spi_tx_buffer,
         sizeof(spi_tx_buffer),
         SPI_BUFFER,
         spi_rx_buffer,
         sizeof(spi_rx_buffer),
         SPI_FIRST,
         SPI_LAST);

   return spi_rx_buffer[2];
}

void radio_spiWriteReg(uint8_t reg_addr, uint8_t reg_setting) {
   uint8_t spi_tx_buffer[2];
   uint8_t spi_rx_buffer[2];
   
   spi_tx_buffer[0] = (0xC0 | reg_addr);        // turn address in a 'reg write' address
   spi_tx_buffer[1] = reg_setting;
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_BUFFER,
            (uint8_t*)spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_LAST);
}

uint8_t radio_spiReadReg(uint8_t reg_addr) {
   uint8_t spi_tx_buffer[2];
   uint8_t spi_rx_buffer[2];
   
   spi_tx_buffer[0] = (0x80 | reg_addr);        // turn addess in a 'reg read' address
   spi_tx_buffer[1] = 0x00;                     // send a no_operation command just to get the reg value
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_BUFFER,
            (uint8_t*)spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_LAST);
   

  return spi_rx_buffer[1];
}

/** for testing purposes, remove if not needed anymore**/

void radio_spiWriteTxFifo(uint8_t* bufToWrite, uint8_t  lenToWrite) {
   uint8_t spi_tx_buffer[2];
   uint8_t spi_rx_buffer[1+1+127];               // 1B SPI address, 1B length, max. 127B data
   
   spi_tx_buffer[0] = 0x60;                      // SPI destination address for TXFIFO
   spi_tx_buffer[1] = lenToWrite;                // length byte
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_BUFFER,
            spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_NOTLAST);
   
   spi_txrx(bufToWrite,
            lenToWrite,
            SPI_BUFFER,
            spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_NOTFIRST,
            SPI_LAST);
}



void radio_spiReadRxFifo(uint8_t* pBufRead,
                         uint8_t* pLenRead,
                         uint8_t  maxBufLen,
                         uint8_t* pLqi) {
   // when reading the packet over SPI from the RX buffer, you get the following:
   // - *[1B]     dummy byte because of SPI
   // - *[1B]     length byte
   // -  [0-125B] packet (excluding CRC)
   // - *[2B]     CRC
   // - *[1B]     LQI
   uint8_t spi_tx_buffer[125];
   uint8_t spi_rx_buffer[3];
   
   spi_tx_buffer[0] = 0x20;
   
   // 2 first bytes
   spi_txrx(spi_tx_buffer,
            2,
            SPI_BUFFER,
            spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_NOTLAST);
   
   *pLenRead  = spi_rx_buffer[1];
   
   if (*pLenRead>2 && *pLenRead<=127) {
      // valid length
      
      //read packet
      spi_txrx(spi_tx_buffer,
               *pLenRead,
               SPI_BUFFER,
               pBufRead,
               125,
               SPI_NOTFIRST,
               SPI_NOTLAST);
      
      // CRC (2B) and LQI (1B)
      spi_txrx(spi_tx_buffer,
               2+1,
               SPI_BUFFER,
               spi_rx_buffer,
               3,
               SPI_NOTFIRST,
               SPI_LAST);
      
      *pLqi   = spi_rx_buffer[2];
      
   } else {
      // invalid length
      
      // read a just byte to close spi
      spi_txrx(spi_tx_buffer,
               1,
               SPI_BUFFER,
               spi_rx_buffer,
               sizeof(spi_rx_buffer),
               SPI_NOTFIRST,
               SPI_LAST);
   }
}

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

kick_scheduler_t radio_isr(void) {
   PORT_TIMER_WIDTH capturedTime;
   uint8_t  irq_status;

   // capture the time
   capturedTime = radiotimer_getCapturedTime();

   // reading IRQ_STATUS causes radio's IRQ pin to go low
   irq_status = radio_spiReadReg(RG_IRQ_STATUS);
    
   // start of frame event
   if (irq_status & AT_IRQ_RX_START) {
      // change state
      radio_vars.state = RADIOSTATE_RECEIVING;
      if (radio_vars.startFrame_cb!=NULL) {
         // call the callback
         radio_vars.startFrame_cb(capturedTime);
         // kick the OS
         return KICK_SCHEDULER;
      } else {
         while(1);
      }
   }
   // end of frame event
   if (irq_status & AT_IRQ_TRX_END) {
      // change state
      radio_vars.state = RADIOSTATE_TXRX_DONE;
      if (radio_vars.endFrame_cb!=NULL) {
         // call the callback
         radio_vars.endFrame_cb(capturedTime);
         // kick the OS
         return KICK_SCHEDULER;
      } else {
         while(1);
      }
   }
   
   return DO_NOT_KICK_SCHEDULER;
}
