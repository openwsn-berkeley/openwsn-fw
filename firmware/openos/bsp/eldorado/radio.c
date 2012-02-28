/**
\brief TelosB-specific definition of the "radio" bsp module.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, February 2012.
*/

#include "PE_Types.h"
#include "string.h"
#include "radio.h"
#include "eldorado.h"
#include "spi.h"
#include "radiotimer.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   radiotimer_capture_cbt    startFrameCb;
   radiotimer_capture_cbt    endFrameCb;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

void    radio_spiWriteReg(uint8_t reg_addr, uint16_t reg_setting);
uint16_t radio_spiReadReg(uint8_t reg_addr);
void    radio_spiWriteTxFifo(uint8_t* bufToWrite, uint8_t lenToWrite);
void    radio_spiReadRxFifo(uint8_t* bufRead, uint8_t length);

//=========================== public ==========================================

void radio_init() {
   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   
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
   radio_vars.startFrameCb = cb;
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
   radio_vars.endFrameCb = cb;
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
   uint8_t junk;
   len = radio_spiReadReg(RX_PKT_LEN);//Read the RX packet length 
   len &= 0x007F;          /* Mask out all but the RX packet length */
   junk = SPI1S;
   junk = SPI1D;//as specified in the generated radio code
  
   if (len>2 && len<=127) {
      // retrieve the whole packet (including 1B SPI address, 1B length, the packet, 1B LQI)
      radio_spiReadRxFifo(bufRead,len);
   }
}

void radio_rfOff() {
   // turn radio off
   radio_spiWriteReg(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != TRX_OFF); // busy wait until done
}

//=========================== private =========================================

void radio_spiWriteReg(uint8_t reg_addr, uint16_t reg_setting) {
   uint8_t u8TempValue=0;

  MC13192DisableInterrupts();   // Necessary to prevent double SPI access 
  AssertCE();                   // Enables MC13192 SPI
   
  SPI1D = reg_addr & 0x3F;      // Write the command   
  while (!(SPI1S_SPRF));        // busywait
  u8TempValue = SPI1D;          //Clear receive data register. SPI entirely 

  SPI1D = ((UINT8)(u16Content >> 8));    /* Write MSB */       
  while (!(SPI1S_SPRF));         
  u8TempValue = SPI1D;         
  
  
  SPI1D = ((UINT8)(u16Content & 0x00FF));    /* Write LSB */
  while (!(SPI1S_SPRF)); 
  u8TempValue = SPI1D;
  
  DeAssertCE();
  MC13192RestoreInterrupts();
}

uint16_t radio_spiReadReg(uint8_t reg_addr) {
   uint8_t u8TempValue=0;
   uint16_t  u16Data=0;            /* u16Data[0] is MSB, u16Data[1] is LSB */


    MC13192DisableInterrupts(); /* Necessary to prevent double SPI access */
    AssertCE();                 /* Enables MC13192 SPI */
    
    
    SPI1D = ((u8Addr & 0x3f) | 0x80);      // Write the command   
    while (!(SPI1S_SPRF));        // busywait
    u8TempValue = SPI1D;          //Clear receive data register. SPI entirely 
    
   
    SPISendChar(reg_addr );      // Dummy write.
    while (!(SPI1S_SPRF));        // busywait
    ((UINT8*)u16Data)[0] = SPI1D;               /* MSB */
    
    
    SPISendChar(reg_addr );      // Dummy write.
    while (!(SPI1S_SPRF));        // busywait
    ((UINT8*)u16Data)[1] = SPI1D;               /* LSB */
    
    
    DeAssertCE();                     /* Disables MC13192 SPI */
    MC13192RestoreInterrupts();       /* Restore MC13192 interrupt status */
    return u16Data;
}

void radio_spiWriteTxFifo(uint8_t* bufToWrite, uint8_t  lenToWrite) {
     uint8_t junk;
     radio_spiWriteReg(TX_PKT_LEN, lenToWrite);/* Update the TX packet length field (2 extra bytes for CRC) */
     junk = SPI1S;
     junk = SPI1D;//as specified in the generated radio code
     
     //now check if we have an even number of bytes and update accordingly (the radio requires an even number)
     if(lenToWrite&0x01)
       lenToWrite +=1;
     
     uint8_t bufToWrite2[lenToWrite+1];//recreate packet with TX_PKT address
     bufToWrite2[0] = TX_PKT;
     for(junk=0;junk<lenToWrite;junk++)
       bufToWrite2[junk+1] = bufToWrite[junk];
       
     
     uint8_t spi_rx_buffer[lenToWrite+1];              // 1B SPI address + length	   
	   spi_txrx(bufToWrite,
	            lenToWrite+1,
	            SPI_BUFFER,
	            spi_rx_buffer,
	            sizeof(spi_rx_buffer),
	            SPI_FIRST,
	            SPI_LAST);
                              
}

void radio_spiReadRxFifo(uint8_t* bufRead, uint16_t length) {

  uint8_t junk;
  bool removeOneByte = 0;
  
  //now check if we have an even number of bytes and update accordingly (the radio requires an even number)
     if(length&0x01){
      length +=1;
      removeOneByte = 1;   //need to remove the MSB of the last word
     }

  uint8_t spi_tx_buffer[length+1+2];//1byte for the address+2junk bytes+length
  spi_tx_buffer[0] = RX_PKT | 0x80;  /* SPI RX ram data register */
  
  spi_txrx(spi_tx_buffer,
            length+1+2,
            SPI_BUFFER,
            bufRead,
            length+1+2,
            SPI_FIRST,
            SPI_LAST);
   //now remove the first three bytes because they're junk        
   bufRead = *bufRead + 3; //or should it be (&bufRead) = *bufRead + 3??? poipoi
   if(removeOneByte){
    bufRead(length-1) = bufread(length);
    length-=1;
    //refer to the datasheet to know why this is needed (even number of bytes)    
   }
}

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR (void) {
   uint16_t capturedTime;
   uint8_t  irq_status;
   // clear interrupt flag
   P1IFG &= ~0x40;
   // capture the time
   capturedTime = radiotimer_getCapturedTime();
   // reading IRQ_STATUS causes IRQ_RF (P1.6) to go low
   irq_status = radio_spiReadReg(RG_IRQ_STATUS);
   switch (irq_status) {
      case AT_IRQ_RX_START:
         if (radio_vars.startFrameCb!=NULL) {
            // call the callback
            radio_vars.startFrameCb(capturedTime);
            // make sure CPU restarts after leaving interrupt
            __bic_SR_register_on_exit(CPUOFF);
         }
         break;
      case AT_IRQ_TRX_END:
         if (radio_vars.endFrameCb!=NULL) {
            // call the callback
            radio_vars.endFrameCb(capturedTime);
            // make sure CPU restarts after leaving interrupt
            __bic_SR_register_on_exit(CPUOFF);
         }
         break;
      default:
         while(1);
         break;
   }
}