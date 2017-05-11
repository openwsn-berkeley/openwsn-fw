/**
\brief This is a program which shows how to use the bsp modules for the board
       and UART.

\note: Since the bsp modules for different platforms have the same declaration,
       you can use this project with any platform.

Load this program on your board. Open a serial terminal client (e.g. PuTTY or
TeraTerm):
- You will read "Hello World!" printed over and over on your terminal client.
- when you enter a character on the client, the board echoes it back (i.e. you
  see the character on the terminal client) and the "ERROR" led blinks.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "stdio.h"
#include "string.h"
// bsp modules required
#include "board.h"
#include "uart.h"
#include "leds.h"
#include "radio.h"

//=========================== defines =========================================

#define CHANNEL        26

#define FRAME_START   0x7E
#define FRAME_END     0xE7


#define FRAME_LENGTH    13
#define PACKAGE_LENGTH 256

#define LENGTH_CRC       2
#define LENGTH_PACKET  125+LENGTH_CRC ///< maximum length is 127 bytes

#define PAYLOAD_SIZE    90

static const uint8_t c_readConfig[] = {
   0x7e,0x00,0x08,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0xe7
}; 

// configure baudrate 115200, ID (not set), packet content 256 bytes
static const uint8_t c_configParameter[] = {
   0x7e,0x00,0x08,0x00,0x03,0x05,0xff,0x01,0x00,0x00,0x00,0x10,0xe7
}; 

// auto focus once
static const uint8_t c_configfocus[] = {
   0x7e,0x00,0x08,0x00,0x09,0x01,0x00,0x00,0x00,0x00,0x00,0x12,0xe7
};

// picture: 800*600, grey, quality middle
static const uint8_t c_takePicture[] = {
   0x7e,0x00,0x08,0x00,0x05,0x03,0x13,0x01,0x00,0x00,0x00,0x24,0xe7
}; 

// could be changed
uint8_t c_uploadPic[] = {
   0x7e,0x00,0x08,0x00,0x07,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0xe7
}; 

static const uint8_t c_admin_restoreFactorySetting[] = {
   0x7e,0x00,0x08,0x00,0x10,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0xe7
};

static const uint8_t c_admin_reset[] = {
   0x7e,0x00,0x08,0x00,0x10,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0xe7
};

typedef enum {
    S_IDLE       = 0,
    S_CONFIG     = 1,
    S_FOCUS      = 2,
    S_TAKEPIC    = 3,
    S_UPLOAD     = 4,
    S_TRANSMIT_1 = 5,
    S_TRANSMIT_2 = 6,
    S_TRANSMIT_3 = 7,
} next_action_t;

#define S_MAXACTION  8
#define PIC_TO_TAKE  1

#define INDEX_PACKET_SEG_1 11
#define INDEX_PACKET_SEG_2 11+PAYLOAD_SIZE
#define INDEX_PACKET_SEG_3 11+PAYLOAD_SIZE+PAYLOAD_SIZE

//=========================== variables =======================================

typedef struct {
              uint8_t uart_lastTxByteIndex;
             uint16_t uart_lastRxByteIndex;
   volatile   uint8_t uartDone;
   volatile   uint8_t uartSendNow;
              uint8_t uartReceiveBytes[FRAME_LENGTH];
              uint8_t uartReceiveBytesWithPackage[PACKAGE_LENGTH+FRAME_LENGTH];
        next_action_t nextAction;
              uint8_t packageNumberL;
              uint8_t packageNumberH;
              uint8_t packet[LENGTH_PACKET];
             uint16_t currentPackageSize;
             uint16_t currentpackageIndex;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_compare(void);
void cb_uartTxDone(void);
void cb_uartRxCb(void);

void cb_startFrame(PORT_RADIOTIMER_WIDTH timestamp);
void cb_endFrame(PORT_RADIOTIMER_WIDTH timestamp);

//==== helper ====
uint16_t findCharAtEndOfBuffer(uint8_t* buffer,uint16_t length,uint8_t target);
uint8_t checksum(uint8_t* buffer, uint8_t length);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
  
   uint16_t i,j,temp;
   
   
   // clear local variable
   memset(&app_vars,0,sizeof(app_vars_t));
    
   app_vars.uartSendNow = 1;
   
   // initialize the board
   board_init();
   
   // setup UART
   uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);
   uart_enableInterrupts();
   
   // set up radio
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL);
   
   while(1) {
      
      // wait for timer to elapse
      while (app_vars.uartSendNow==0);
      app_vars.uartSendNow = 0;
      leds_sync_off();
      
      // send string over UART
      app_vars.uartDone              = 0;
      app_vars.uart_lastTxByteIndex  = 0;
      if (app_vars.nextAction==S_IDLE){
          app_vars.nextAction++;
      }
      switch (app_vars.nextAction){
      case S_IDLE    :
        // do nothing
        break;
      case S_CONFIG  :
        uart_writeByte(c_configParameter[app_vars.uart_lastTxByteIndex]);
        break;
      case S_FOCUS   :
        uart_writeByte(c_configfocus[app_vars.uart_lastTxByteIndex]);
        break;
      case S_TAKEPIC :
        uart_writeByte(c_takePicture[app_vars.uart_lastTxByteIndex]);
        break;
      case S_UPLOAD  :
        if (app_vars.currentpackageIndex == app_vars.packageNumberH*256+app_vars.packageNumberL){
            // Done!!!
            return 0;
        } else {
            app_vars.currentpackageIndex++;
        }
        // upload package 01
        c_uploadPic[6]  = (uint8_t)((app_vars.currentpackageIndex>>8) & 0xff);
        c_uploadPic[7]  = (uint8_t)(app_vars.currentpackageIndex      & 0xff);
        // total number packet
        c_uploadPic[8]  = app_vars.packageNumberH;
        c_uploadPic[9]  = app_vars.packageNumberL;
        // checksum
        c_uploadPic[11] = checksum(&c_uploadPic[1],11);
        memset(&app_vars.uartReceiveBytesWithPackage[0],0,PACKAGE_LENGTH+FRAME_LENGTH);
        uart_writeByte(c_uploadPic[app_vars.uart_lastTxByteIndex]);
        break;
      case S_TRANSMIT_1:
        // reset radio status
        radio_rfOff();
        
        temp = findCharAtEndOfBuffer(&app_vars.uartReceiveBytesWithPackage[0],PACKAGE_LENGTH+FRAME_LENGTH,FRAME_END);
        if (temp > 12 && temp <PACKAGE_LENGTH+FRAME_LENGTH){
            app_vars.currentPackageSize = temp - 1 - 11; // checksum+header and parameters
        } else {
            app_vars.currentPackageSize = 0;
            return 0;
        }
        
        // prepare packet
        if (app_vars.currentPackageSize/PAYLOAD_SIZE>0){
            for (i=0;i<PAYLOAD_SIZE;i++) {
                app_vars.packet[i] = app_vars.uartReceiveBytesWithPackage[i+INDEX_PACKET_SEG_1];
            }
            radio_loadPacket(app_vars.packet,PAYLOAD_SIZE);
        } else {
            for (i=0;i<app_vars.currentPackageSize;i++) {
                app_vars.packet[i] = app_vars.uartReceiveBytesWithPackage[i+INDEX_PACKET_SEG_1];
            }
            radio_loadPacket(app_vars.packet,app_vars.currentPackageSize);
        }
        
        // start transmitting packet
        radio_txEnable();
        radio_txNow();
        break;
      case S_TRANSMIT_2:
        // reset radio status
        radio_rfOff();
        if (app_vars.currentPackageSize==0){
            return 0;
        }
        
        if (app_vars.currentPackageSize/PAYLOAD_SIZE>1){
            for (i=0;i<PAYLOAD_SIZE;i++) {
                app_vars.packet[i] = app_vars.uartReceiveBytesWithPackage[i+INDEX_PACKET_SEG_2];
            }
            radio_loadPacket(app_vars.packet,PAYLOAD_SIZE);
        } else {
            for (i=0;i<app_vars.currentPackageSize-PAYLOAD_SIZE;i++) {
                app_vars.packet[i] = app_vars.uartReceiveBytesWithPackage[i+INDEX_PACKET_SEG_2];
            }
            radio_loadPacket(app_vars.packet,app_vars.currentPackageSize-PAYLOAD_SIZE);
        }
        // start transmitting packet
        radio_txEnable();
        radio_txNow();
        break;
      case S_TRANSMIT_3:
        // reset radio status
        radio_rfOff();
        if (app_vars.currentPackageSize==0){
            return 0;
        }
        
        if (app_vars.currentPackageSize/PAYLOAD_SIZE>2){
            // something wrong
            return 0;
        } else {
            for (i=0;i<app_vars.currentPackageSize-2*PAYLOAD_SIZE;i++) {
                app_vars.packet[i] = app_vars.uartReceiveBytesWithPackage[i+INDEX_PACKET_SEG_3];
            }
            radio_loadPacket(app_vars.packet,app_vars.currentPackageSize-2*PAYLOAD_SIZE);
        }
        // start transmitting packet
        radio_txEnable();
        radio_txNow();
        break;
      }
      while(app_vars.uartDone==0);
      // a little delay
      for (i=0;i<0xfff;i++){
        for (j=0;j<0xff;j++);
      }
   }
   return 0;
}

//=========================== helper ==========================================

uint16_t findCharAtEndOfBuffer(uint8_t* buffer,uint16_t length,uint8_t target){
    uint16_t i;
    for (i=length-1;i>0;i--){
        if (target == buffer[i]){
            break;
        }
    }
    return i;
}

uint8_t checksum(uint8_t* buffer, uint8_t length){
    uint8_t i;
    uint8_t checksum = 0;
    for (i=0;i<length;i++){
        checksum += buffer[i];
    }
    return checksum;
}

//=========================== callbacks =======================================

void cb_uartTxDone(void) {
   app_vars.uart_lastTxByteIndex++;
   if (app_vars.uart_lastTxByteIndex<FRAME_LENGTH) {
      switch (app_vars.nextAction){
      case S_IDLE    :
        // do nothing
        break;
      case S_CONFIG  :
        uart_writeByte(c_configParameter[app_vars.uart_lastTxByteIndex]);
        break;
      case S_FOCUS   :
        uart_writeByte(c_configfocus[app_vars.uart_lastTxByteIndex]);
        break;
      case S_TAKEPIC :
        uart_writeByte(c_takePicture[app_vars.uart_lastTxByteIndex]);
        break;
      case S_UPLOAD  :
        uart_writeByte(c_uploadPic[app_vars.uart_lastTxByteIndex]);
        break;
      }
   } else {
      app_vars.uartDone = 1;
   }
}

void cb_uartRxCb(void) {
   uint16_t byteLengthToRead = FRAME_LENGTH;
   // toggle LED
   leds_error_toggle();
   
   // read received byte
   if (app_vars.nextAction==S_UPLOAD){
      byteLengthToRead +=  PACKAGE_LENGTH;
      app_vars.uartReceiveBytesWithPackage[app_vars.uart_lastRxByteIndex] = uart_readByte();
   } else {
      app_vars.uartReceiveBytes[app_vars.uart_lastRxByteIndex] = uart_readByte();
   }
   app_vars.uart_lastRxByteIndex++;
   
   // echo that byte over serial
   if (app_vars.uart_lastRxByteIndex==byteLengthToRead){
      leds_sync_on();
      app_vars.uart_lastRxByteIndex = 0;
      if (app_vars.nextAction==S_TAKEPIC){
          app_vars.packageNumberL  = app_vars.uartReceiveBytes[10];
          app_vars.packageNumberH |= app_vars.uartReceiveBytes[9];
      }
      app_vars.nextAction = (app_vars.nextAction+1)%S_MAXACTION;
      app_vars.uartSendNow = 1;
   }
}

void cb_startFrame(PORT_RADIOTIMER_WIDTH timestamp) {

}

void cb_endFrame(PORT_RADIOTIMER_WIDTH timestamp) {
    if (app_vars.nextAction == S_TRANSMIT_3){
        app_vars.nextAction  = S_UPLOAD;
    } else {
        app_vars.nextAction = (app_vars.nextAction+1)%S_MAXACTION;
    }
    app_vars.uartSendNow = 1;
    app_vars.uartDone    = 1;
}