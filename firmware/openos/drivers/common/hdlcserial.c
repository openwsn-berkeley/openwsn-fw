#include "hdlcserial.h"
#include "board.h"
//#include "schedule.h"
#include "uart.h"

//callback variable
hdlc_rx_cbt hdlcrxCb;

//variables
uint16_t crc16;
char hdlc_buffer[HDLC_MAX_LEN];
uint16_t hdlc_index;
uint16_t hdlc_len;
volatile int hdlc_num_chars_left; // current # of chars remaining to TX
volatile char hdlc_numflags;//for header
volatile char hdlc_packetsum;
hdlc_state_t hdlc_state;
volatile char hdlc_tx_char_str[HDLC_MAX_LEN];           // pointer to remaining chars to tx
uint8_t tx_index_hdlc;//used in isr_hdlc_tx to index the array
char isStuffing;//used in isr_hdlc_rx to account for stuffing

//====prototypes=================
char fcs_calc(char *buffer,char length,uint16_t crc);
static uint16_t fcs_fcs16(uint16_t fcs, uint8_t data);

//=====function implementations==

void hdlcserial_init(){
  uart_setCallbacks(isr_hdlcserial_tx, isr_hdlcserial_rx);
  hdlc_index = 0;
  hdlc_len = 0;
  hdlc_state = HDLC_STATE_DONE_RECEIVING;//initially, we're not receiving anything
  hdlc_numflags = 0;
  //  hdlc_packetsum = 0;
  isStuffing = 0x00;
  tx_index_hdlc = 0;
  hdlc_num_chars_left = 0;
  uart_enableInterrupts();
}


void hdlcserial_send(uint8_t* str, uint16_t len){
  //compute crc:
  uint16_t    fcs = 0;
  uint8_t length = len;
  uint8_t count,i,c;
  uint8_t crc1;
    uint8_t crc2;
    uint8_t stuff_count,index;
  
  // prepare checksum
  #ifdef HDLC_XBEE
  crc1 = 0;
  for ( count = 0; count < length; count++ ) {
    crc1 += str[count];
  }
  crc1 = 0xff - crc1;
  #else
  fcs = (uint16_t) 0xffff;
  for( count=0;count <length;count++)
    fcs = fcs_fcs16(fcs, str[count]);
  fcs = ~fcs;//one's complement
  crc1 = fcs;
  crc2 = fcs>>8;
  #endif
  
  //prepare packet for transmission:
  //first count the number of bytes to stuff
  stuff_count = 0;
  #ifdef HDLC_XBEE
  for ( i=0;i<len;i++) if((str[i] == 0x7E) || (str[i] == 0x7D) || (str[i] == 0x11) || (str[i] == 0x13)) stuff_count++;
  #else
  for ( i=0;i<len;i++) if((str[i] == 0x7E) || (str[i] == 0x7D)) stuff_count++;
  #endif
  
  #ifdef HDLC_XBEE
  if ((crc1 == 0x7E) || (crc1 == 0x7D) || (crc1 == 0x11) || (crc1 == 0x13)) stuff_count++;
  #else
  if ((crc1 == 0x7E) || (crc1 == 0x7D)) stuff_count++;
  if ((crc2 == 0x7E) || (crc2 == 0x7D)) stuff_count++;
  #endif

  
  #ifdef HDLC_XBEE
  len += (4+stuff_count); // to account for header, length (2B), crc, and stuffing bytes
  #else
  len += (4+stuff_count);//to account for the header, footer, crc (2B), and stuffing bytes
  #endif
  //now fill hdlc_tx_char with the bytes to send.
  //perform byte stuffing
  hdlc_num_chars_left = len;  
   
  index=0;
  hdlc_tx_char_str[index++] = 0x7E; //first delimiter
  #ifdef HDLC_XBEE
  // fill in length
  hdlc_tx_char_str[index++] = (length >> 8)&0xFF;
  hdlc_tx_char_str[index++] = (length)&0xFF;
  #endif
  for ( c=0;c<length;c++){
    if (str[c]==0x7E){
      str[index] = 0x7D;
      str[index+1] = 0x5E;
      index+=2;
    }
    else if(str[c] == 0x7D){
      hdlc_tx_char_str[index] = 0x7D;
      hdlc_tx_char_str[index+1] = 0x5D;
      index+=2;
    }
    #ifdef HDLC_XBEE
    else if (str[c] == 0x11) {
      hdlc_tx_char_str[index] = 0x7D;
      hdlc_tx_char_str[index+1] = 0x31;
      index+=2;        
    }
    else if ( str[c] == 0x13) {
      hdlc_tx_char_str[index] = 0x7D;
      hdlc_tx_char_str[index+1] = 0x33;
      index+=2;
    }
    #endif
    else
      hdlc_tx_char_str[index++] = str[c];
  }
  // fill in crc
  hdlc_tx_char_str[index++] = crc1;//fill in crc
  #ifndef HDLC_XBEE
  hdlc_tx_char_str[index++] = crc2;  
  // end delimiter
  hdlc_tx_char_str[index++] = 0x7E;
  #endif
  
  tx_index_hdlc = 0;//reset the send index
  uart_writeByte(hdlc_tx_char_str[tx_index_hdlc++]);//write the first byte
}

void hdlcserial_setcb(hdlc_rx_cbt rxCb){
  hdlcrxCb = rxCb;
}


//============interrupt handlers===
void    isr_hdlcserial_rx(){  
    uint16_t c;
    uint8_t check;
  register char hdlcrxc = uart_readByte();//read the rx buffer
  uart_clearRxInterrupts();//this should be taken care of in hardware (when the byte is read, the interrupt is cleared)
  if (hdlcrxc == HDLC_HEADER_FLAG)
    ++hdlc_numflags;
  else 
    hdlc_numflags = 0;
  
  if ((hdlc_numflags == HDLC_HEADER_LEN) && (hdlc_state == HDLC_STATE_DONE_RECEIVING)) {//means we just started
  #ifdef HDLC_XBEE
    hdlc_state = HDLC_STATE_RECEIVING_LENGTH_MSB;
  #else 
    hdlc_state = HDLC_STATE_RECEIVING;
  #endif
    hdlc_index = 0;//reset counter
  }
  #ifdef HDLC_XBEE
  else if ((hdlc_numflags == 0) && (hdlc_state == HDLC_STATE_RECEIVING_LENGTH_MSB)){
    hdlc_len = (uint16_t)hdlcrxc << 8;
    hdlc_state = HDLC_STATE_RECEIVING_LENGTH_LSB;
  }
  else if ((hdlc_numflags == 0) && (hdlc_state == HDLC_STATE_RECEIVING_LENGTH_LSB)){
    hdlc_len |= 0xFF&(uint16_t)hdlcrxc;
    hdlc_state = HDLC_STATE_RECEIVING;
  }
  #endif
  else if((hdlc_numflags == 0) && (hdlc_state == HDLC_STATE_RECEIVING)){//getting the payload
    if(isStuffing == 0x01) // stuffed character recieved
      hdlc_buffer[hdlc_index-1] = hdlcrxc ^ 0x20;
    else //business as usual
      hdlc_buffer[hdlc_index++] = hdlcrxc;
  }
  #ifdef HDLC_XBEE
  else if (hdlc_state == HDLC_STATE_RECEIVING && hdlc_index == hdlc_len) { // checksum has arrived
    check = hdlcrxc;
    for ( c = 0; c < hdlc_len; c++ ) {
        check += hdlc_buffer[c];
    }
    if ( check == 0xFF ) {
        hdlcrxCb();
    }
  }
  #else
  else if ((hdlc_numflags == HDLC_HEADER_LEN) && (hdlc_state == HDLC_STATE_RECEIVING)) {//means we're done
    hdlc_state = HDLC_STATE_DONE_RECEIVING;
    //hdlc_index--;//because we increased it more than enough
    crc16 = hdlc_buffer[hdlc_index - 2] + (hdlc_buffer[hdlc_index-1] <<8);
    if (fcs_calc(hdlc_buffer,hdlc_index,crc16))
      //push the callback to the scheduler or just call it
      hdlcrxCb();
  }
  #endif
  
  //the next 3 lines enables removing stuffed bytes on the fly
  if(hdlcrxc == 0x7D)
    isStuffing = 0x01;
  else isStuffing = 0x00;
}


void    isr_hdlcserial_tx(){
  if (tx_index_hdlc<hdlc_num_chars_left) {
    uart_writeByte(hdlc_tx_char_str[tx_index_hdlc++]);
    uart_clearTxInterrupts();//I really am not comfortable putting this. I think the hardware takes care of it.
  } else{
    hdlc_num_chars_left = 0;
  }  
}
//============private===============
char fcs_calc(char *buffer,char length,uint16_t crc){//change function to bool?
  uint16_t    fcs;
  uint8_t count;
 
  fcs = (uint16_t) 0xffff;
  for ( count=0;count<length-2;count++)
    fcs = fcs_fcs16(fcs, buffer[count]);
  return ((~fcs) == crc); /* add 1's complement then compare*/
}

static uint16_t fcs_fcs16(uint16_t fcs, uint8_t data){
  return (fcs >> 8) ^ fcstab[(fcs ^ data) & 0xff];
}