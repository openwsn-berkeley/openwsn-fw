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

    //Modem = LoRa
    SX1276SetSleep();

    //Set frequency == 868 MHz


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

    //Spreading Factor = SF9 = 512 -- CRC enable
    spi_tx_buffer[0]     = REG_LR_MODEMCONFIG2 | (1 << 7);
    spi_tx_buffer[1]     = 0x94;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //Preamble length
    spi_tx_buffer[0]     = REG_LR_PREAMBLEMSB | (1 << 7);
    spi_tx_buffer[1]     = 0x08;
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

//void SX1276SetRxConfig( void );



/*void radio_init_sx1276(){
       
    RxChainCalibration( );

}

static void RxChainCalibration( void )
{
    uint8_t regPaConfigInitVal;
    uint32_t initialFreq;

    // Save context
    regPaConfigInitVal = SX1276Read( REG_PACONFIG );
    initialFreq = ( double )( ( ( uint32_t )SX1276Read( REG_FRFMSB ) << 16 ) |
                              ( ( uint32_t )SX1276Read( REG_FRFMID ) << 8 ) |
                              ( ( uint32_t )SX1276Read( REG_FRFLSB ) ) ) * ( double )FREQ_STEP;

    // Cut the PA just in case, RFO output, power = -1 dBm
    SX1276Write( REG_PACONFIG, 0x00 );

    // Launch Rx chain calibration for LF band
    SX1276Write( REG_IMAGECAL, ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
    while( ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
    {
    }

    // Sets a Frequency in HF band
    SX1276SetChannel( 868000000 );

    // Launch Rx chain calibration for HF band
    SX1276Write( REG_IMAGECAL, ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
    while( ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
    {
    }

    // Restore context
    SX1276Write( REG_PACONFIG, regPaConfigInitVal );
    SX1276SetChannel( initialFreq );
}*/


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
