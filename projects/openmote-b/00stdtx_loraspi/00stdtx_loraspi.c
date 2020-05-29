/**
\brief definition of the "spi" bsp module.
\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, September 2017.
*/
#include "board.h"
#include "board_info.h"
#include "ssi.h"
#include "spi.h"
#include "gpio.h"
#include "ioc.h"
#include "sys_ctrl.h"

#include <headers/hw_ints.h>
#include <headers/hw_ioc.h>
#include <headers/hw_memmap.h>
#include <headers/hw_ssi.h>
#include <headers/hw_sys_ctrl.h>
#include <headers/hw_types.h>

//================== Semtech includes ====================

#include <math.h>
#include <string.h>
//#include "utilities.h"
//#include "timer.h"
//#include "radio_sx1276.h"
//#include "delay.h"
#include "sx1276.h"
//#include "sx1276-board.h"


//================== Semtech defines ====================

#define MODEM_TYPE                                   1        // 1 Lora , 0 FSK

#define TX_OUTPUT_POWER                             14        // 14 dBm

#define FREQU_DEV                                    0        //Sets the frequency deviation (FSK only)
                                                              //FSK : [Hz]
                                                              //LoRa: 0

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_CRC_ON                                 true
#define LORA_FREQU_HOP_ON                           false      
#define LORA_HopPeriod                              0
#define LORA_IQ_INVERSION_ON                        false
#define LORA_TIMEOUT                                3000

uint8_t lora_buffer[2];

//=========================== defines =========================================
#define SPI_PIN_SSI_CLK             GPIO_PIN_0      //    CLK
#define SPI_PIN_SSI_FSS             GPIO_PIN_1      //    CSn
#define SPI_PIN_SSI_RX              GPIO_PIN_2      //    MISO
#define SPI_PIN_SSI_TX              GPIO_PIN_3      //    MOSI
#define SPI_GPIO_SSI_BASE           GPIO_B_BASE

//=========================== variables =======================================

typedef struct {
    // information about the current transaction
    uint8_t*        pNextTxByte;
    uint16_t        numTxedBytes;
    uint16_t        txBytesLeft;
    spi_return_t    returnType;
    uint8_t*        pNextRxByte;
    uint16_t        maxRxBytes;
    spi_first_t     isFirst;
    spi_last_t      isLast;
    // state of the module
    uint8_t         busy;
#ifdef SPI_IN_INTERRUPT_MODE
    // callback when module done
    spi_cbt         callback;
#endif
} spi_vars_t;

spi_vars_t spi_vars;

//================ Variables to use as parameters of spi_txrx function ============

/*static char stringToPrint[]="MessageToTest\r\n";
size_t len=sizeof(stringToPrint);
uint8_t  spi_tx_buffer[2];
uint8_t  spi_rx_buffer[2];*/

//=========================== prototypes ======================================
static void disableInterrupts(void);
static void enableInterrupts(void);
//=========================== public ==========================================

void spi_init(){
    // clear variables
    memset(&spi_vars,0,sizeof(spi_vars_t));

    //Set the CLK , MOSI(TX) and MISO(RX) pins as Hardware-controlled (Configures GPIO_AFSEL-->1)
    GPIOPinTypeSSI(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_CLK );
    GPIOPinTypeSSI(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_RX );
    GPIOPinTypeSSI(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_TX );

    //Set the SS pin as Software-controlled outputs (configures GPIO_AFSEL-->0 (software) and GPIO_DIR-->1 (Output) )
    //Only FSS can be used as SOFTWARE controlled OUTPUT
    GPIOPinTypeGPIOOutput(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_FSS);

    //Set SS to High (writes into GPIO_DATA register)
    GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_FSS, SPI_PIN_SSI_FSS);

    /*//Set pins to low
    GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_TX, 0);
    GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_CLK, 0);*/

    //Configuration of SSI1 peripheral
    SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_SSI1);
    SysCtrlPeripheralSleepEnable(SYS_CTRL_PERIPH_SSI1);
    SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_SSI1);

    //Disabling SSI (writes into SSI_CR1 --> 0)
    SSIDisable(SSI1_BASE);

    //Set clock source (The baud clock source for the SSI to select.)
    SSIClockSourceSet(SSI1_BASE, SSI_CLOCK_PIOSC); //The precision internal oscillator

    /*Configure output signal to IOC_Pxx_SEL register
    Each signal is identified with its address table 9.1 in T.I datasheet*/
    IOCPinConfigPeriphOutput(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_CLK, IOC_MUX_OUT_SEL_SSI1_CLKOUT);
    IOCPinConfigPeriphOutput(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_TX, IOC_MUX_OUT_SEL_SSI1_TXD);
    IOCPinConfigPeriphOutput(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_FSS, IOC_MUX_OUT_SEL_SSI1_FSSOUT);
    //Configures hardware peripheral input selection (i.e. IOC_SSIRXD_SSI1)
    IOCPinConfigPeriphInput(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_RX, IOC_SSIRXD_SSI1);


    //Sets SSI_CR0 (SPI MODE and Data Size), SSI_CR1(Protocol -- Master/Slave), SSI_CPSR(Clock prescaler divisor)
    //According to the LoRaSX1276 datasheet, the corresponding mode is CPOL=0 , CPHA=0 which is FRF mode 0
    SSIConfigSetExpClk(SSI1_BASE, SysCtrlIOClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, SysCtrlIOClockGet()/200 /*16000000*/, 8);

    //Enable the SSI1 module(writes into CR1)
    SSIEnable(SSI1_BASE);
    
    //spi_tx_buffer[0]     = 0x42;
    //spi_tx_buffer[1]     = 0xFF;
    
    //Call the spi_txrx function
    //spi_txrx((uint8_t*)stringToPrint,len,SPI_FIRSTBYTE,(uint8_t*)16,16,SPI_FIRST,SPI_NOTLAST);
    //spi_txrx(spi_tx_buffer, sizeof(spi_tx_buffer),SPI_FIRSTBYTE,spi_rx_buffer,sizeof(spi_rx_buffer),SPI_FIRST,SPI_LAST);
    
    
    //=========== Calling LoRa process transmission functions ================
    
    /*void SX1276Init( RadioEvents_t *events )
    void SX1276Init( RadioEvents_t *events )*/
      
    /*Protoype of TXconfig function
    SX1276SetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev,
                        uint32_t bandwidth, uint32_t datarate,
                        uint8_t coderate, uint16_t preambleLen,
                        bool fixLen, bool crcOn, bool freqHopOn,
                        uint8_t hopPeriod, bool iqInverted, uint32_t timeout );*/
    
    SX1276SetTxConfig( MODEM_TYPE , TX_OUTPUT_POWER , FREQU_DEV,
                        LORA_BANDWIDTH , LORA_SPREADING_FACTOR,
                        LORA_CODINGRATE, LORA_PREAMBLE_LENGTH,
                        LORA_FIX_LENGTH_PAYLOAD_ON, LORA_CRC_ON, LORA_FREQU_HOP_ON,
                        LORA_HopPeriod, LORA_IQ_INVERSION_ON, LORA_TIMEOUT);
      
    /*This function writes the payload buffer and change the mode request to TX mode
    SX1276Send( uint8_t *buffer, uint8_t size );*/
    SX1276Send( lora_buffer, sizeof(lora_buffer));
    
    //Clear the GPIO
    GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_3, 0);
    
    //This function corresponds to the Interruption Dio0 which is specified for the sent packet
    //\param [IN] context User defined data object pointer to pass back on IRQ handler callback
    //SX1276OnDio0Irq( void* context );  
    
    //If TxDone interruption were called, set the GPIO to High
    GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_3, 1);
}

#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCb(spi_cbt cb) {
   spi_vars.spi_cb = cb;
}
#endif

void    spi_txrx(uint8_t*     bufTx,
                 uint16_t     lenbufTx,
                 spi_return_t returnType,
                 uint8_t*     bufRx,
                 uint16_t     maxLenBufRx,
                 spi_first_t  isFirst,
                 spi_last_t   isLast) {

    uint32_t data,i;
    GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_3, GPIO_PIN_3);
    // register spi frame to send
    spi_vars.pNextTxByte      =  bufTx;
    spi_vars.numTxedBytes     =  0;
    spi_vars.txBytesLeft      =  lenbufTx;
    spi_vars.returnType       =  returnType;
    spi_vars.pNextRxByte      =  bufRx;
    spi_vars.maxRxBytes       =  maxLenBufRx;
    spi_vars.isFirst          =  isFirst;
    spi_vars.isLast           =  isLast;

    // SPI is now busy
    spi_vars.busy             =  1;

    // lower CS signal to have slave listening (FSS is managed through software, not hardware)
    if (spi_vars.isFirst==SPI_FIRST) {
       GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_FSS, 0);
    }

    for ( i =  0; i < lenbufTx; i++)
    {
        // Push a byte
        SSIDataPut(SSI1_BASE, spi_vars.pNextTxByte[i]);

        // Wait until it is complete
        while(SSIBusy(SSI1_BASE));

        // Read a byte
        SSIDataGet(SSI1_BASE, &data);

        // Store the result
        spi_vars.pNextRxByte[i] = (uint8_t)(data & 0xFF);
        // one byte less to go
     }

     if (spi_vars.isLast==SPI_LAST) {
        GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_FSS, SPI_PIN_SSI_FSS);
     }

    // SPI is not busy anymore
    spi_vars.busy             =  0;
    GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_3, 0);
}

//=========================== private =========================================

port_INLINE void enableInterrupts(void)
{
    // Enable the SPI interrupt
    SSIIntEnable(SSI1_BASE, (SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR));

    // Enable the SPI interrupt
    IntEnable(INT_SSI1);
}

port_INLINE void disableInterrupts(void)
{
    // Disable the SPI interrupt
    SSIIntDisable(SSI1_BASE, (SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR));

    // Disable the SPI interrupt
    IntDisable(INT_SSI1);
}

//=========================== interrupt handlers ==============================

kick_scheduler_t spi_isr() {
#ifdef SPI_IN_INTERRUPT_MODE
    uint32_t data;
    // save the byte just received in the RX buffer
    status = SSIIntStatus(SSI1_BASE, true);

    // Clear SPI interrupt in the NVIC
    IntPendClear(INT_SSI1);

    SSIDataGet(SSI1_BASE, &data);

    // Store the result
    spi_vars.pNextRxByte = (uint8_t)(data & 0xFF);

    // one byte less to go
    spi_vars.pNextTxByte++;
    spi_vars.pNextRxByte++;
    spi_vars.txBytesLeft--;

    if (spi_vars.txBytesLeft>0) {
        // write next byte to TX buffer
        SSIDataPut(SSI1_BASE, *spi_vars.pNextTxByte);

    } else {
        // SPI is not busy anymore
        spi_vars.busy             =  0;

        // SPI is done!
        if (spi_vars.callback!=NULL) {
           // call the callback
            spi_vars.spi_cb();
            // kick the OS
            return KICK_SCHEDULER;
        }
    }

#endif
    return DO_NOT_KICK_SCHEDULER;
}
