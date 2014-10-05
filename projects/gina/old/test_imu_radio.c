/**
\brief This is a standalone program which retrieves all the IMU sensors values on the
       GINA2.2b/c board

That is, gyro, large range accel, magnetometer, sensitive accel,
temperature and sends this data over the radio (30 byte payload, no headers).
Download the program to a GINA board, run it, and use a sniffer to see the packets
fly through the air.

The digital connection to the gyro, large range accel and magnetometeris is 
done through a two-wire I2C serial bus:
   - P5.2: B1_I2C_SCL
   - P5.1: B1_I2C_SDA

In addition:
   - P1.5: interrupt pin from the gyro (not used)
   - P1.7: interrupt pin from the large range accelerometer (not used)
   - P5.4: output to configure the I2C mode of the large range accelerometer (keep high)

The debug pins are:
   - P1.1: toggles at every measurement
   - P1.2: on during initial configuration and pulses at a new measurement
   - P1.3: toggles upon an USCIAB1TX_VECTOR interrupt (in ti_i2c.c)
   - P1.4: toggles upon an USCIAB1RX_VECTOR interrupt (in ti_i2c.c)

Speed:
   - one measurement every ~2.787ms

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

//board
#include "gina.h"
//drivers
#include "leds.h"
#include "gyro.h"
#include "timers.h"
#include "large_range_accel.h"
#include "magnetometer.h"
#include "sensitive_accel_temperature.h"
//openwsn
#include "opendefs.h"
#include "scheduler.h"
#include "packetfunctions.h"
#include "openqueue.h"
#include "radio.h"

OpenQueueEntry_t* testImuRadiopacketToSend;
uint8_t counter;
void task_application_imu_radio(uint16_t input);

void main(void) {
   //configuring
   P1OUT |=  0x04;                               // set P1.2 for debug
   P4DIR  |= 0x20;                               // P4.5 as output (for debug)
   
   gina_init();
   scheduler_init();
   leds_init();
   
   if (*(&eui64+3)==0x09) {                      // this is a GINA board (not a basestation)
      gyro_init();
      large_range_accel_init();
      magnetometer_init();
      sensitive_accel_temperature_init();
   }
   
   radio_init();
   timer_init();
   
   P1OUT &= ~0x04;                               // clear P1.2 for debug
   
   //check sensor configuration is right
   gyro_get_config();
   large_range_accel_get_config();
   magnetometer_get_config();
   sensitive_accel_temperature_get_config();

   //scheduler_push_task(ID_TASK_APPLICATION);
   
   scheduler_register_application_task(&task_application_imu_radio, 0, FALSE);

   scheduler_start();
}

void task_application_imu_radio(uint16_t input) {
   counter++;
   P1OUT ^= 0x02;                                // toggle P1.1 for debug
   P4OUT ^= 0x20;                                // toggle P4.5 for debug
   P1OUT |= 0x04;P1OUT &= ~0x04;                 // pulse P1.2 for debug
   if (counter==0) {
      leds_circular_shift();                     // circular shift LEDs for debug
   }
   //get RAM space for packet
   testImuRadiopacketToSend = openqueue_getFreePacketBuffer();
   //l1
   testImuRadiopacketToSend->l1_channel  = DEFAULTCHANNEL;
   //payload
   packetfunctions_reserveHeaderSize(testImuRadiopacketToSend,8);
   gyro_get_measurement(&(testImuRadiopacketToSend->payload[0]));
   P1OUT |= 0x04;P1OUT &= ~0x04;                 // pulse P1.2 for debug
   packetfunctions_reserveHeaderSize(testImuRadiopacketToSend,6);
   large_range_accel_get_measurement(&(testImuRadiopacketToSend->payload[0]));
   P1OUT |= 0x04;P1OUT &= ~0x04;                 // pulse P1.2 for debug
   packetfunctions_reserveHeaderSize(testImuRadiopacketToSend,6);
   magnetometer_get_measurement(&(testImuRadiopacketToSend->payload[0]));
   P1OUT |= 0x04;P1OUT &= ~0x04;                 // pulse P1.2 for debug
   packetfunctions_reserveHeaderSize(testImuRadiopacketToSend,10);
   sensitive_accel_temperature_get_measurement(&(testImuRadiopacketToSend->payload[0]));
   P1OUT |= 0x04;P1OUT &= ~0x04;                 // pulse P1.2 for debug
   packetfunctions_reserveFooterSize(testImuRadiopacketToSend,2);//space for radio to fill in CRC
   //send packet
   radio_send(testImuRadiopacketToSend);
}

void stupidmac_sendDone(OpenQueueEntry_t* pkt, error_t error) {
   openqueue_freePacketBuffer(pkt);
   scheduler_push_task(ID_TASK_APPLICATION);     // ask the scheduler to do run task_application again
}

void radio_packet_received(OpenQueueEntry_t* packetReceived) {
   P1OUT ^= 0x02;                                // toggle P1.1 for debug
   leds_circular_shift();                        // circular-shift LEDs for debug
}
