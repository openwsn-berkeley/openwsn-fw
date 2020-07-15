#include "spi.h"
#include "gpio.h"
#include <headers/hw_memmap.h>
#include <math.h>
#include <string.h>
#include "sx1276Regs-LoRa.h"
#include "sx1276Regs-FSK.h"
#include "def_spitxrx.h"
#include "radio_sx1276.h"


//=========================== defines =========================================


//=========================== variables ==========================================

//=========================== public ==========================================

uint8_t  spi_tx_buffer[6];
uint8_t  spi_rx_buffer[6];

uint8_t lora_tx_buffer[2];
uint8_t lora_rx_buffer[2];

//static char lora_buffer[]="MessageToSend";


// uint8_t *lora_buffer;
// Declarer le lora buffer en externe

/*!
 * \brief Sets the radio in SLEEP mode
 */
void SX1276SetSleep( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    //spi_tx_buffer[1]     = 0x80;
    spi_tx_buffer[1]     =  RFLR_OPMODE_SLEEP;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in STANDBY mode
 */
void SX1276SetStby( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    //spi_tx_buffer[1]     = 0x81;
    spi_tx_buffer[1]     =  RFLR_OPMODE_STANDBY ;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in FSTX mode
 */
void SX1276SetFstx( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    //spi_tx_buffer[1]     = 0x82;
    spi_tx_buffer[1]     =  RFLR_OPMODE_SYNTHESIZER_TX ;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}


/*!
 * \brief Sets the radio in TX mode
 */
void SX1276SetTx( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    //spi_tx_buffer[1]     = 0x83;
    spi_tx_buffer[1]     =  RFLR_OPMODE_TRANSMITTER ;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in FSRX mode
 */
void SX1276SetFsrx( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    //spi_tx_buffer[1]     = 0x84;
    spi_tx_buffer[1]     =  RFLR_OPMODE_SYNTHESIZER_RX;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in RXCONTINUOUS mode
 */
void SX1276SetRxContinuous( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    //spi_tx_buffer[1]     = 0x85;
    spi_tx_buffer[1]     =  RFLR_OPMODE_RECEIVER ;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in RXSINGLE mode
 */
void SX1276SetRxSingle( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    //spi_tx_buffer[1]     = 0x86;
    spi_tx_buffer[1]     =  RFLR_OPMODE_RECEIVER_SINGLE;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

/*!
 * \brief Sets the radio in CAD(Channel Activity Detection) mode
 */
void SX1276SetCad( void ){
    spi_tx_buffer[0]     = REG_LR_OPMODE | (1 << 7);
    //spi_tx_buffer[1]     = 0x87;
    spi_tx_buffer[1]     =  RFLR_OPMODE_CAD ;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

void SX1276SetTxConfig(){

    //Modem = LoRa -- Set the device in SLEEP mode
    SX1276SetSleep();

    //Set frequency == 868 MHz
    SX1276SetChannel( 868000000 );

    //TX_OUTPUT_POWER max= +14dBm  --> 0
    //MaxPower for exemple = 7 --> Pmax=10.8+0.6*MaxPower= 15 dB --> 111
    //Output power :  15 - 15 --> 1111
    spi_tx_buffer[0]     = REG_LR_PACONFIG | (1 << 7);
    spi_tx_buffer[1]     = 0x7F; //0111 1111
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
    
        
    spi_tx_buffer[0]     = REG_LR_FIFOADDRPTR     & ~(1 << 7);
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
}

void exercice(void){
  
    //writing from address 17 values 1 2 3 
    spi_tx_buffer[0]     = REG_LR_FIFOADDRPTR   | (1 << 7);
    spi_tx_buffer[1]     = 17;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    spi_tx_buffer[0]     = REG_LR_FIFO    | (1 << 7);
    spi_tx_buffer[1]     = 1;
    spi_tx_buffer[2]     = 2;
    spi_tx_buffer[3]     = 3;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

        
    //writing from address 233 values 10 11 12 13 14
    spi_tx_buffer[0]     = REG_LR_FIFOADDRPTR   | (1 << 7);
    spi_tx_buffer[1]     = 233;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    spi_tx_buffer[0]     = REG_LR_FIFO    | (1 << 7);
    spi_tx_buffer[1]     = 10;  
    spi_tx_buffer[2]     = 11; 
    spi_tx_buffer[3]     = 12;    
    spi_tx_buffer[4]     = 13;
    spi_tx_buffer[5]     = 14;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
    
     
    for (int i=0; i<9; i++){
      spi_tx_buffer[0]     = REG_LR_FIFO    & ~(1 << 7);
      spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
    }
       
}

void SX1276WriteFifoBuffer(uint8_t addr, uint8_t size){

    //Initializes the payload size
    spi_tx_buffer[0]     = REG_LR_PAYLOADLENGTH   | (1 << 7);
    spi_tx_buffer[1]     = size;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    // Full buffer used for Tx
    spi_tx_buffer[0]     = REG_LR_FIFOTXBASEADDR   | (1 << 7);
    spi_tx_buffer[1]     = 0;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    spi_tx_buffer[0]     = REG_LR_FIFOADDRPTR   | (1 << 7);
    spi_tx_buffer[1]     = 0;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);


    // FIFO operations can not take place in Sleep mode
    SX1276SetStby();

    lora_tx_buffer[0]     = addr  | (1 << 7);
    //lora_tx_buffer[1]     = (uint8_t)(lora_buffer); //lora_buffer;
    lora_tx_buffer[1]  = 200;
    spi_txrx(lora_tx_buffer, sizeof(lora_tx_buffer),SPI_FIRSTBYTE,lora_rx_buffer,sizeof(lora_rx_buffer),SPI_FIRST,SPI_LAST);
}



void sx1276ReadFifoBuffer(uint8_t addr){

    // FIFO operations can not take place in Sleep mode
    SX1276SetStby();

    lora_tx_buffer[0]     = addr  | (~(1 << 7));
    spi_txrx(lora_tx_buffer, sizeof(lora_tx_buffer),SPI_FIRSTBYTE,lora_rx_buffer,sizeof(lora_rx_buffer),SPI_FIRST,SPI_LAST);

}


void  sx1276Send(void){

    //Set the radio in STDBY mode
    SX1276SetStby();

    //Tx_init
    SX1276SetTxConfig();

    //Write Data FIFO
    //SX1276WriteFifoBuffer(0);

    //TX Mode request
    SX1276SetTx();

    //IRQ TxDone Interrupt

    GPIOPinWrite(GPIO_A_BASE, GPIO_PIN_7, 0);

    while( SX1276ReadTxDone() == 0 );
       
    GPIOPinWrite(GPIO_A_BASE, GPIO_PIN_7, 1);
    
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
    GPIOPinWrite(GPIO_A_BASE, GPIO_PIN_7, 0);

    while( SX1276ReadRxDone() == 0 );
       
    GPIOPinWrite(GPIO_A_BASE, GPIO_PIN_7, 1);

    //CRC error interrupt
    
    GPIOPinWrite(GPIO_A_BASE, GPIO_PIN_7, 0);

    while( SX1276ReadCrcError() == 0 );
       
    GPIOPinWrite(GPIO_A_BASE, GPIO_PIN_7, 1);
   
    //Read DATA
    sx1276ReadFifoBuffer(0);

    //STDBY mode 
    SX1276SetStby();
}


//Txdone interrupt 
uint8_t SX1276ReadTxDone(void){

    //uint8_t value=0;
    
    spi_tx_buffer[0]     = REG_LR_IRQFLAGS  & (~(1 << 7));
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
    return (spi_rx_buffer[1] & (1<<3));


    //if !( value & (1 << 3) )  return false;
    //else return true;

}

//RxDone interrupt 
uint8_t  SX1276ReadRxDone(void){

    //uint8_t value=0;
    
    spi_tx_buffer[0]     = REG_LR_IRQFLAGS  & (~(1 << 7));
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
    return spi_rx_buffer[1] & (1<<6);

}

//CRC error interrupt 
uint8_t  SX1276ReadCrcError(void){

    //uint8_t value=0;
    
    spi_tx_buffer[0]     = REG_LR_IRQFLAGS  & (~(1 << 7));
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
    return spi_rx_buffer[1] & (1<<5);
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

//SPI address pointer in FIFO data buffer
    //SX1276Write( REG_LR_FIFOADDRPTR, 0 );
    /*spi_tx_buffer[0]     = REG_LR_FIFOADDRPTR  | (1 << 7);
    spi_tx_buffer[1]     = 0x00;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

    //Write base address in FIFO data buffer for TX modulator
    //SX1276Write( REG_LR_FIFOTXBASEADDR, 0 );
    spi_tx_buffer[0]     = REG_LR_FIFOTXBASEADDR | (1 << 7);
    spi_tx_buffer[1]     = 0x00;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);*/

/*void radio_getReceivedFrame_sx1276(
    uint8_t* bufRead,
    uint8_t* lenRead,
    uint8_t  maxBufLen,
    int8_t*  rssi,
    uint8_t* lqi,
    bool*    crc)
{*
 

}*/