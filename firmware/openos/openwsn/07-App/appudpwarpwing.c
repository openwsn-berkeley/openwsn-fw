/**
\brief UDP WARPWING application

\author Ankur Mehta <mehtank@eecs.berkeley.edu>, October 2010
*/

#include "openwsn.h"
#include "appudpwarpwing.h"
//drivers
#include "gyro.h"
#include "large_range_accel.h"
#include "magnetometer.h"
#include "sensitive_accel_temperature.h"
//openwsn stack
#include "udp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
//scheduler 
#include "scheduler.h"
#include "leds.h"

//=========================== variables =======================================

uint8_t           appudpwarpwing_channel;

//=========================== prototypes ======================================

void appudpwarpwing_send();
void appudpwarpwing_reset();

//=========================== public ==========================================

void appudpwarpwing_init() {
   if (*(&eui64+3)==0x09) {                      // this is a GINA board (not a basestation)
      gyro_init();
      large_range_accel_init();
      magnetometer_init();
      sensitive_accel_temperature_init();
   }
}

//this is called when the UdpWarpwing button is pressed on the OpenVisualizer interface
void appudpwarpwing_trigger() {
}

//I just received a request, register task in the scheduler
void appudpwarpwing_receive(OpenQueueEntry_t* msg) {
   int period = 0;
   msg->owner = COMPONENT_APPUDPWARPWING;
#ifdef TASK_APPLICATION
   if (msg->length==3) {
      period = packetfunctions_ntohs(&(msg->payload[1]));
      if (period == 0) {
         appudpwarpwing_reset();
         openqueue_freePacketBuffer(msg);
      } else {
         scheduler_register_application_task(&appudpwarpwing_task, period, TRUE);
         appudpwarpwing_channel = msg->payload[0];
      }
   } else
     openqueue_freePacketBuffer(msg);
#else
         // Todo: send back error
   openqueue_freePacketBuffer(msg);
#endif

}

//I just sent a IMU packet, check I need to resend one
void appudpwarpwing_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_APPUDPWARPWING;
   if (msg->creator!=COMPONENT_APPUDPWARPWING) {
      openserial_printError(COMPONENT_APPUDPWARPWING,ERR_SENDDONE_FOR_MSG_I_DID_NOT_SEND,0,0);
   }
   openqueue_freePacketBuffer(msg);
}

bool appudpwarpwing_debugPrint() {
   return FALSE;
}

#ifdef TASK_APPLICATION
void appudpwarpwing_task(uint16_t n) {
   appudpwarpwing_send(n);
   leds_increment();
}
#endif

//=========================== private =========================================

void appudpwarpwing_send(uint16_t n) {
   OpenQueueEntry_t* packetToSend;
   packetToSend = openqueue_getFreePacketBuffer();
   if (packetToSend==NULL) {
      openserial_printError(COMPONENT_APPUDPWARPWING,ERR_NO_FREE_PACKET_BUFFER,0,0);
      appudpwarpwing_reset();
      return;
   }
   
   packetToSend->creator                     = COMPONENT_APPUDPWARPWING;
   packetToSend->owner                       = COMPONENT_APPUDPWARPWING;
   packetToSend->l1_channel = appudpwarpwing_channel;

   //payload, gyro data
   packetfunctions_reserveHeaderSize(packetToSend,8);
   gyro_get_measurement(&(packetToSend->payload[0]));
   //payload, large_range_accel data
   packetfunctions_reserveHeaderSize(packetToSend,6);
   large_range_accel_get_measurement(&(packetToSend->payload[0]));
   //payload, magnetometer data
   packetfunctions_reserveHeaderSize(packetToSend,6);
   magnetometer_get_measurement(&(packetToSend->payload[0]));
   //payload, sensitive_accel_temperature data
   packetfunctions_reserveHeaderSize(packetToSend,10);
   sensitive_accel_temperature_get_measurement(&(packetToSend->payload[0]));
   //packet number
   packetfunctions_reserveHeaderSize(packetToSend,2);
   packetfunctions_htons(n, &(packetToSend->payload[0]));
   //send packet
   
   if (radio_send(packetToSend)==E_FAIL) {
      leds_increment();
      // appudpwarpwing_reset();
   }
   openqueue_freePacketBuffer(packetToSend);
}

void appudpwarpwing_reset() {
#ifdef TASK_APPLICATION
   scheduler_free_application_task(&appudpwarpwing_task);
#endif
}
