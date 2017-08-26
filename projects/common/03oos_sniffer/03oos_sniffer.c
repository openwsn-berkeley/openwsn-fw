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
#include "idmanager.h"
#include "sixtop.h"
#include "processIE.h"
#include "neighbors.h"
#include "sf0.h"
#include "openrandom.h"

//=========================== defines =========================================

#define LENGTH_PACKET   125+LENGTH_CRC ///< maximum length is 127 bytes
#define CHANNEL         20             ///< 20=2.450GHz
#define ID              0x99           ///< byte sent in the packets
#define TIMER_PERIOD    0x1fff         

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
   uint8_t              channel;
   uint8_t              outputOrInput;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void     cb_startFrame(PORT_TIMER_WIDTH timestamp);
void     cb_endFrame(PORT_TIMER_WIDTH timestamp);
void     cb_timer(void);
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
   idmanager_init();
   openrandom_init();
 
   // add callback functions radio
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);
   
   // start bsp timer
   bsp_timer_set_callback(cb_timer);
   bsp_timer_scheduleIn(TIMER_PERIOD);
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL);
   app_vars.channel = CHANNEL;
   
   // switch in RX by default
   radio_rxEnable();
   radio_rxNow();
   
   scheduler_start();
   
   return 0;
}

//=========================== interface =======================================
void sniffer_setListeningChannel(uint8_t channel){
    
    while(app_vars.flag != APP_FLAG_IDLE);
    radio_rfOff();
    radio_rfOn();
    radio_setFrequency(channel);
    app_vars.channel = channel;
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

void cb_timer(void) {
    
   app_vars.outputOrInput = (app_vars.outputOrInput+1)%2;
   if (app_vars.outputOrInput == 1) {
       openserial_stop();
       openserial_startOutput();
   } else {
       openserial_stop();
       openserial_startInput();
   }
   // schedule again
   bsp_timer_scheduleIn(TIMER_PERIOD);
}

// ================================ task =======================================
void task_uploadPacket(){
    openserial_printSniffedPacket(
        &(app_vars.packet[0]),
        app_vars.packet_len,
        app_vars.channel
    );
}
// ================================= stubbing ==================================

void ieee154e_setSingleChannel(uint8_t channel)         {return;}
void ieee154e_setIsSecurityEnabled(bool isEnabled)      {return;}
void ieee154e_setSlotDuration(uint16_t duration)        {return;}
void ieee154e_setIsAckEnabled(bool isEnabled)           {return;}
void ieee154e_getAsn(uint8_t* array)                    {return;}

void schedule_startDAGroot(void)                        {return;}
void schedule_setFrameLength(uint16_t frameLength)      {return;}

void sixtop_setEBPeriod(uint8_t ebPeriod)               {return;}
void sixtop_addORremoveCellByInfo(uint8_t code,open_addr_t* neighbor,cellInfo_ht* cellInfo){return;}
void sixtop_request(uint8_t code,open_addr_t* neighbor, uint8_t numCells)   {return;}
bool sixtop_setHandler(six2six_handler_t handler)                           {return TRUE;}
void sixtop_setIsResponseEnabled(bool isEnabled)                            {return;}
void sixtop_setKaPeriod(uint16_t kaPeriod)                                  {return;}
void sf0_appPktPeriod(uint8_t numAppPacketsPerSlotFrame)                    {return;}

void openbridge_triggerData(void)                                           {return;}

void icmpv6rpl_setDIOPeriod(uint16_t dioPeriod)                             {return;}
void icmpv6rpl_setDAOPeriod(uint16_t daoPeriod)                             {return;}
void icmpv6rpl_setMyDAGrank(uint16_t rank)                                  {return;}
void icmpv6rpl_writeDODAGid(uint8_t* dodagid)                               {return;}
void icmpv6rpl_updateMyDAGrankAndParentSelection(void)                      {return;}
bool icmpv6rpl_getPreferredParentEui64(open_addr_t* neighbor)               {return TRUE;}
void icmpv6echo_setIsReplyEnabled(bool isEnabled)                           {return;}

bool debugPrint_asn(void)       {return TRUE;}
bool debugPrint_isSync(void)    {return TRUE;}
bool debugPrint_macStats(void)  {return TRUE;}
bool debugPrint_schedule(void)  {return TRUE;}
bool debugPrint_backoff(void)   {return TRUE;}
bool debugPrint_queue(void)     {return TRUE;}
bool debugPrint_neighbors(void) {return TRUE;}
bool debugPrint_myDAGrank(void) {return TRUE;}
bool debugPrint_kaPeriod(void)  {return TRUE;}
