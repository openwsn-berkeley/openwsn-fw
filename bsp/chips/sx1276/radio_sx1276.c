#include "spi.h"
#include <math.h>
#include <string.h>
#include "sx1276Regs-LoRa.h"
#include "def_spitxrx.h"



//=========================== defines =========================================


//=========================== variables ==========================================

//=========================== public ==========================================

//static void radio_read_isr_sx1276(void);
//static void radio_clear_isr_sx1276(void);

uint8_t  spi_tx_buffer[2];
uint8_t  spi_rx_buffer[2];

void radio_init_sx1276(void) {
    
//SX1276Init ()
}

void radio_rfOn_sx1276(void){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    spi_tx_buffer[1]     = 0x81;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

void radio_rfOff_sx1276(void){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    spi_tx_buffer[1]     = 0x80;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*void radio_setConfigTx_sx1276(){
// put any static configuration here
    //SX1276SetTxConfig ()
}
void radio_setConfigRx_sx1276(){
// put any static configuration here
    //SX1276SetRxConfig ()
}


void radio_setStartFrameCb_sx1276() {
    // will take care of it later
}

void radio_setEndFrameCb_sx1276() {
    // will take care of it later
}

void radio_setFrequency_sx1276() {
    // set the frequency
    //SX1276SetChannel()
}


void radio_loadPacket_sx1276(uint8_t* packet, uint16_t len) {
    // load the packet into the buffer
    //SX1276Send( buffer,  size )
}

void radio_txEnable_sx1276(void) {
    // forget about this now
}


void radio_txNow_sx1276(void) {
  //SX1276SetTx( txTimeout );
  
}



//===== RX

void radio_rxEnable_sx1276(void) {
    
  
}

void radio_rxNow_sx1276(void){
    // ignore that now
   // SX1276SetRx()
}

void radio_getReceivedFrame_sx1276(
    uint8_t* bufRead,
    uint8_t* lenRead,
    uint8_t  maxBufLen,
    int8_t*  rssi,
    uint8_t* lqi,
    bool*    crc
) */

//=========================== private =========================================


/*void radio_read_isr_sx1276(void){
    // ignore this now
}


//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

// Wrapper function for radio_isr_sx1276 to be called by the radio interrupt
void radio_isr_internal_sx1276(void){
    
//SX1276OnDio0Irq,  SX1276OnDio1Irq?? 

    
}
void kick_scheduler_t_radio_isr_sx1276(void){

  //ignore this now
}*/
