#include "openwsn.h"
#include "imu.h"
//drivers
#include "gyro.h"
#include "large_range_accel.h"
#include "magnetometer.h"
#include "sensitive_accel_temperature.h"
//openwsn stack
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"

//=========================== variables =======================================

typedef struct {
   uint8_t              mesurements_left;
   OpenQueueEntry_t*    pktReceived;
} imu_vars_t;

imu_vars_t imu_vars;

//=========================== prototypes ======================================

void imu_send();
void imu_reset();

//=========================== public ==========================================

void imu_init() {
   if (*(&eui64+3)==0x09) {                      // this is a GINA board (not a basestation)
      gyro_init();
      large_range_accel_init();
      magnetometer_init();
      sensitive_accel_temperature_init();
   }
   imu_vars.mesurements_left = 0;
}

//this is called when the UdpGina button is pressed on the OpenVisualizer interface
void imu_trigger() {
}

//I just received a request, send a packet with IMU data
void imu_receive(OpenQueueEntry_t* msg) {
   msg->owner = COMPONENT_IMU;
   if (imu_vars.pktReceived==NULL) {
      imu_vars.pktReceived      = msg;
      imu_vars.mesurements_left = imu_vars.pktReceived->payload[0];
      imu_send();
   } else {
      openqueue_freePacketBuffer(msg);
   }
}

//I just sent a IMU packet, check I need to resend one
void imu_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_IMU;
   if (msg->creator!=COMPONENT_IMU) {
      openserial_printError(COMPONENT_IMU,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
   if (imu_vars.mesurements_left>0) {
      imu_send();
   } else {
      imu_reset();
   }
}

BOOL imu_debugPrint() {
   return FALSE;
}

//=========================== private =========================================

void imu_send() {
   OpenQueueEntry_t* packetToSend;
   packetToSend = openqueue_getFreePacketBuffer();
   if (packetToSend==NULL) {
      openserial_printError(COMPONENT_IMU,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      imu_reset();
      return;
   }
   packetToSend->creator                     = COMPONENT_IMU;
   packetToSend->owner                       = COMPONENT_IMU;
   packetToSend->l4_protocol                 = IANA_UDP;
   packetToSend->l4_sourcePortORicmpv6Type   = imu_vars.pktReceived->l4_destination_port;
   packetToSend->l4_destination_port         = imu_vars.pktReceived->l4_sourcePortORicmpv6Type;
   packetToSend->l3_destinationORsource.type = ADDR_128B;
   memcpy(&(packetToSend->l3_destinationORsource.addr_128b[0]),
         &(imu_vars.pktReceived->l3_destinationORsource.addr_128b[0]),
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
   if ((openudp_send(packetToSend))==E_FAIL) {
      openqueue_freePacketBuffer(packetToSend);
      imu_reset();
   }
   imu_vars.mesurements_left--;
}

void imu_reset() {
   imu_vars.mesurements_left=0;
   if (imu_vars.pktReceived!=NULL) {
      openqueue_freePacketBuffer(imu_vars.pktReceived);
      imu_vars.pktReceived = NULL;
   }
}
