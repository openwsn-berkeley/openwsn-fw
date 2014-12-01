/**

\note: Since the bsp modules for different platforms have the same declaration,
       you can use this project with any platform.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"
#include "flash.h"
#include "uart.h"

//=========================== define ==========================================
#define PAGE255_ADDRESS    0x0807F800
#define ID_ADDRESS         0x0807FFF0
#define ID_LENGTH          8

#define WRP3_ADDRESS       0x1FFFF80E
#define nWRP3_VALUE        0xFF //page 62~255 is non- written protection

//address:0x14-15-92-05-01-01-00-01
#define HEADBYTE_FIR  0x1514  
#define HEADBYTE_SEC  0x0592
#define HEADBYTE_THR  0x0101
#define HEADBYTE_FOU  0x0100

//=========================== variables =======================================

//=========================== main ============================================

int mote_main(void) {
  
   uint8_t status;
  
   board_init();
   
   RCC_HSICmd(ENABLE);
   flash_init();
   
/* *******************************************************
   make page 62~255 becoming written protection by 
   setting WRP3 as WRP3_VALUE or non written protection
   by setting nWRP3_VALUE 
   *******************************************************/
   flash_erase_optByte();
   //if you want to add written protection on page 62~255, replace 0xFF by 0x7F
   status = flash_write_optByte(WRP3_ADDRESS,0xFF);
   
   /******************************************************
    if non-written protection, write EUI64 to page 255, 
    then make the page written protection
   ********************************************************/
   // checking status of page 62~255
   if(flash_read_optByte(WRP3_ADDRESS)&0x80)
   {
      // no written protection
      flash_erasePage(PAGE255_ADDRESS);
      flash_write(ID_ADDRESS,  HEADBYTE_FIR);
      flash_write(ID_ADDRESS+2,HEADBYTE_SEC);
      flash_write(ID_ADDRESS+4,HEADBYTE_THR);
      flash_write(ID_ADDRESS+6,HEADBYTE_FOU);
      // make page 62~255 written protection
      flash_erase_optByte();
      status = flash_write_optByte(WRP3_ADDRESS,0x7F);
      // check writing status
      if(status == 0x04)   leds_sync_on();
   }
   else
   {
     leds_error_on();
   }
   
   while (1) {
     board_sleep();
   }
}