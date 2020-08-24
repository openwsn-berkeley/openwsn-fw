//#include "spi.h"
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

uint8_t spi_tx_buffer[2];
uint8_t spi_rx_buffer[2];
/*!
 * \brief Sets the radio in SLEEP mode
 */
void SX1276SetSleep( void ){

    sx1276_spiWriteReg(REG_LR_OPMODE, RFLR_OPMODE_SLEEP);
}

/*!
 * \brief Sets the radio in STANDBY mode
 */
void SX1276SetStby( void ){
 
    sx1276_spiWriteReg(REG_LR_OPMODE, RFLR_OPMODE_STANDBY);
}

/*!
 * \brief Sets the radio in FSTX mode
 */
void SX1276SetFstx( void ){

    sx1276_spiWriteReg(REG_LR_OPMODE, RFLR_OPMODE_SYNTHESIZER_TX);
}


/*!
 * \brief Sets the radio in TX mode
 */
void SX1276SetTx( void ){
 
    sx1276_spiWriteReg(REG_LR_OPMODE, RFLR_OPMODE_TRANSMITTER);
}


/*!
 * \brief Sets the radio in FSRX mode
 */
void SX1276SetFsrx( void ){

    sx1276_spiWriteReg(REG_LR_OPMODE, RFLR_OPMODE_SYNTHESIZER_RX);
}


/*!
 * \brief Sets the radio in RXCONTINUOUS mode
 */
void SX1276SetRxContinuous( void ){

    sx1276_spiWriteReg(REG_LR_OPMODE, RFLR_OPMODE_RECEIVER);
}


/*!
 * \brief Sets the radio in RXSINGLE mode
 */
void SX1276SetRxSingle( void ){

    sx1276_spiWriteReg(REG_LR_OPMODE, RFLR_OPMODE_RECEIVER_SINGLE);
}


/*!
 * \brief Sets the radio in CAD(Channel Activity Detection) mode
 */
void SX1276SetCad( void ){

    sx1276_spiWriteReg(REG_LR_OPMODE, RFLR_OPMODE_CAD);
}


void SX1276SetTxConfig(){

    //Modem = LoRa -- Set the device in SLEEP mode
    SX1276SetSleep();

    //Set frequency == 868 MHz
    SX1276SetChannel( 868000000 );

    //TX_OUTPUT_POWER max= +14dBm  --> 0
    //MaxPower for exemple = 7 --> Pmax=10.8+0.6*MaxPower= 15 dB --> 111
    //Output power :  15 - 15 --> 1111
    sx1276_spiWriteReg(REG_LR_PACONFIG, 0x7F);

    //LORA_BANDWIDTH = 125 KHZ & CodingRate = 4/5
    sx1276_spiWriteReg(REG_LR_MODEMCONFIG1, 0x72);

    //Spreading Factor = SF9 = 512 -- CRC disable
    // 7-4 : SF
    // 3 : 0 normal mode / 1: continous mode
    // 2 : 0 CRC disable / 1: CRC enable
    // 1-0 : RxTimeout
    sx1276_spiWriteReg(REG_LR_MODEMCONFIG2, 0x90);

    //Preamble Length LSB = 8 + MSB length = 0
    sx1276_spiWriteReg(REG_LR_PREAMBLELSB, 0x08);
    sx1276_spiWriteReg(REG_LR_PREAMBLEMSB, 0x00);

    //HopPeriod
    sx1276_spiWriteReg(REG_LR_HOPPERIOD, 0x00);

    //iqInverted
    sx1276_spiWriteReg(REG_LR_INVERTIQ, 0x00);
    
}

void SX1276SetRxConfig( void ){

    //Modem = LoRa -- Set the device in SLEEP mode
    SX1276SetSleep();

    //LORA_BANDWIDTH = 125 KHZ & CodingRate = 4/5
    sx1276_spiWriteReg(REG_LR_MODEMCONFIG1, 0x72);

    //Spreading Factor = SF9 = 512 -- CRC disable
    // 7-4 : SF
    // 3 : 0 normal mode / 1: continous mode
    // 2 : 0 CRC disable / 1: CRC enable
    // 1-0 : RxTimeout MSB    
    sx1276_spiWriteReg(REG_LR_MODEMCONFIG2, 0x90);

    //Preamble Length LSB = 8 + MSB length = 0
    sx1276_spiWriteReg(REG_LR_PREAMBLELSB, 0x08);
    sx1276_spiWriteReg(REG_LR_PREAMBLEMSB, 0x00);

    //HopPeriod
    sx1276_spiWriteReg(REG_LR_HOPPERIOD, 0x00);

    //iqInverted
    sx1276_spiWriteReg(REG_LR_INVERTIQ, 0x00);

    //symbTimeout  RxTimeout LSB
    sx1276_spiWriteReg(REG_LR_SYMBTIMEOUTLSB, 0x05);

}


void SX1276SetChannel( uint32_t freq )
{
    //SX1276.Settings.Channel = freq;
    freq = ( uint32_t )( ( double )freq / ( double )FREQ_STEP );

    //SX1276Write( REG_FRFMSB, ( uint8_t )( ( freq >> 16 ) & 0xFF ) );
    sx1276_spiWriteReg(REG_LR_FRFMSB, ( uint8_t )( ( freq >> 16 ) & 0xFF ));

    //SX1276Write( REG_FRFMID, ( uint8_t )( ( freq >> 8 ) & 0xFF ) );
    sx1276_spiWriteReg(REG_LR_FRFMID , ( uint8_t )( ( freq >> 8 ) & 0xFF ));

    //SX1276Write( REG_FRFLSB, ( uint8_t )( freq & 0xFF ) );
    sx1276_spiWriteReg(REG_LR_FRFLSB , ( uint8_t )( freq & 0xFF ));

}


void exercice(void){


    uint8_t spi_tx_buffer[6];
    uint8_t spi_rx_buffer[6];
  
    //writing from address 17 values 1 2 3 
    sx1276_spiWriteReg(REG_LR_FIFOADDRPTR , 17);
 
    spi_tx_buffer[0]     = REG_LR_FIFO    | (1 << 7);
    spi_tx_buffer[1]     = 1;
    spi_tx_buffer[2]     = 2;
    spi_tx_buffer[3]     = 3;
    spi_txrx(spi_tx_buffer, 4, SPI_FIRSTBYTE,spi_rx_buffer, 4, SPI_FIRST, SPI_LAST);

    //writing from address 233 values 10 11 12 13 14
    sx1276_spiWriteReg(REG_LR_FIFOADDRPTR, 233);

    spi_tx_buffer[0]     = REG_LR_FIFO    | (1 << 7);
    spi_tx_buffer[1]     = 10;  
    spi_tx_buffer[2]     = 11; 
    spi_tx_buffer[3]     = 12;    
    spi_tx_buffer[4]     = 13;
    spi_tx_buffer[5]     = 14;
    spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);

 
    //Reading multiple bytes
    sx1276_spiWriteReg(REG_LR_FIFOADDRPTR , 17);
    for(int i=0;i<3;i++){
        sx1276_spiReadReg(REG_LR_FIFO);
    }
    

    sx1276_spiWriteReg(REG_LR_FIFOADDRPTR , 233);
    for(int i=0;i<5;i++){
        sx1276_spiReadReg(REG_LR_FIFO);
    }
          
}


void sx1276_spiWriteReg(uint8_t reg, uint8_t regValueToWrite) {

    reg = reg | (1 << 7);
    spi_tx_buffer[0]     = reg ;
    spi_tx_buffer[1]     = regValueToWrite;


    spi_txrx(
        spi_tx_buffer,              // bufTx
        sizeof(spi_tx_buffer),      // lenbufTx
        SPI_FIRSTBYTE,              // returnType
        spi_rx_buffer,              // bufRx
        sizeof(spi_rx_buffer),      // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
}


uint8_t sx1276_spiReadReg(uint8_t reg) {

    reg = reg & (~(1 << 7));

    spi_tx_buffer[0]  = reg;

    spi_txrx(
        spi_tx_buffer,              // bufTx
        sizeof(spi_tx_buffer),      // lenbufTx
        SPI_BUFFER,                 // returnType
        spi_rx_buffer,              // bufRx
        sizeof(spi_rx_buffer),      // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
    return spi_rx_buffer[1];
}

void SX1276WriteFifo(uint8_t size){

    //Initializes the payload size
    sx1276_spiWriteReg(REG_LR_PAYLOADLENGTH , size);

    // Full buffer used for Tx
    sx1276_spiWriteReg(REG_LR_FIFOTXBASEADDR  , 0);
    sx1276_spiWriteReg(REG_LR_FIFOADDRPTR  , 0);

    // FIFO operations can not take place in Sleep mode
    if( sx1276_spiReadReg(REG_LR_OPMODE)  == RFLR_OPMODE_SLEEP ) SX1276SetStby();
    
    //Write Paylaod buffer
    SX1276WriteFifoBuffer((uint8_t)("MessageToSend"));

}


void SX1276WriteFifoBuffer(uint8_t buffer){

    sx1276_spiWriteReg(REG_LR_FIFO, buffer);

}


uint8_t sx1276ReadFifoBuffer(){

    uint8_t buffer=0;

    buffer = sx1276_spiReadReg(REG_LR_FIFO);

    return buffer;

}


void  sx1276Send(void){

    //Set the radio in STDBY mode
    SX1276SetStby();

    //Tx_init
    SX1276SetTxConfig();

    //Write Data FIFO
    SX1276WriteFifo(1);

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
    sx1276ReadFifoBuffer();

    //STDBY mode 
    SX1276SetStby();
}


//Txdone interrupt 
uint8_t SX1276ReadTxDone(void){

    uint8_t value=0;

    value = sx1276_spiReadReg(REG_LR_IRQFLAGS);

    return value & (1<<3);
}

//RxDone interrupt 
uint8_t  SX1276ReadRxDone(void){

    uint8_t value=0;
    
    value = sx1276_spiReadReg(REG_LR_IRQFLAGS);

    return value & (1<<6);
}

//CRC error interrupt 
uint8_t  SX1276ReadCrcError(void){

    uint8_t value=0;

    value = sx1276_spiReadReg(REG_LR_IRQFLAGS);

    return value & (1<<5);
}



