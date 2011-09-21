#include "openwsn.h"
#include "appudpgina.h"
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

//=========================== variables =======================================

typedef struct {
   uint8_t              mesurements_left;
   OpenQueueEntry_t*    pktReceived;
} appudpgina_vars_t;

appudpgina_vars_t appudpgina_vars;

//=========================== prototypes ======================================

void appudpgina_send();
void appudpgina_reset();

//=========================== public ==========================================

void appudpgina_init() {
   if (*(&eui64+3)==0x09) {                      // this is a GINA board (not a basestation)
      gyro_init();
      large_range_accel_init();
      magnetometer_init();
      sensitive_accel_temperature_init();
   }
   appudpgina_vars.mesurements_left = 0;
}

//this is called when the UdpGina button is pressed on the OpenVisualizer interface
void appudpgina_trigger() {
}

//I just received a request, send a packet with IMU data
void appudpgina_receive(OpenQueueEntry_t* msg) {
   msg->owner = COMPONENT_APPUDPGINA;
   if (appudpgina_vars.pktReceived==NULL) {
      appudpgina_vars.pktReceived      = msg;
      appudpgina_vars.mesurements_left = appudpgina_vars.pktReceived->payload[0];
      appudpgina_send();
   } else {
      openqueue_freePacketBuffer(msg);
   }
}

//I just sent a IMU packet, check I need to resend one
void appudpgina_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_APPUDPGINA;
   if (msg->creator!=COMPONENT_APPUDPGINA) {
      openserial_printError(COMPONENT_APPUDPGINA,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
   if (appudpgina_vars.mesurements_left>0) {
      appudpgina_send();
   } else {
      appudpgina_reset();
   }
}

bool appudpgina_debugPrint() {
   return FALSE;
}

//=========================== private =========================================

void appudpgina_send() {
   OpenQueueEntry_t* packetToSend;
   packetToSend = openqueue_getFreePacketBuffer();
   if (packetToSend==NULL) {
      openserial_printError(COMPONENT_APPUDPGINA,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      appudpgina_reset();
      return;
   }
   packetToSend->creator                     = COMPONENT_APPUDPGINA;
   packetToSend->owner                       = COMPONENT_APPUDPGINA;
   packetToSend->l4_protocol                 = IANA_UDP;
   packetToSend->l4_sourcePortORicmpv6Type   = appudpgina_vars.pktReceived->l4_destination_port;
   packetToSend->l4_destination_port         = appudpgina_vars.pktReceived->l4_sourcePortORicmpv6Type;
   packetToSend->l3_destinationORsource.type = ADDR_128B;
   memcpy(&(packetToSend->l3_destinationORsource.addr_128b[0]),
         &(appudpgina_vars.pktReceived->l3_destinationORsource.addr_128b[0]),
         16);
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
   //send packet
   if ((udp_send(packetToSend))==E_FAIL) {
      openqueue_freePacketBuffer(packetToSend);
      appudpgina_reset();
   }
   appudpgina_vars.mesurements_left--;
}

void appudpgina_reset() {
   appudpgina_vars.mesurements_left=0;
   if (appudpgina_vars.pktReceived!=NULL) {
      openqueue_freePacketBuffer(appudpgina_vars.pktReceived);
      appudpgina_vars.pktReceived = NULL;
   }
}
