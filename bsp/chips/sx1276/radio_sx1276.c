#include "spi.h"
#include <math.h>
#include <string.h>
#include "sx1276Regs-LoRa.h"
#include "def_spitxrx.h"
#include "radio_sx1276.h"


//=========================== defines =========================================


//=========================== variables ==========================================

//=========================== public ==========================================

uint8_t  spi_tx_buffer[2];
uint8_t  spi_rx_buffer[2];


/*!
 * \brief Sets the radio in SLEEP mode
 */
void SX1276SetSleep( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    spi_tx_buffer[1]     = 0x80;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in STANDBY mode
 */
void SX1276SetStby( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    spi_tx_buffer[1]     = 0x81;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in FSTX mode
 */
void SX1276SetFstx( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    spi_tx_buffer[1]     = 0x82;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}


/*!
 * \brief Sets the radio in TX mode
 */
void SX1276SetTx( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    spi_tx_buffer[1]     = 0x83;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in FSRX mode
 */
void SX1276SetFsrx( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    spi_tx_buffer[1]     = 0x84;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in RXCONTINUOUS mode
 */
void SX1276SetRxContinuous( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    spi_tx_buffer[1]     = 0x85;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in RXSINGLE mode
 */
void SX1276SetRxSingle( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    spi_tx_buffer[1]     = 0x86;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in CAD(Channel Activity Detection) mode
 */
void SX1276SetCad( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    spi_tx_buffer[1]     = 0x87;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

void SX1276SetTxConfig(){

    //Modem = LoRa -- Set the device in SLEEP mode
    SX1276SetSleep();

    //Set frequency == 868 MHz
    SX1276SetChannel( 868000000 );

    //TX_OUTPUT_POWER max= +14dBm
    //MaxPower for exemple = 4 --> Pmax=10.8+0.6*MaxPower= 13.2 dB
    //Output power : 
    spi_tx_buffer[0]     = REG_LR_PACONFIG | (1 << 7);
    spi_tx_buffer[1]     = 0x00; //0b0100xxxx
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //LORA_BANDWIDTH = 125 KHZ & CodingRate = 4/5
    spi_tx_buffer[0]     = REG_LR_MODEMCONFIG1 | (1 << 7);
    spi_tx_buffer[1]     = 0x72;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //Spreading Factor = SF9 = 512 -- CRC disable
    // 7-4 : SF
    // 3 : 0 normal mode / 1: continous mode
    // 2 : 0 CRC disable / 1: CRC enable
    // 1-0 : RxTimeout

    spi_tx_buffer[0]     = REG_LR_MODEMCONFIG2 | (1 << 7);
    spi_tx_buffer[1]     = 0x90;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //Preamble Length LSB = 8 + MSB length = 0

    spi_tx_buffer[0]     = REG_LR_PREAMBLELSB | (1 << 7);
    spi_tx_buffer[1]     = 0x08;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    spi_tx_buffer[0]     = REG_LR_PREAMBLEMSB | (1 << 7);
    spi_tx_buffer[1]     = 0x00;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //HopPeriod
    spi_tx_buffer[0]     = REG_LR_HOPPERIOD | (1 << 7);
    spi_tx_buffer[1]     = 0x00;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //iqInverted
    spi_tx_buffer[0]     = REG_LR_INVERTIQ  | (1 << 7);
    spi_tx_buffer[1]     = 0x00;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
    
    //Transmission timeout
    //FreqHopOn
    //fixLen 
}

void SX1276SetRxConfig( void ){

    //Modem = LoRa -- Set the device in SLEEP mode
    SX1276SetSleep();

    //LORA_BANDWIDTH = 125 KHZ & CodingRate = 4/5
    spi_tx_buffer[0]     = REG_LR_MODEMCONFIG1 | (1 << 7);
    spi_tx_buffer[1]     = 0x72;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //Spreading Factor = SF9 = 512 -- CRC disable
    // 7-4 : SF
    // 3 : 0 normal mode / 1: continous mode
    // 2 : 0 CRC disable / 1: CRC enable
    // 1-0 : RxTimeout MSB 

    spi_tx_buffer[0]     = REG_LR_MODEMCONFIG2 | (1 << 7);
    spi_tx_buffer[1]     = 0x90;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //Preamble Length LSB = 8 + MSB length = 0

    spi_tx_buffer[0]     = REG_LR_PREAMBLELSB | (1 << 7);
    spi_tx_buffer[1]     = 0x08;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    spi_tx_buffer[0]     = REG_LR_PREAMBLEMSB | (1 << 7);
    spi_tx_buffer[1]     = 0x00;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //HopPeriod
    spi_tx_buffer[0]     = REG_LR_HOPPERIOD | (1 << 7);
    spi_tx_buffer[1]     = 0x00;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //iqInverted
    spi_tx_buffer[0]     = REG_LR_INVERTIQ  | (1 << 7);
    spi_tx_buffer[1]     = 0x00;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //symbTimeout  RxTimeout LSB
    spi_tx_buffer[0]     = REG_LR_SYMBTIMEOUTLSB   | (1 << 7);
    spi_tx_buffer[1]     = 0x05;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

}


void SX1276SetChannel( uint32_t freq )
{
    //SX1276.Settings.Channel = freq;
    freq = ( uint32_t )( ( double )freq / ( double )FREQ_STEP );

    //SX1276Write( REG_FRFMSB, ( uint8_t )( ( freq >> 16 ) & 0xFF ) );
    spi_tx_buffer[0]     = REG_LR_FRFMSB  | (1 << 7);
    spi_tx_buffer[1]     = ( uint8_t )( ( freq >> 16 ) & 0xFF );
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //SX1276Write( REG_FRFMID, ( uint8_t )( ( freq >> 8 ) & 0xFF ) );
    spi_tx_buffer[0]     = REG_LR_FRFMID | (1 << 7);
    spi_tx_buffer[1]     = ( uint8_t )( ( freq >> 8 ) & 0xFF );
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //SX1276Write( REG_FRFLSB, ( uint8_t )( freq & 0xFF ) );
    spi_tx_buffer[0]     = REG_LR_FRFLSB | (1 << 7);
    spi_tx_buffer[1]     = ( uint8_t )( freq & 0xFF );
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

}


 void sx1276WriteFifo(void){

    // FIFO operations can not take place in Sleep mode
    SX1276SetStby();

    //SPI address pointer in FIFO data buffer
    //SX1276Write( REG_LR_FIFOADDRPTR, 0 );
    spi_tx_buffer[0]     = REG_LR_FIFOADDRPTR  | (1 << 7);
    spi_tx_buffer[1]     = 0x05;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //Write base address in FIFO data buffer for TX modulator
    //SX1276Write( REG_LR_FIFOTXBASEADDR, 0 );
    spi_tx_buffer[0]     = REG_LR_FIFOTXBASEADDR | (1 << 7);
    spi_tx_buffer[1]     = 0x05;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

 }

void sx1276ReadFifo(void){

    // FIFO operations can not take place in Sleep mode
    SX1276SetStby();

    // Read base address in FIFO data buffer for RX demodulator
    spi_tx_buffer[0]     = REG_LR_FIFORXBASEADDR  & (~(1 << 7));
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

}

/*void radio_getReceivedFrame_sx1276(
    uint8_t* bufRead,
    uint8_t* lenRead,
    uint8_t  maxBufLen,
    int8_t*  rssi,
    uint8_t* lqi,
    bool*    crc)
{*


}*/

void  sx1276Send(void){

    //Set the radio in STDBY mode
    SX1276SetStby();

    //Tx_init
    SX1276SetTxConfig();

    //Write Data FIFO
    sx1276WriteFifo();

    //TX Mode request
    SX1276SetTx();

    //IRQ TxDone Interrupt

    //STDBY mode 
    SX1276SetStby();
}

void sx1276Receive(void){

    //Set the radio in STDBY mode
    SX1276SetStby();

    //Rx_init
    SX1276SetRxConfig();

    //RXsingle mode
    SX1276SetRxSingle();

    //RxDone Interrupt


    //CRC error interrupt


    //Read DATA
    sx1276ReadFifo();

    //STDBY mode 
    SX1276SetStby();

}

/*void radio_setStartFrameCb_sx1276() {
    // will take care of it later
}

void radio_setEndFrameCb_sx1276() {
    // will take care of it later
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
