/**
\brief CC1101-specific definition of the "radio" bsp module.

\author Adilla Susungi <adilla.susungi@etu.unistra.fr>, August 2013.
*/

#include "board.h"
#include "radio.h"
#include "cc1101.h"
#include "spi.h"
#include "debugpins.h"
#include "leds.h"
#include "sctimer.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   cc1101_status_t radioStatusByte;
   radio_state_t   state;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

void radio_spiStrobe     (uint8_t strobe, cc1101_status_t* statusRead);
void radio_spiWriteReg   (uint8_t reg,    cc1101_status_t* statusRead, uint8_t  regValueToWrite);
void radio_spiReadReg    (uint8_t reg,    cc1101_status_t* statusRead, uint8_t* regValueRead);
void radio_spiWriteTxFifo(                cc1101_status_t* statusRead, uint8_t* bufToWrite, uint8_t  lenToWrite);
void radio_spiReadRxFifo (                cc1101_status_t* statusRead, uint8_t* bufRead,    uint8_t* lenRead, uint8_t maxBuf);

void delay(uint16_t usec);

//=========================== public ==========================================

void delay(uint16_t usec) {
   volatile uint16_t d;
   for (d=usec;d>0;d--);
}

//==== admin

void radio_init(void) {

  // clear variables
  memset(&radio_vars, 0, sizeof(radio_vars_t));

  // change state 
  radio_vars.state            = RADIOSTATE_STOPPED;
  
  // Set SCLK = 1 and SIMO = 0
  PORT_PIN_SCLK_HIGH();
  PORT_PIN_SIMO_LOW(); 

  // Strobe CSn low/high
  PORT_PIN_CS_LOW();
  PORT_PIN_CS_HIGH();
  
  // Hold CSn low and high for 40 microsec
  PORT_PIN_CS_LOW();
  delay(40);
  PORT_PIN_CS_HIGH();
  delay(40);

  // Pull CSn low
  PORT_PIN_CS_LOW();

  // Wait for SOMI to go low
  while (radio_vars.radioStatusByte.CHIP_RDYn != 0x00);

  // reset radio
  radio_reset();

  // Wait until SOMI goes low again
  while (radio_vars.radioStatusByte.CHIP_RDYn != 0x00);  

  // change state 
  radio_vars.state            = RADIOSTATE_RFOFF;

}

void radio_setStartFrameCb(radio_capture_cbt cb) {
    sctimer_setStartFrameCb(cb);
}

void radio_setEndFrameCb(radio_capture_cbt cb) {
    sctimer_setEndFrameCb(cb);
}

//==== reset

void radio_reset(void) {
  cc1101_IOCFG0_reg_t   cc1101_IOCFG0_reg;
  cc1101_PKTCTRL0_reg_t cc1101_PKTCTRL0_reg;
  cc1101_PKTLEN_reg_t   cc1101_PKTLEN_reg;
  cc1101_MDMCFG2_reg_t  cc1101_MDMCFG2_reg;
  cc1101_MCSM0_reg_t    cc1101_MCSM0_reg;


  // global reset 
  radio_spiStrobe(CC1101_SRES, &radio_vars.radioStatusByte);

  // default setting as recommended in datasheet
  cc1101_IOCFG0_reg.GDO0_CFG           = 63;   
  cc1101_IOCFG0_reg.GDO0_INV           = 0;
  cc1101_IOCFG0_reg.TEMP_SENSOR_ENABLE = 0;

  radio_spiWriteReg(CC1101_IOCFG0,
		    &radio_vars.radioStatusByte,
		    *(uint8_t*)&cc1101_IOCFG0_reg);

 
  // setting packet control
  cc1101_PKTCTRL0_reg.LENGTH_CONFIG    = 0;  // Fixing packet length 
  cc1101_PKTCTRL0_reg.CRC_EN           = 1;  // Enabling CRC calculation
  cc1101_PKTCTRL0_reg.unused_r0_2      = 0;
  cc1101_PKTCTRL0_reg.PKT_FORMAT       = 0;
  cc1101_PKTCTRL0_reg.WHITE_DATA       = 1;  // Data whitening

  radio_spiWriteReg(CC1101_PKTCTRL0,
		    &radio_vars.radioStatusByte,
		    *(uint8_t*)&cc1101_PKTCTRL0_reg);

  // Setting packet length to 128 bytes
  cc1101_PKTLEN_reg.PACKET_LENGTH      = 128;
  
  radio_spiWriteReg(CC1101_PKTLEN,
		    &radio_vars.radioStatusByte,
		    *(uint8_t*)&cc1101_PKTLEN_reg);


  //Emulating 32 bits word as recommended in datasheet
  cc1101_MDMCFG2_reg.SYNC_MODE         = 3;   // 32 bits sync word
  cc1101_MDMCFG2_reg.MANCHESTER_EN     = 0;
  cc1101_MDMCFG2_reg.MOD_FORMAT        = 0;
  cc1101_MDMCFG2_reg.DEM_DCFILT_OFF    = 0;
  
  radio_spiWriteReg(CC1101_MDMCFG2,
		    &radio_vars.radioStatusByte,
		    *(uint8_t*)&cc1101_MDMCFG2_reg);

  // setting main control state machine 
  cc1101_MCSM0_reg.FS_AUTOCAL          = 0;
  cc1101_MCSM0_reg.PO_TIMEOUT          = 1;
  cc1101_MCSM0_reg.PIN_CTRL_EN         = 0;
  cc1101_MCSM0_reg.XOSC_FORCE_ON       = 1;   // Forcing the XOSC to stay on even in SLEEP state

  radio_spiWriteReg(CC1101_MCSM0,
		    &radio_vars.radioStatusByte,
		    *(uint8_t*)&cc1101_MCSM0_reg);
  
  
}

//==== RF admin

void radio_setFrequency(uint8_t frequency, radio_freq_t tx_or_rx) {
  cc1101_FREQ0_reg_t cc1101_FREQ0_reg;
  cc1101_FREQ1_reg_t cc1101_FREQ1_reg;
  cc1101_FREQ2_reg_t cc1101_FREQ2_reg;


  // change state
  radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;

  // setting least significant bits 
  cc1101_FREQ0_reg.FREQ   = frequency;

  radio_spiWriteReg(CC1101_FREQ0,
		    &radio_vars.radioStatusByte,
		    *(uint8_t*)&cc1101_FREQ0_reg);


  cc1101_FREQ1_reg.FREQ   = 0;

  radio_spiWriteReg(CC1101_FREQ1,
		    &radio_vars.radioStatusByte,
		    *(uint8_t*)&cc1101_FREQ1_reg);


  cc1101_FREQ2_reg.FREQ_1 = 0;            // always 0
  cc1101_FREQ2_reg.FREQ_2 = 0;  

  radio_spiWriteReg(CC1101_FREQ2,
		    &radio_vars.radioStatusByte,
		    *(uint8_t*)&cc1101_FREQ2_reg);


  // change state
  radio_vars.state = RADIOSTATE_FREQUENCY_SET;
  
  
}

void radio_rfOn(void) {
  // crystal oscillator already on
}

void radio_rfOff(void) {
  // change state
  radio_vars.state = RADIOSTATE_TURNING_OFF;

  // calibrates frequency synthesizer and turns it off
  radio_spiStrobe(CC1101_SCAL, &radio_vars.radioStatusByte);
 
  debugpins_radio_clr();
  leds_radio_off();
  
  // change state
  radio_vars.state = RADIOSTATE_RFOFF;
   
}

int8_t radio_getFrequencyOffset(void){

    // not available
    return 0;
}

//==== TX

void radio_loadPacket(uint8_t* packet, uint16_t len) {
   // change state
   radio_vars.state = RADIOSTATE_LOADING_PACKET;
   
   radio_spiStrobe(CC1101_SFTX, &radio_vars.radioStatusByte);
   radio_spiWriteTxFifo(&radio_vars.radioStatusByte, packet, len);
   
   // change state
   radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable(void) {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_TX;
   
   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
    
   // change state
   radio_vars.state = RADIOSTATE_TX_ENABLED;
}


void radio_txNow(void) {
   // change state
   radio_vars.state = RADIOSTATE_TRANSMITTING;
   
   radio_spiStrobe(CC1101_STX, &radio_vars.radioStatusByte);
}

//==== RX


void radio_rxEnable(void) {

   // change state
  radio_vars.state = RADIOSTATE_ENABLING_RX;
	    

   // put radio in reception mode
  radio_spiStrobe(CC1101_SRX, &radio_vars.radioStatusByte);
  radio_spiStrobe(CC1101_SFRX, &radio_vars.radioStatusByte);
  
   // wiggle debug pin
  debugpins_radio_set();
  leds_radio_on();
   

   // busy wait until radio really listening
  while (radio_vars.radioStatusByte.STATE == 0x01) {
    radio_spiStrobe(CC1101_SNOP, &radio_vars.radioStatusByte);
  }
   
   // change state
  radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow(void) {
  // nothing to do, the radio is already listening.
}

void radio_getReceivedFrame(uint8_t* bufRead,
                            uint8_t* lenRead,
                            uint8_t maxBufLen,
                             int8_t* rssi,
                            uint8_t* lqi,
                               bool* crc) {
   // read the received packet from the RXFIFO
   radio_spiReadRxFifo(&radio_vars.radioStatusByte, bufRead, lenRead, maxBufLen);
   
   // On reception, because of PCKTCTRL.APPEND_STATUS enabled,
   // we receive :
   // - [1B] the rssi
   // - [1B] whether CRC checked (bit 7) and LQI (bit 6-0)
   *rssi = *(bufRead+*lenRead-2);
   *crc = ((*(bufRead+*lenRead-1))&0x80)>>7;
   *lqi = (*(bufRead+*lenRead-1))&0x7f;
}

//====================== private =========================


void radio_spiStrobe(uint8_t strobe, cc1101_status_t* statusRead) {
   uint8_t  spi_tx_buffer[1];
   
   spi_tx_buffer[0]     = (CC1101_WRITE_SINGLE | strobe);
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_FIRSTBYTE,
            (uint8_t*)statusRead,
            1,
            SPI_FIRST,
            SPI_LAST);
}


void radio_spiWriteReg(uint8_t reg, cc1101_status_t* statusRead, uint8_t regValueToWrite) {
   uint8_t              spi_tx_buffer[3];
   
   spi_tx_buffer[0]     = (CC1101_WRITE_SINGLE | reg); 
   spi_tx_buffer[1]     = regValueToWrite/256;
   spi_tx_buffer[2]     = regValueToWrite%256;
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_FIRSTBYTE,
            (uint8_t*)statusRead,
            1,
            SPI_FIRST,
            SPI_LAST);
}

void radio_spiReadReg(uint8_t reg, cc1101_status_t* statusRead, uint8_t* regValueRead) {
   uint8_t              spi_tx_buffer[3];
   uint8_t              spi_rx_buffer[3];
   
   spi_tx_buffer[0]     = (CC1101_READ_SINGLE | reg);
   spi_tx_buffer[1]     = 0x00;
   spi_tx_buffer[2]     = 0x00;
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_BUFFER,
            spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_LAST);
   
   *statusRead          = *(cc1101_status_t*)&spi_rx_buffer[0];
   *(regValueRead+0)    = spi_rx_buffer[2];
   *(regValueRead+1)    = spi_rx_buffer[1];
}


void radio_spiWriteTxFifo(cc1101_status_t* statusRead, uint8_t* bufToWrite, uint8_t len) {
   uint8_t              spi_tx_buffer[2];
   
   // step 1. send SPI address and length byte
   spi_tx_buffer[0]     = (CC1101_TX_BURST);
   spi_tx_buffer[1]     = len;
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_FIRSTBYTE,
            (uint8_t*)statusRead,
            1,
            SPI_FIRST,
            SPI_NOTLAST);
   
   // step 2. send payload
   spi_txrx(bufToWrite,
            len,
            SPI_LASTBYTE,
            (uint8_t*)statusRead,
            1,
            SPI_NOTFIRST,
            SPI_LAST);
}



void radio_spiReadRxFifo(cc1101_status_t* statusRead,
                         uint8_t*         pBufRead,
                         uint8_t*         pLenRead,
                         uint8_t          maxBufLen) {
   // when reading the packet over SPI from the RX buffer, you get the following:
   // - *[1B]     dummy byte because of SPI
   // - *[1B]     length byte
   // -  [0-125B] packet (excluding CRC)
   // - *[2B]     RSSI, CRC and LQI
   uint8_t spi_tx_buffer[125];
   uint8_t spi_rx_buffer[3];
   
   spi_tx_buffer[0]     = (CC1101_RX_BURST);
   
   // 2 first bytes
   spi_txrx(spi_tx_buffer,
            2,
            SPI_BUFFER,
            spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_NOTLAST);
   
   *statusRead          = *(cc1101_status_t*)&spi_rx_buffer[0];
   *pLenRead            = spi_rx_buffer[1];
   
   if (*pLenRead>2 && *pLenRead<=127) {
      // valid length
      
      //read packet
      spi_txrx(spi_tx_buffer,
               *pLenRead,
               SPI_BUFFER,
               pBufRead,
               125,
               SPI_NOTFIRST,
               SPI_LAST);

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

//====================== callbacks =======================
