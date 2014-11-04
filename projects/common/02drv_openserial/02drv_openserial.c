/**
\brief This is a program which shows how to use the "openserial" driver module.

Since the driver modules for different platforms have the same declaration, you
can use this project with any platform.

This application allows you to test that the openserial driver it working fine.
Once your board is running this application, use the serialTesterCli Python
application (part of the openwsn-sw repo) to issue serial echo commands, making
sure all is well.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, January 2013.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, January 2014.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"
#include "uart.h"
#include "bsp_timer.h"

// driver modules required
#include "openserial.h"

//=========================== defines =========================================

#define BSP_TIMER_PERIOD (32768/10)

//=========================== variables =======================================

typedef struct {
   uint8_t     timerFired;
   uint8_t     outputting;
   open_addr_t addr;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_compare(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
in order to echo chunks of bytes, each chunk needs to start with character 'H' as
openserial takes different actions according to the initial character of the stream.
*/
int mote_main(void) {
   
   board_init();
   openserial_init();
   
   bsp_timer_set_callback(cb_compare);
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
   
   while(1) {
      board_sleep();
      if (app_vars.timerFired==1) {
         app_vars.timerFired = 0;
         if (app_vars.outputting==1) {
            openserial_startInput();
            app_vars.outputting = 0;
         } else {
            openserial_startOutput();
            app_vars.outputting = 1;
         }
      }
   }
}

//=========================== callbacks =======================================

void cb_compare(void) {
   app_vars.timerFired = 1;
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
}

//=========================== stub functions ==================================

open_addr_t* idmanager_getMyID(uint8_t type) {
   return &app_vars.addr;
}

void ieee154e_getAsn(uint8_t* array) {
   array[0]   = 0x00;
   array[1]   = 0x01;
   array[2]   = 0x02;
   array[3]   = 0x03;
   array[4]   = 0x04;
}

void idmanager_triggerAboutRoot(void) {
}
void openbridge_triggerData(void) {
}
void tcpinject_trigger(void) {
}
void udpinject_trigger(void) {
}
void icmpv6echo_trigger(void) {
}

bool debugPrint_isSync(void) {
   return FALSE;
}
bool debugPrint_id(void) {
   return FALSE;
}
bool debugPrint_kaPeriod(void) {
   return FALSE;
}
bool debugPrint_myDAGrank(void) {
   return FALSE;
}
bool debugPrint_asn(void) {
   return FALSE;
}
bool debugPrint_macStats(void) {
   return FALSE;
}
bool debugPrint_schedule(void) {
   return FALSE;
}
bool debugPrint_backoff(void) {
   return FALSE;
}
bool debugPrint_queue(void) {
   return FALSE;
}
bool debugPrint_neighbors(void) {
   return FALSE;
}
