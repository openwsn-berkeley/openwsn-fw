#include "hdlcserial.h"
#include "board.h"
//#include "schedule.h"
#include "uart.h"

//callback variable
hdlc_rx_cbt hdlcrxCb;

//variables
uint16_t crc16;
char hdlc_buffer[HDLC_MAX_LEN];
char hdlc_index;
char hdlc_len;
volatile int hdlc_num_chars_left; // current # of chars remaining to TX
volatile char hdlc_numflags;//for header
volatile char hdlc_packetsum;
hdlc_state_t hdlc_state;
volatile char hdlc_tx_char_str[HDLC_MAX_LEN];           // pointer to remaining chars to tx
uint8_t tx_index;//used in isr_hdlc_tx to index the array
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
  tx_index = 0;
  hdlc_num_chars_left = 0;
  uart_enableInterrupts();
}


void hdlcserial_send(uint8_t* str, uint16_t len){
  //compute crc:
  uint16_t    fcs = 0;
  uint8_t length = len;
  fcs = (uint16_t) 0xffff;
  for(uint8_t count=0;count <length;count++)
    fcs = fcs_fcs16(fcs, str[count]);
  fcs = ~fcs;//one's complement
  
  //prepare packet for transmission:
  //first count the number of bytes to stuff
  uint8_t stuff_count = 0;
  for (uint8_t i=0;i<len;i++) if((str[i] == 0x7E) || (str[i] == 0x7D)) stuff_count++;
  uint8_t crc1;
  uint8_t crc2;
  crc1 = fcs;
  crc2 = fcs>>8;
  if ((crc1 == 0x7E) || (crc1 == 0x7D)) stuff_count++;
  if ((crc2 == 0x7E) || (crc2 == 0x7D)) stuff_count++;
  str[len] = crc1;//fill in crc
  str[len+1] = crc2;
  
  len += (4+stuff_count);//to account for the header, footer and crc stuffing bytes
  //now fill hdlc_tx_char with the bytes to send.
  //perform byte stuffing
  hdlc_num_chars_left = len;  
  uint8_t index=1;
  hdlc_tx_char_str[0] = 0x7E; //first delimiter
  for (uint8_t c=0;c<length+2;c++){//length+2 for crc
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
    else
      hdlc_tx_char_str[index++] = str[c];
  }
  hdlc_tx_char_str[len-1] = 0x7E;
  
  tx_index = 0;//reset the send index
  uart_writeByte(hdlc_tx_char_str[tx_index++]);//write the first byte
}

void hdlcserial_setcb(hdlc_rx_cbt rxCb){
  hdlcrxCb = rxCb;
}


//============interrupt handlers===
void    isr_hdlcserial_rx(){  
  register char hdlcrxc = uart_readByte();//read the rx buffer
  uart_clearRxInterrupts();//this should be taken care of in hardware (when the byte is read, the interrupt is cleared)
  if (hdlcrxc == HDLC_HEADER_FLAG)
    ++hdlc_numflags;
  else 
    hdlc_numflags = 0;
  
  if ((hdlc_numflags == HDLC_HEADER_LEN) && (hdlc_state == HDLC_STATE_DONE_RECEIVING)) {//means we just started
    hdlc_state = HDLC_STATE_RECEIVING;
    hdlc_index = 0;//reset counter
  }
  else if((hdlc_numflags == 0) && (hdlc_state == HDLC_STATE_RECEIVING)){//getting the payload
    if(isStuffing == 0x01 && hdlcrxc==0x5E) //or 5D
      hdlc_buffer[hdlc_index-1] = 0x7E;
    else if ((isStuffing == 0x01) && (hdlcrxc == 0x5D));//ignore this byte
    else //business as usual
      hdlc_buffer[hdlc_index++] = hdlcrxc;
  }
  else if ((hdlc_numflags == HDLC_HEADER_LEN) && (hdlc_state == HDLC_STATE_RECEIVING)) {//means we're done
    hdlc_state = HDLC_STATE_DONE_RECEIVING;
    //hdlc_index--;//because we increased it more than enough
    crc16 = hdlc_buffer[hdlc_index - 2] + (hdlc_buffer[hdlc_index-1] <<8);
    if (fcs_calc(hdlc_buffer,hdlc_index,crc16))
      //push the callback to the scheduler or just call it
      hdlcrxCb();
  }
  
  //the next 3 lines enables removing stuffed bytes on the fly
  if(hdlcrxc == 0x7D)
    isStuffing = 0x01;
  else isStuffing = 0x00;
}


void    isr_hdlcserial_tx(){
  if (tx_index<hdlc_num_chars_left) {
    uart_writeByte(hdlc_tx_char_str[tx_index++]);
    uart_clearTxInterrupts();//I really am not comfortable putting this. I think the hardware takes care of it.
  } else{
    hdlc_num_chars_left = 0;
  }  
}
//============private===============
char fcs_calc(char *buffer,char length,uint16_t crc){//change function to bool?
  uint16_t    fcs = 0;
  fcs = (uint16_t) 0xffff;
  for (uint8_t count=0;count<length-2;count++)
    fcs = fcs_fcs16(fcs, buffer[count]);
  return ((~fcs) == crc); /* add 1's complement then compare*/
}

static uint16_t fcs_fcs16(uint16_t fcs, uint8_t data){
  return (fcs >> 8) ^ fcstab[(fcs ^ data) & 0xff];
}