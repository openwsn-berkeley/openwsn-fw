/**
\brief TelosB-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "string.h"
#include "radio.h"
#include "at86rf231.h"
#include "spi.h"
#include "radiotimer.h"

//=========================== defines =========================================

enum radio_antennaselection_enum {
   RADIO_UFL_ANTENNA              = 0x06,   /**< Use the antenna connected by U.FL. */
   RADIO_CHIP_ANTENNA             = 0x05,   /**< Use the on-board chip antenna. */
};

//=========================== variables =======================================

typedef struct {
   uint8_t         spiActive;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

void    radio_spiWriteReg(uint8_t reg_addr, uint8_t reg_setting);
uint8_t radio_spiReadReg(uint8_t reg_addr);
void    radio_spiWriteTxFifo(uint8_t* bufToWrite, uint8_t lenToWrite);
void    radio_spiReadRxFifo(uint8_t* bufRead, uint8_t length);
void    spiCallback(void);

//=========================== public ==========================================

void radio_init() {
   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   
   // tell spi driver to call me when done
   spi_setCallback(&spiCallback);
   
   // initialize communication between MSP430 and radio
   //-- RF_SLP_TR_CNTL (P4.7) pin (output)
   P4OUT  &= ~0x80;                              // set low
   P4DIR  |=  0x80;                              // configure as output
   //-- IRQ_RF (P1.6) pin (input)
   P1OUT  &= ~0x40;                              // set low
   P1DIR  &= ~0x40;                              // input direction
   P1IES  &= ~0x40;                              // interrup when transition is low-to-high
   P1IE   |=  0x40;                              // enable interrupt
   
   // configure the radio
   radio_spiWriteReg(RG_TRX_STATE, CMD_FORCE_TRX_OFF);    // turn radio off
   radio_spiWriteReg(RG_IRQ_MASK, 0x0C);                  // tell radio to fire interrupt on TRX_END and RX_START
   radio_spiReadReg(RG_IRQ_STATUS);                       // deassert the interrupt pin (P1.6) in case is high
   radio_spiWriteReg(RG_ANT_DIV, RADIO_CHIP_ANTENNA);     // use chip antenna
#define RG_TRX_CTRL_1 0x04
   radio_spiWriteReg(RG_TRX_CTRL_1, 0x20);                // have the radio calculate CRC   

   //busy wait until radio status is TRX_OFF
   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != TRX_OFF);
   
   //radiotimer_start(0xffff);//poipoi
}

void radio_startTimer(uint16_t period) {
   radiotimer_start(period);
}

void radio_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_setOverflowCb(cb);
}

void radio_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_setCompareCb(cb);
}

void radio_setStartFrameCb(radiotimer_capture_cbt cb) {
   radiotimer_setStartFrameCb(cb);
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
   radiotimer_setEndFrameCb(cb);
}

void radio_reset() {
   //poipoi
}

void radio_setFrequency(uint8_t frequency) {
   // configure the radio to the right frequecy
   radio_spiWriteReg(RG_PHY_CC_CCA,0x20+frequency);
}

void radio_rfOn() {
   //poipoi
}

void radio_loadPacket(uint8_t* packet, uint8_t len) {
   // load packet in TXFIFO
   radio_spiWriteTxFifo(packet,len);
}

void radio_txEnable() {
   // turn on radio's PLL
   radio_spiWriteReg(RG_TRX_STATE, CMD_PLL_ON);
   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != PLL_ON); // busy wait until done
}

void radio_txNow() {
   // send packet by pulsing the RF_SLP_TR_CNTL pin
   P4OUT |=  0x80;
   P4OUT &= ~0x80;
   
   // The AT86RF231 does not generate an interrupt when the radio transmits the
   // SFD, which messes up the MAC state machine. The danger is that, if we leave
   // this funtion like this, any radio watchdog timer will expire.
   // Instead, we cheat an mimick a start of frame event by calling
   // ieee154e_startOfFrame from here. This also means that software can never catch
   // a radio glitch by which #radio_txEnable would not be followed by a packet being
   // transmitted (I've never seen that).
   //poipoiieee154e_startOfFrame(ieee154etimer_getCapturedTime());
}

void radio_rxEnable() {
   // put radio in reception mode
   radio_spiWriteReg(RG_TRX_STATE, CMD_RX_ON);
   //busy wait until radio status is PLL_ON
   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != RX_ON);
}

void radio_rxNow() {
   // nothing to do
}

void radio_getReceivedFrame(uint8_t* bufRead, uint8_t* lenRead, uint8_t maxBufLen) {
   uint8_t len;
   
   /*
   uint8_t temp_reg_value;
   // read whether CRC was is correct
   temp_reg_value = radio_spiReadReg(RG_PHY_RSSI);
   crc            = (temp_reg_value & 0x80)>>7;  // msb is whether packet passed CRC
   
   // read the RSSI
   // according to section 8.4.3 of the AT86RF231, the RSSI is calculate as:
   // -91 + ED [dBm]
   temp_reg_value = radio_spiReadReg(RG_PHY_ED_LEVEL);
   rssi           = -91 + temp_reg_value;
   */
   // copy packet from rx buffer in radio over SPI
   radio_spiReadRxFifo(bufRead,2); // first read only 2 bytes to receive the length
   len = bufRead[1];
   if (len>2 && len<=127) {
      // retrieve the whole packet (including 1B SPI address, 1B length, the packet, 1B LQI)
      radio_spiReadRxFifo(bufRead,1+1+len+1);
      // shift start by 2B (1B answer received when MSP sent SPI address + 1B length).
      //writeToBuffer->payload += 2;
      // read 1B "footer" (LQI) and store that information
      //lqi = writeToBuffer->payload[writeToBuffer->length];
      // toss CRC (2 last bytes)
      //packetfunctions_tossFooter(writeToBuffer, 2);
   }
}

void radio_rfOff() {
   // turn radio off
   radio_spiWriteReg(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != TRX_OFF); // busy wait until done
}

//=========================== private =========================================

void radio_spiWriteReg(uint8_t reg_addr, uint8_t reg_setting) {
   uint8_t spi_tx_buffer[2];
   uint8_t spi_rx_buffer[2];
   
   spi_tx_buffer[0] = (0xC0 | reg_addr);        // turn addess in a 'reg write' address
   spi_tx_buffer[1] = reg_setting;
   
   radio_vars.spiActive = 1;
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_BUFFER,
            (uint8_t*)spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_LAST);
   while (radio_vars.spiActive==1);
}

uint8_t radio_spiReadReg(uint8_t reg_addr) {
   uint8_t spi_tx_buffer[2];
   uint8_t spi_rx_buffer[2];
   
   spi_tx_buffer[0] = (0x80 | reg_addr);        // turn addess in a 'reg read' address
   spi_tx_buffer[1] = 0x00;                     // send a no_operation command just to get the reg value
   
   radio_vars.spiActive = 1;
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_BUFFER,
            (uint8_t*)spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_LAST);
   while (radio_vars.spiActive==1);
   
   return spi_rx_buffer[1];
}

void radio_spiWriteTxFifo(uint8_t* bufToWrite, uint8_t  lenToWrite) {
   // poipoi add SPI byte + len
   /*
   // add 1B length at the beginning (PHY header)
   packetfunctions_reserveHeaderSize(packet,1);
   packet->payload[0] = packet->length-1;   // length (not counting length field)
   
   // add 1B SPI address at the beginning (internally for SPI)
   packetfunctions_reserveHeaderSize(packet,1);
   packet->payload[0] = 0x60;
   */
   uint8_t spi_rx_buffer[1+1+127];              // 1B SPI address, 1B length, max. 127B data
   
   radio_vars.spiActive = 1;
   spi_txrx(bufToWrite,
            lenToWrite,
            SPI_BUFFER,
            spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_LAST);
   while (radio_vars.spiActive==1);
}

void radio_spiReadRxFifo(uint8_t* bufRead, uint8_t length) {
   uint8_t spi_tx_buffer[1+1+127];              // 1B SPI address, 1B length, 127B data
   spi_tx_buffer[0] = 0x20;                     // spi address for 'read frame buffer'
   
   radio_vars.spiActive = 1;
   spi_txrx(spi_tx_buffer,
            length,
            SPI_BUFFER,
            bufRead,
            length,
            SPI_FIRST,
            SPI_LAST);
   while (radio_vars.spiActive==1);
}

//=========================== callbacks =======================================

void spiCallback(void)
{
   radio_vars.spiActive = 0;
}