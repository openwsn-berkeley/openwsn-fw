/**
\brief cc1200dk-specific definition of the "spi" bsp module.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, August 2016.
*/

#include "msp430f5438a.h"
#include "spi.h"
#include "leds.h"
#include "bsp.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
    // information about the current transaction
    uint8_t*        pNextTxByte;
    uint8_t         numTxedBytes;
    uint8_t         txBytesLeft;
    spi_return_t    returnType;
    uint8_t*        pNextRxByte;
    uint8_t         maxRxBytes;
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

//=========================== prototypes ======================================

//=========================== public ==========================================

void spi_init(void) {
    // clear variables
    memset(&spi_vars,0,sizeof(spi_vars_t));  
   
    UCB0CTL1   |=  UCSWRST ;                       // reset state
   
    // RF SPI0 CS as GPIO output high
    //
    P3SEL &= ~TRXEM_SPI_SC_N_PIN;
    P3OUT |=  TRXEM_SPI_SC_N_PIN;
    P3DIR |=  TRXEM_SPI_SC_N_PIN;
    // Configuration
    // -  8-bit
    // -  Master Mode
    //-  3-pin
    // -  synchronous mode
    // -  MSB first
    // -  Clock phase select = captured on first edge
    // -  Inactive state is low
    // -  SMCLK as clock source
    // -  Spi clk is adjusted corresponding to systemClock as the highest rate
    //    supported by the supported radios: this could be optimized and done
    //    after chip detect.
 
    UCB0CTL0  =  0x00+UCMST + UCSYNC + UCMODE_0 + UCMSB + UCCKPH;
    UCB0CTL1 |=  UCSSEL_2;
    UCB0BR1   =  0x00;
    UCB0BR0   =  0x06;
    // Configure port and pins
    // - MISO/MOSI/SCLK GPIO controlled by peripheral
    // - CS_n GPIO controlled manually, set to 1
 
    P3SEL    |=  TRXEM_SPI_MOSI_PIN | TRXEM_SPI_MISO_PIN | TRXEM_SPI_SCLK_PIN ;
    P3SEL    &= ~TRXEM_SPI_SC_N_PIN ;
    P3OUT    |=  TRXEM_SPI_SC_N_PIN | TRXEM_SPI_MISO_PIN ;/* Pullup on MISO */
   
    P3DIR    |=  TRXEM_SPI_SC_N_PIN;
    // In case not automatically set 
    P3DIR    |= TRXEM_SPI_MOSI_PIN | TRXEM_SPI_SCLK_PIN ;
    P3DIR    &= ~TRXEM_SPI_MISO_PIN ;
   
    UCB0CTL1     &= ~UCSWRST;
   
    // enable interrupts via the IEx SFRs
#ifdef SPI_IN_INTERRUPT_MODE
    UCB0IE       |=  UCRXIE;                      // we only enable the SPI RX interrupt
                                                  // since TX and RX happen concurrently,
                                                  // i.e. an RX completion necessarily
                                                  // implies a TX completion.
#endif
}

#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCb(spi_cbt cb) {
   spi_vars.spi_cb = cb;
}
#endif

void spi_txrx(uint8_t*     bufTx,
              uint8_t      lenbufTx,
              spi_return_t returnType,
              uint8_t*     bufRx,
              uint8_t      maxLenBufRx,
              spi_first_t  isFirst,
              spi_last_t   isLast) {

#ifdef SPI_IN_INTERRUPT_MODE
    // disable interrupts
    __disable_interrupt();
#endif
   
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
   
    // lower CS signal to have slave listening
    /* Pull CS_N low and wait for SO to go low before communication starts */
    if (spi_vars.isFirst==SPI_FIRST) {
        TRXEM_SPI_BEGIN();
        while(P3IN & TRXEM_SPI_MISO_PIN);
    }
   
#ifdef SPI_IN_INTERRUPT_MODE
    // implementation 1. use a callback function when transaction finishes
   
    // write first byte to TX buffer
    UCB0TXBUF                   = *spi_vars.pNextTxByte;
   
    // re-enable interrupts
    __enable_interrupt();
#else
   
    // send all bytes
    while (spi_vars.txBytesLeft>0) {
      
        TRXEM_SPI_TX(*spi_vars.pNextTxByte);
        // busy wait on the interrupt flag
        while ((UCB0IFG & UCRXIFG)==0);
        // save the byte just received in the RX buffer
        switch (spi_vars.returnType) {
            case SPI_FIRSTBYTE:
                if (spi_vars.numTxedBytes==0) {
                    *spi_vars.pNextRxByte   = UCB0RXBUF;
                }
                break;
            case SPI_BUFFER:
                *spi_vars.pNextRxByte      = UCB0RXBUF;
                spi_vars.pNextRxByte++;
                break;
            case SPI_LASTBYTE:
                *spi_vars.pNextRxByte      = UCB0RXBUF;
                break;
        }
        // one byte less to go
        spi_vars.pNextTxByte++;
        spi_vars.numTxedBytes++;
        spi_vars.txBytesLeft--;
    }
   
    // put CS signal high to signal end of transmission to slave
    if (spi_vars.isLast==SPI_LAST) {
        TRXEM_SPI_END();
    }
   
    // SPI is not busy anymore
    spi_vars.busy             =  0;
#endif
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t spi_isr(void) {
#ifdef SPI_IN_INTERRUPT_MODE   
    // save the byte just received in the RX buffer
    switch (spi_vars.returnType) {
        case SPI_FIRSTBYTE:
            if (spi_vars.numTxedBytes==0) {
                *spi_vars.pNextRxByte   = UCB0RXBUF;
            }
            break;
        case SPI_BUFFER:
            *spi_vars.pNextRxByte      = UCB0RXBUF;
            spi_vars.pNextRxByte++;
            break;
        case SPI_LASTBYTE:
            *spi_vars.pNextRxByte      = UCB0RXBUF;
            break;
    }
   
    // one byte less to go
    spi_vars.pNextTxByte++;
    spi_vars.numTxedBytes++;
    spi_vars.txBytesLeft--;
   
    if (spi_vars.txBytesLeft>0) {
        // write next byte to TX buffer
        UCB0TXBUF                = *spi_vars.pNextTxByte;
    } else {
        // put CS signal high to signal end of transmission to slave
        if (spi_vars.isLast==SPI_LAST) {
            P3OUT                 |=  0x01;
        }
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
    return DO_NOT_KICK_SCHEDULER;
#else
    // this should never happpen!
   
    // we can not print from within the BSP. Instead:
    // blink the error LED
    leds_error_blink();
    // reset the board
    board_reset();
   
    return DO_NOT_KICK_SCHEDULER; // we will not get here, statement to please compiler
#endif
}
