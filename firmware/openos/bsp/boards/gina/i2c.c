#include "msp430x26x.h"
#include "i2c.h"

//=========================== variables =======================================

struct {
   volatile unsigned char* ctl0[2];
   volatile unsigned char* ctl1[2];
   volatile unsigned char* br0[2];
   volatile unsigned char* br1[2];
   volatile unsigned char* i2cie[2];
   volatile unsigned char* stat[2];
   volatile unsigned char* txbuf[2];
   volatile unsigned const char* rxbuf[2];
   volatile unsigned short* i2coa[2];
   volatile unsigned short* i2csa[2];
   
   volatile unsigned char* port[2];
   
   volatile unsigned char* ie[2];
   unsigned char ierx[2];
   unsigned char ietx[2];
   
   volatile unsigned char* iflag[2];
   unsigned char iflagrx[2];
   unsigned char iflagtx[2];
} i2c_control; //table of control registers

unsigned char *TI_transmit_field;
unsigned char *TI_receive_field;
signed   char byteCtr;

//=========================== prototypes ======================================

void i2c_init_transmit(int bus_num,unsigned char slave_address);
void i2c_transmit(int bus_num,unsigned char byteCount, unsigned char *field);
void i2c_init_receive(int bus_num,unsigned char slave_address);
void i2c_receive(int bus_num,unsigned char byteCount, unsigned char *field);
unsigned char i2c_busy(int bus_num);

//=========================== public ==========================================

//#define bus_num 1

void i2c_init() {
   i2c_control.ctl0[0]=&UCB0CTL0;
   i2c_control.ctl0[1]=&UCB1CTL0;
   i2c_control.ctl1[0]=&UCB0CTL1;
   i2c_control.ctl1[1]=&UCB1CTL1;
   i2c_control.br0[0]=&UCB0BR0;
   i2c_control.br0[1]=&UCB1BR0;
   i2c_control.br1[0]=&UCB0BR1;
   i2c_control.br1[1]=&UCB1BR1;
   i2c_control.i2cie[0]=&UCB0I2CIE;
   i2c_control.i2cie[1]=&UCB1I2CIE;
   i2c_control.stat[0]=&UCB0STAT;
   i2c_control.stat[1]=&UCB1STAT;
   i2c_control.txbuf[0]=&UCB0TXBUF;
   i2c_control.txbuf[1]=&UCB1TXBUF;
   i2c_control.rxbuf[0]=&UCB0RXBUF;
   i2c_control.rxbuf[1]=&UCB1RXBUF;
   i2c_control.i2coa[0]=&UCB0I2COA;
   i2c_control.i2coa[1]=&UCB1I2COA;
   i2c_control.i2csa[0]=&UCB0I2CSA;
   i2c_control.i2csa[1]=&UCB1I2CSA;
   
   i2c_control.port[0]=&P3SEL;
   i2c_control.port[1]=&P5SEL;
   
   i2c_control.ie[0]=&IE2;
   i2c_control.ie[1]=&UC1IE;
   i2c_control.ietx[0]=UCB0TXIE;
   i2c_control.ietx[1]=UCB1TXIE;
   i2c_control.ierx[0]=UCB0RXIE;
   i2c_control.ierx[1]=UCB1RXIE;
   
   i2c_control.iflag[0]=&IFG2;
   i2c_control.iflag[1]=&UC1IFG;
   i2c_control.iflagrx[0]=UCB0RXIFG;
   i2c_control.iflagrx[1]=UCB1RXIFG;
   i2c_control.iflagtx[0]=UCB0TXIFG;
   i2c_control.iflagtx[1]=UCB1TXIFG;
}

void i2c_write_register(uint8_t bus_num, uint8_t slave_addr, uint8_t reg_addr, uint8_t reg_setting) {
   uint8_t i2c_packet[2] = {reg_addr, reg_setting};
   while ( i2c_busy(bus_num) );
   i2c_init_transmit(bus_num,slave_addr);
   i2c_transmit(bus_num,sizeof(i2c_packet),i2c_packet);
   while ( i2c_busy(bus_num) );
   __delay_cycles(I2C_BUS_FREE_TIME);
}

void i2c_read_registers(uint8_t bus_num, uint8_t slave_addr, uint8_t reg_addr, uint8_t numBytes, uint8_t* spaceToWrite) {
   uint8_t i2c_packet[1] = {reg_addr};
   //transmit reg address to gyro
   i2c_init_transmit(bus_num,slave_addr);
   while ( i2c_busy(bus_num) );
   i2c_transmit(bus_num,sizeof(i2c_packet),i2c_packet);
   while ( i2c_busy(bus_num) );
   __delay_cycles(I2C_BUS_FREE_TIME);
   //read what gyro has to say
   i2c_init_receive(bus_num,slave_addr);
   while ( i2c_busy(bus_num) );
   i2c_receive(bus_num,numBytes,spaceToWrite);
   while ( i2c_busy(bus_num) );
   __delay_cycles(I2C_BUS_FREE_TIME);
}

//-----------------------------------------------------------------------------
// unsigned char i2c_slave_present(unsigned char slave_address)
//
// This function is used to look for a slave address on the I2C bus.  
// It sends a START and STOP condition back-to-back and wait for an ACK from the slave
//
// IN:   unsigned char slave_address  =>  Slave Address
// OUT:  unsigned char                =>  0: address was not found, 
//                                        1: address found
//-----------------------------------------------------------------------------
unsigned char i2c_slave_present(int bus_num, unsigned char slave_address) {
   unsigned char uc1ie_bak, slaveadr_bak, ucb1i2cie_bak, returnValue;
   //store state
   ucb1i2cie_bak =  *i2c_control.i2cie[bus_num];                   // store *i2c_control.i2cie[bus_num] register
   uc1ie_bak     =  *i2c_control.ie[bus_num];                       // store UC1IE register
   slaveadr_bak  =  *i2c_control.i2csa[bus_num];                   // store old slave address
   //start i2c
   while ( i2c_busy(bus_num) );
   i2c_init_transmit(bus_num,slave_address);
   //
   *i2c_control.i2cie[bus_num]    &= ~UCNACKIE;                    // no NACK interrupt
   *i2c_control.i2csa[bus_num]     =  slave_address;               // set slave address
   *i2c_control.ie[bus_num]        &= ~(i2c_control.ietx[bus_num] + i2c_control.ierx[bus_num]);       // no RX or TX interrupts
   __disable_interrupt();
   *i2c_control.ctl1[bus_num]     |=  UCTR + UCTXSTT + UCTXSTP;    // transmitter, start and stop condition
   while (*i2c_control.ctl1[bus_num] & UCTXSTP);                   // wait for STOP condition
   returnValue   = !(*i2c_control.stat[bus_num] & UCNACKIFG);      // 0 if an ACK was received
   __enable_interrupt();
   
   //restore state
   *i2c_control.ie[bus_num]         =  uc1ie_bak;                   // restore IE2
   *i2c_control.i2csa[bus_num]     =  slaveadr_bak;                // restore old slave address
   *i2c_control.i2cie[bus_num]     =  ucb1i2cie_bak;               // restore old UCB0CTL1
   return returnValue;                           // return whether or not 
}

//=========================== private =========================================

//-----------------------------------------------------------------------------
// void i2c_init_transmit(unsigned char slave_address)
//
// This function initializes the USCI module for master-transmit operation. 
//
// IN:   unsigned char slave_address   =>  Slave Address
//-----------------------------------------------------------------------------
void i2c_init_transmit(int bus_num, unsigned char slave_address){
   *i2c_control.port[bus_num]     |=  SDA_PIN + SCL_PIN;              // Assign I2C pins to USCI_B1
   *i2c_control.ctl1[bus_num]   =  UCSWRST;                        // set SW reset on USCI module
   *i2c_control.ctl0[bus_num]   =  UCMST + UCMODE_3 + UCSYNC;      // I2C Master, synchronous mode
   *i2c_control.ctl1[bus_num]   =  UCSSEL_2 + UCSWRST;             // Use SMCLK, keep SW reset
   *i2c_control.br0[bus_num]    =  I2C_PRESCALE;                   // set prescaler (SCL speed)
   *i2c_control.br1[bus_num]    =  0;
   *i2c_control.i2csa[bus_num]  =  slave_address;                  // Set slave address
   *i2c_control.ctl1[bus_num]  &= ~UCSWRST;                        // clear SW reset on USCI module
   *i2c_control.i2cie[bus_num]  =  UCNACKIE;                       // Not-acknowledge interrupt enabled
   *i2c_control.ie[bus_num]      =  i2c_control.ietx[bus_num];                       // USCI_B1 transmit interrupt enabled
}

//-----------------------------------------------------------------------------
// void i2c_transmit(unsigned char byteCount, unsigned char *field)
//
// This function is used to start an I2C commuincation in master-transmit mode. 
//
// IN:   unsigned char byteCount  =>  number of bytes that should be transmitted
//       unsigned char *field     =>  array variable. Its content will be sent.
//-----------------------------------------------------------------------------
void i2c_transmit(int bus_num, unsigned char byteCount, unsigned char *field) {
   TI_transmit_field = field;
   byteCtr   = byteCount;
   *i2c_control.ctl1[bus_num] |= UCTR + UCTXSTT;                   // I2C transmitter, transmit start condition
}

//-----------------------------------------------------------------------------
// void i2c_init_receive(unsigned char slave_address)
//
// This function initializes the USCI module for master-receive operation. 
//
// IN:   unsigned char slave_address   =>  Slave Address
//-----------------------------------------------------------------------------
void i2c_init_receive(int bus_num, unsigned char slave_address) {
   *i2c_control.port[bus_num]     |=  SDA_PIN + SCL_PIN;              // Assign I2C pins to USCI_B1
   *i2c_control.ctl1[bus_num]   =  UCSWRST;                        // set SW reset on USCI module
   *i2c_control.ctl0[bus_num]   =  UCMST + UCMODE_3 + UCSYNC;      // I2C Master, synchronous mode
   *i2c_control.ctl1[bus_num]   =  UCSSEL_2 + UCSWRST;             // Use SMCLK, keep SW reset (NOT master)
   *i2c_control.br0[bus_num]    =  I2C_PRESCALE;                   // set prescaler
   *i2c_control.br1[bus_num]    =  0;
   *i2c_control.i2csa[bus_num]  =  slave_address;                  // set slave address
   *i2c_control.ctl1[bus_num]  &= ~UCSWRST;                        // clear SW reset on USCI module
   *i2c_control.i2cie[bus_num]  =  UCNACKIE;                       // No-acknowledge interrupt enabled
   *i2c_control.ie[bus_num]      =  i2c_control.ierx[bus_num];                       // Enable RX interrupt
}

//-----------------------------------------------------------------------------
// void i2c_receive(unsigned char byteCount, unsigned char *field)
//
// This function is used to start an I2C commuincation in master-receiver mode. 
//
// IN:   unsigned char byteCount  =>  number of bytes that should be read
//       unsigned char *field     =>  array variable used to store received data
//-----------------------------------------------------------------------------
void i2c_receive(int bus_num, unsigned char byteCount, unsigned char *field) {
   TI_receive_field = field;
   if ( byteCount == 1 ){
      byteCtr = 0 ;
      __disable_interrupt();
      *i2c_control.ctl1[bus_num] |= UCTXSTT;                       // I2C start condition
      while (*i2c_control.ctl1[bus_num] & UCTXSTT);                // Start condition sent?
      *i2c_control.ctl1[bus_num] |= UCTXSTP;                       // I2C stop condition
      __enable_interrupt();
   } else if ( byteCount > 1 ) {
      byteCtr = byteCount - 2 ;
      *i2c_control.ctl1[bus_num] |= UCTXSTT;                       // I2C start condition
   } else {
      while (1);                                 // illegal parameter
   }
}

//-----------------------------------------------------------------------------
// unsigned char i2c_busy()
//
// This function is used to check if there is commuincation in progress. 
//
// OUT:  unsigned char  =>  0: I2C bus is idle, 
//                          1: communication is in progress
//-----------------------------------------------------------------------------
unsigned char i2c_busy(int bus_num) {
   return (*i2c_control.stat[bus_num] & UCBBUSY);
}

//=========================== interrupt handlers ==============================

//executed in ISR, called from scheduler.c
void isr_i2c_rx(int bus_num) {
   // fires when a data byte is received from the slave
   if (*i2c_control.stat[bus_num] & UCNACKIFG) {                   // if slave does not send ACK (only happens after I send addr)
      *i2c_control.ctl1[bus_num] |=  UCTXSTP;                      // send STOP
      *i2c_control.stat[bus_num] &= ~UCNACKIFG;                    // clear NACK IFG
   }
}

//executed in ISR, called from scheduler.c
void isr_i2c_tx(int bus_num) {
   // fires when a byte of data (including START) is written
   // from the register to the USCI shift register
   if (*i2c_control.iflag[bus_num] & i2c_control.iflagrx[bus_num]){                      // I received something
     *TI_receive_field = *i2c_control.rxbuf[bus_num];
      TI_receive_field++;
      if ( byteCtr == 0 ) {
         *i2c_control.ctl1[bus_num] |= UCTXSTP;                    // I2C stop condition
      } else {
         byteCtr--;
      }
   } else {
     //didn't receive anything, I will transmit
      if (byteCtr == 0){
         *i2c_control.ctl1[bus_num] |=  UCTXSTP;                   // I2C stop condition
         *i2c_control.iflag[bus_num]   &= ~i2c_control.iflagtx[bus_num];                 // Clear USCI_B0 TX int flag
      } else {
         *i2c_control.txbuf[bus_num] = *TI_transmit_field;
         TI_transmit_field++;
         byteCtr--;
      }
   }
}
