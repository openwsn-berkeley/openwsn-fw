/**
\brief Eldorado-specific definition of the "radio" bsp module.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, February 2012.
*/

#include "stdint.h"
#include "string.h"
#include "radio.h"
#include "eldorado.h"
#include "spi.h"
#include "radiotimer.h"
#include <hidef.h>

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   radiotimer_capture_cbt    startFrameCb;
   radiotimer_capture_cbt    endFrameCb;
} radio_vars_t;

radio_vars_t radio_vars;
uint8_t counter; //declared as global because compiler is retarded
uint16_t lenvalue;//same reasons

//=========================== prototypes ======================================

void    radio_spiWriteReg(uint8_t reg_addr, uint16_t reg_setting);
uint16_t radio_spiReadReg(uint8_t reg_addr);
void    radio_spiWriteTxFifo(uint8_t* bufToWrite, uint8_t lenToWrite);
void    radio_spiReadRxFifo(uint8_t* bufRead, uint8_t length);

//=========================== public ==========================================

void radio_init() {
   //start fabien code:
	uint8_t temp;
   IRQInit();
   MC13192_RESET_PULLUP = 0;//pullup resistor on the RST pin
   MC13192_RESET_PORT = 1;//specify direction for RST pin
   MC13192_CE_PORT = 1;
   MC13192_ATTN_PORT = 1;
   MC13192_RTXEN_PORT = 1;
   MC13192_CE = 1;  //chip enable (SPI)   
   MC13192_ATTN = 1; //modem attenuation (used to send it to sleep mode for example)
   MC13192_RTXEN = 0;//to enable RX or TX
   
   //PA: put the external PA in bypass mode:
    EN_PA1_DIR = 0;
    EN_PA2_DIR = 0;
    EN_PA1 = 0;
    EN_PA2 = 1;
   
    
    
    MC13192_RESET = 1;//turn radio ON
    while (IRQSC_IRQF == 0);//busy wait until radio is on
    (void)radio_spiReadReg(0x24);// Clear MC13192 interrupts 
    IRQACK();                   /* ACK the pending IRQ interrupt */
    IRQPinEnable();             /* Pin Enable, IE, IRQ CLR, negative edge. */
    
    
    
   //init routine:
   
/*   The newest version of the MC1321x now uses an updated transceiver device. 
Although fully compliant with earlier versions of the transceiver, proper performance
of the radio requires that the following modem registers be over-programmed: Register
0x31 to 0xA0C0 Register 0x34 to 0xFEC6 These registers must be over-programmed for MC1321x
devices in which the modem Chip_ID Register 0x2C reads 0x6800.*/

   if(radio_spiReadReg(0x2C)==0x6800){
     radio_spiWriteReg(0x31,0xA0C0);
	 radio_spiWriteReg(0x34,0xFEC6);
	 }

   
   
	/* Please refer to document MC13192RM for hidden register initialization */
   //radio_spiWriteReg(0x04,0xA08D);   /* cca stuff, not needed */
   radio_spiWriteReg(0x05,0x0040);   /* no interrupts from this register */
   radio_spiWriteReg(0x06,0x4310);   /* TX_end interrupt, RX_end interrupt, start in idle mode */
   radio_spiWriteReg(0x07,0x4EC0);   /* Enable CLKo in Doze, ct bias enabled, dual port mode,  *///this registered used to send mote into doze mode
   //(0x08,0xFFF7);   // Preferred injection; this is the default
   //radio_spiWriteReg(0x09,0xF36B);   //default: clko enabled
   //radio_spiWriteReg(0x0A,0x7E86);   //default: clko is 32.768kHz
   //radio_spiWriteReg(0x0C,0x0380);   //default: IRQ pull-up enabled.
   radio_spiWriteReg(0x11,0x20FF);   /* Eliminate Unlock Conditions due to L01 THIS IS WRITTEN BY FREESCALE AND UNDOCUMENTED IN THE DATASHEET!!!*/
   radio_spiWriteReg(0x1B,0x8000);   /* Disable TC1. */
   radio_spiWriteReg(0x1D,0x8000);   /* Disable TC2. */
   radio_spiWriteReg(0x1F,0x8000);   /* Disable TC3. */
   radio_spiWriteReg(0x21,0x8000);   /* Disable TC4. */
   
   
   (void)radio_spiReadReg(0x25);           /* The RST_Ind Register 25 contains the reset indicator bit. Bit reset_ind is cleared during a reset and gets
set if Register 25 is read after a reset and remains set */

   

   
 
    /* Read the status register to clear any undesired IRQs. */
   (void)radio_spiReadReg(0x24);           /* Clear the status register, if set */
	
	//end init routine
   
   //end fabien code

   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   
 

   //busy wait until radio status is TRX_OFF
   
   //radiotimer_start(0xffff);//poipoi
}
/*
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
*/
void radio_reset() {
   PTDD_PTDD3 = 0;
   PTDD_PTDD3 = 1;//goes back to idle mode
}

void radio_setFrequency(uint8_t frequency) {
   // configure the radio to the right frequecy
   radio_spiWriteReg(LO1_IDIV_ADDR,0x0f95);
   radio_spiWriteReg(LO1_NUM_ADDR,frequency-11);//in this radio they index the channels from 0 to 15
}

void radio_rfOn() {
   //poipoi
   //to leave doze mode, assert ATTN then deassert it
   MC13192_ATTN = 0;
   MC13192_ATTN = 1;
}

void radio_loadPacket(uint8_t* packet, uint8_t len) {
   // load packet in TXFIFO
   radio_spiWriteTxFifo(packet,len);
}

void radio_txEnable() {
   // turn on radio's PLL
     //read status reg
   uint16_t stat_reg = radio_spiReadReg(MODE_ADDR);
   stat_reg &= 0xfff8;
   stat_reg |= TX_MODE;
   //turn off LNA
   //MC13192_LNA_CTRL = LNA_OFF;
   //turn on PA
   //MC13192_PA_CTRL = PA_ON;
   // put radio in reception mode
   radio_spiWriteReg(MODE_ADDR, stat_reg);
   while((radio_spiReadReg(STATUS_ADDR) & 0x8000)); // busy wait until pll locks
}

void radio_txNow() {
   // send packet by assterting the RTXEN pin
   MC13192_RTXEN = 1;
   
   
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
   //read status reg
   uint16_t stat_reg = radio_spiReadReg(MODE_ADDR);
   stat_reg &= 0xfff8;
   stat_reg |= RX_MODE;
   //turn on LNA
   //MC13192_LNA_CTRL = LNA_ON;
   //turn off PA
   //MC13192_PA_CTRL = PA_OFF;
   // put radio in reception mode
   radio_spiWriteReg(MODE_ADDR, stat_reg);
   MC13192_RTXEN = 1;//assert the signal to put radio in rx mode.
   while((radio_spiReadReg(STATUS_ADDR) & 0x8000)); // busy wait until pll locks
}

void radio_rxNow() {
   // nothing to do
}

void radio_getReceivedFrame(uint8_t* bufRead,
                            uint8_t* lenRead,
                            uint8_t  maxBufLen,
                                int* rssi,
                            uint8_t* lqi,
                               bool* crc) {
   uint16_t temp;
   uint8_t len;
   uint8_t junk;
   temp = radio_spiReadReg(RX_PKT_LEN);//Read the RX packet length (includes CRC) //If the 2-byte CRC data is read. The byte order is reversed (last CRC byte isread first).
   temp &= 0x007F;          /* Mask out all but the RX packet length */
   len = (uint8_t) temp;//LSB of the 16-bit register value
   junk = SPI1S;
   junk = SPI1D;//as specified in the generated radio code to remove any unwanted interrupts
  
   if (len>2 && len<=127) {
      // retrieve the whole packet (including 1B SPI address, 1B length, the packet, 1B LQI)
      radio_spiReadRxFifo(bufRead,len);
   }
   
   //modify this function with the new variables: lenRead, maxBufLen, rssi, lqi, crc: look in telos for examples.
}

void radio_rfOff() {// turn radio off
   uint16_t stat_reg;
   
   MC13192_RTXEN = 0;//go back to idle
   /* The state to enter is "Acoma Doze Mode" because CLKO is made available at any setting
   //now put the radio in idle mode
   //read status reg
   stat_reg = radio_spiReadReg(MODE_ADDR);
   stat_reg &= 0xfff8;
   stat_reg |= IDLE_MODE;
   radio_spiWriteReg(MODE_ADDR, stat_reg);  
   //turn PA off
   MC13192_PA_CTRL = PA_OFF;
   //turn LNA off
   MC13192_LNA_CTRL = LNA_OFF;
   
   //while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != TRX_OFF); // busy wait until done */ //revisit this function REVIEW poipoi
   
}

//=========================== private =========================================

void radio_spiWriteReg(uint8_t reg_addr, uint16_t reg_setting) {
   uint8_t u8TempValue=0;
   
   //clear interrupts
   u8TempValue = SPI1S;
   u8TempValue = SPI1D;

   MC13192_IRQ_Disable();   // Necessary to prevent double SPI access 
  MC13192_CE = 0;                   // Enables MC13192 SPI
   
  SPI1D = reg_addr & 0x3F;      // Write the command   
  while (!(SPI1S_SPRF));        // busywait
  u8TempValue = SPI1D;          //Clear receive data register. SPI entirely 

  SPI1D = ((uint8_t)(reg_setting >> 8));    /* Write MSB */       
  while (!(SPI1S_SPRF));         
  u8TempValue = SPI1D;         
  
  
  SPI1D = ((uint8_t)(reg_setting & 0x00FF));    /* Write LSB */
  while (!(SPI1S_SPRF)); 
  u8TempValue = SPI1D;
  
  
  u8TempValue = SPI1S;//to remove interrupts?
  MC13192_CE = 1;
  //MC13192_IRQ_Enable();
}

uint16_t radio_spiReadReg(uint8_t reg_addr) {
   uint8_t u8TempValue=0;
   uint16_t  u16Data=0;            /* u16Data[0] is MSB, u16Data[1] is LSB */

   //clear interrupts
   u8TempValue = SPI1S;
   u8TempValue = SPI1D;
   
   MC13192_IRQ_Disable(); /* Necessary to prevent double SPI access */
    MC13192_CE = 0;                 /* Enables MC13192 SPI */
    
    
    SPI1D = ((reg_addr & 0x3f) | 0x80);      // Write the command   
    while (!(SPI1S_SPRF));        // busywait
    u8TempValue = SPI1D;          //Clear receive data register. SPI entirely 
    
   
    SPI1D = reg_addr| 0x80;;    // Dummy write.
    while (!(SPI1S_SPRF));        // busywait
    ((uint8_t*)u16Data)[0] = SPI1D;               /* MSB */
    
    
    SPI1D = reg_addr| 0x80;;    // Dummy write.
    while (!(SPI1S_SPRF));        // busywait
    ((uint8_t*)u16Data)[1] = SPI1D;               /* LSB */
    
    u8TempValue = SPI1S;
    MC13192_CE = 1;                     /* Disables MC13192 SPI */
    //MC13192_IRQ_Enable();       /* Restore MC13192 interrupt status */
    return u16Data;
}

void radio_spiWriteTxFifo(uint8_t* bufToWrite, uint8_t  lenToWrite) {
     uint8_t bufToWrite2[127+1]; 
     uint8_t spi_rx_buffer[127+1];
	 uint8_t temp;//used as garbage value
	 uint16_t lenvalue;//used because we need to write 16 bit values inside registers
	 
	 //the compiler is being stupid
	 lenvalue = (uint16_t)(lenToWrite << 8);//lsB of lenvalue is the length (this length already includes the two crc bytes)
	 lenvalue = lenvalue>>8;
	 
     radio_spiWriteReg(0x03, lenvalue);/*Choose RAM register 1; Update the TX packet length field (2 extra bytes for CRC) */
     temp = SPI1S;
     temp = SPI1D;//as specified in the generated radio code (removes interrupts and such)
     
     lenToWrite -= 2;//we need not write two empty bytes to the radio like for other manufacturers.
	 //now check if we have an even number of bytes and update accordingly (the radio requires an even number)
     if(lenToWrite&0x01)
       lenToWrite +=1;
     

     //[lenToWrite+1];//recreate packet with TX_PKT spi register address (0x02)
     bufToWrite2[0] = TX_PKT;
     for(counter=0;counter<lenToWrite;counter++)
       bufToWrite2[counter+1] = bufToWrite[counter];
         
	 spi_txrx(bufToWrite2,
	            lenToWrite+1,//+1 because we need to account for the spi command
	            SPI_BUFFER,
	            spi_rx_buffer,
	            sizeof(spi_rx_buffer),
	            SPI_FIRST,
	            SPI_LAST);
	   
	 //EnableInterrupts;
                              
}

void radio_spiReadRxFifo(uint8_t* bufRead, uint8_t length) {

  uint8_t junk;
  bool removeOneByte = 0;
  uint8_t spi_tx_buffer[127+1+2];//1byte for the address+2junk bytes+length
  //now check if we have an even number of bytes and update accordingly (the radio requires an even number of reads)
     if(length&0x01){
      length +=1;
      removeOneByte = 1;   //need to remove the MSB of the last word
     }

  
  spi_tx_buffer[0] = RX_PKT | 0x80;  /* SPI RX ram data register; | 0x80 to specify it's a read */
  
  spi_txrx(spi_tx_buffer,
            length+1+2,
            SPI_BUFFER,
            bufRead,
            length+1+2,
            SPI_FIRST,
            SPI_LAST);
   //now remove the first three bytes because they're junk        
   bufRead = bufRead + 3; //or should it be (&bufRead) = *bufRead + 3??? poipoi
   if(removeOneByte){
    bufRead[length-1] = bufRead[length];
    length-=1;
    //refer to the datasheet to know why this is needed (even number of bytes)    
   }
}

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

ISR(IRQIsr){ 
	//REVIEW poipoi prototype: interrupt void IRQIsr(void); maybe in eldorado.h

   uint16_t capturedTime;
   uint16_t  irq_status;
   // clear interrupt flag
   IRQSC |= 0x04;
   // capture the time
   //capturedTime = radiotimer_getCapturedTime();
   // reading IRQ_STATUS causes IRQ_RF (P1.6) to go low
   irq_status = radio_spiReadReg(STATUS_ADDR);
   
   //note: we may need to change the !=0 to !=1
   if((irq_status&RX_IRQ_MASK)!=0){
     if (radio_vars.endFrameCb!=NULL) {
		 MC13192_RTXEN = 0;//deassert the signal to put radio in idle mode.
            // call the callback
            radio_vars.endFrameCb(capturedTime);
            // make sure CPU restarts after leaving interrupt
            //poipoi
         }
   
   }
   
   //note: we may need to change the !=0 to !=1   
   if((irq_status&TX_IRQ_MASK)!=0){
     if (radio_vars.endFrameCb!=NULL) {
		 MC13192_RTXEN = 0;//deassert the signal to put radio in idle mode.
            // call the callback
            radio_vars.endFrameCb(capturedTime);
            // make sure CPU restarts after leaving interrupt
            //poipoi
         }
   }
   
   //WARNING! We need to find a fix for this!!! We can't capture time because the radio does not provide an interrupt on RX_Start
   /*   case AT_IRQ_RX_START:
         if (radio_vars.startFrameCb!=NULL) {
            // call the callback
            radio_vars.startFrameCb(capturedTime);
            // make sure CPU restarts after leaving interrupt
            __bic_SR_register_on_exit(CPUOFF);
         }
         break;
 */ 
}