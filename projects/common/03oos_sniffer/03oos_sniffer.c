/**
\brief This program shows the use of the "sniffer" module.

\author Tengfei Chang <tengfei.chang@eecs.berkeley.edu>, June 2015.
*/

#include "board.h"
#include "radio.h"
#include "leds.h"
#include "bsp_timer.h"
#include "scheduler.h"
#include "03oos_sniffer.h"
#include "openserial.h"

//=========================== defines =========================================

#define LENGTH_PACKET   125+LENGTH_CRC ///< maximum length is 127 bytes
#define CHANNEL         20             ///< 20=2.450GHz
#define ID              0x99           ///< byte sent in the packets

//=========================== variables =======================================

enum {
   APP_FLAG_START_FRAME = 0x01,
   APP_FLAG_END_FRAME   = 0x02,
   APP_FLAG_IDLE        = 0x00,
};

typedef enum {
   APP_STATE_TX         = 0x01,
   APP_STATE_RX         = 0x02,
} app_state_t;

typedef struct {
   app_state_t          app_state;
   uint8_t              flag;
   uint8_t              packet[LENGTH_PACKET];
   uint8_t              packet_len;
    int8_t              rxpk_rssi;
   uint8_t              rxpk_lqi;
   bool                 rxpk_crc;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void     cb_startFrame(PORT_TIMER_WIDTH timestamp);
void     cb_endFrame(PORT_TIMER_WIDTH timestamp);
void     task_uploadPacket(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   
   // clear local variables
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize board
   board_init();
   scheduler_init();
   openserial_init();
 
   // add callback functions radio
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL);
   
   // switch in RX by default
   radio_rxEnable();
   radio_rxNow();
   
   scheduler_start();
   
   return 0;
}

//=========================== interface =======================================
void sniffer_setListeningChannel(uint8_t channel){
    while(app_vars.flag != APP_FLAG_IDLE);
        radio_rfOn();
        radio_setFrequency(channel);
        radio_rxEnable();
        radio_rxNow();
}

//=========================== callbacks =======================================

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
   app_vars.flag |= APP_FLAG_START_FRAME;
   // led
   leds_error_on();
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
   
   app_vars.flag |= APP_FLAG_END_FRAME;
   // done receiving a packet
   app_vars.packet_len = sizeof(app_vars.packet);
  
   // get packet from radio
   radio_getReceivedFrame(
      app_vars.packet,
      &app_vars.packet_len,
      sizeof(app_vars.packet),
      &app_vars.rxpk_rssi,
      &app_vars.rxpk_lqi,
      &app_vars.rxpk_crc
   );
   
   scheduler_push_task(task_uploadPacket,TASKPRIO_SNIFFER);
   app_vars.flag &= ~APP_FLAG_START_FRAME;
   app_vars.flag &= ~APP_FLAG_END_FRAME;
   // led
   leds_error_off();
}

// ================================ task =======================================
void task_uploadPacket(){
    openserial_printPacket(&(app_vars.packet[0]),app_vars.packet_len);
}
// ================================= stubbing ==================================
void openbridge_triggerData(){return;}
void sixtop_setEBPeriod(uint8_t ebPeriod){return;}
void ieee154e_setSingleChannel(uint8_t channel){return;}
void icmpv6rpl_setDIOPeriod(uint16_t dioPeriod) {return;}
void icmpv6rpl_setDAOPeriod(uint16_t daoPeriod) {return;}
void neighbors_setMyDAGrank(uint16_t rank) {return;}
void sixtop_setKaPeriod(uint8_t kaPeriod) {return;}
void ieee154e_setIsSecurityEnabled(bool isEnabled) {return;}
void schedule_setCustomFrameLength(uint16_t frameLength) {return;}
void icmpv6rpl_writeDODAGid() {return;}
void ieee154e_setIsAckEnabled(bool isEnabled) {return;}
void ieee154e_getAsn() {return;}
void neighbors_updateMyDAGrankAndNeighborPreference() {return;}
void schedule_startDAGroot() {return;}