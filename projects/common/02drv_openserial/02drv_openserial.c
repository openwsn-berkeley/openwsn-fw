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
#include "debugpins.h"
#include "leds.h"
#include "uart.h"
#include "sctimer.h"

// driver modules required
#include "openserial.h"

//=========================== defines =========================================

#define SCTIMER_PERIOD 328           // 328@32kHz ~ 10ms

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
   
   sctimer_set_callback(cb_compare);
   sctimer_setCompare(sctimer_readCounter()+SCTIMER_PERIOD);
   
   while(1) {
      board_sleep();
      debugpins_slot_toggle();
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
   sctimer_setCompare(sctimer_readCounter()+SCTIMER_PERIOD);
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

void idmanager_setJoinKey(void) {}
void idmanager_triggerAboutRoot(void) {}
void openbridge_triggerData(void) {}
void tcpinject_trigger(void) {}
void udpinject_trigger(void) {}
void icmpv6echo_trigger(void) {}
void icmpv6rpl_setDIOPeriod(void){}
void icmpv6rpl_setDAOPeriod(void){}
void icmpv6echo_setIsReplyEnabled(bool isEnabled){}
void sixtop_setEBPeriod(void){}
void sixtop_setKaPeriod(void){}
void sixtop_setHandler(void){}
void sixtop_request(void){}
void sixtop_addORremoveCellByInfo(void){}
void sixtop_setIsResponseEnabled(void){}
void icmpv6rpl_setMyDAGrank(void){}
void icmpv6rpl_getPreferredParentEui64(void){}
void schedule_setFrameLength(void){}
void ieee154e_setSlotDuration(void){}
void ieee154e_setIsSecurityEnabled(void){}
void ieee154e_setIsAckEnabled(void){}
void ieee154e_setSingleChannel(void){}
void sniffer_setListeningChannel(void){}
void msf_appPktPeriod(void){}
uint8_t msf_getsfid(void) {return 0;}

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
bool debugPrint_joined(void) {
   return FALSE;
}
