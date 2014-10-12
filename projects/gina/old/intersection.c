/**
\brief Smart intersection application on GINA

\author Fabien Chraim <chraim@eecs.berkeley.edu>, October 2010
*/

//board
#include "gina.h"
//drivers
#include "leds.h"
#include "timers.h"
#include "magnetometer.h"

//openwsn
#include "opendefs.h"
#include "scheduler.h"
#include "packetfunctions.h"
#include "openqueue.h"
#include "radio.h"

OpenQueueEntry_t* testIntersection;
uint8_t counter;
uint32_t timer_period;
uint8_t mag_reading[6];
int mag_X;
int mag_Y;
int mag_Z;
long XX;
long YY;
long ZZ;
long mag_norm;
float filt_norm;
float alpha[9];
float delay[9];
float out[9];
float threshold;

//state machine variables
uint8_t state;
uint8_t FSMcounter;
uint8_t maxCount;
uint8_t minCount;
bool seenCar;


//prototypes
void task_application_intersection(uint16_t input);
void timer_b5_fired();
int mul_int(int x, int y);
int div_int(int x, int y);
float LPF(long);


//state machine states
enum {
  NOCAR = 0,
  PERHAPS = 1,
  CAR = 2,
};

void main(void) {
   //configuring
   P1OUT |=  0x04;                               // set P1.2 for debug
   P4DIR  |= 0x20;                               // P4.5 as output (for debug)
   
   gina_init();
   scheduler_init();
   leds_init();
   
   if (*(&eui64+3)==0x09) {                      // this is a GINA board (not a basestation)
      magnetometer_init();
   }
   
   radio_init();
   timer_init();
   
   P1OUT &= ~0x04;                               // clear P1.2 for debug
   
   //check sensor configuration is right
   magnetometer_get_config();

   //scheduler_push_task(ID_TASK_APPLICATION);
   
   //initialize variables
   timer_period = 0x033333; //set the timer frequency to 80Hz
   
   for (int c=0;c<9;c++)
   {
     delay[c] = 0;
     out[c] = 0;
   }

   alpha[0]=	0.423466145992279;
   alpha[1]=	0.359764546155930;
   alpha[2]=	0.134587764739990;
   alpha[3]=	0.445259362459183;
   alpha[4]=	0.134587764739990;
   alpha[5]=	0.400678455829620;
   alpha[6]=	0.134587764739990;
   alpha[7]=	0.160087645053864;
   alpha[8]=	0.134587764739990;
   
   //FSM variable initialization
   threshold = 0.1096;
   state = NOCAR; //initial state
   FSMcounter = 0;
   maxCount = 10; //change?
   minCount = 2;
   seenCar=0;
   
   scheduler_register_application_task(&task_application_intersection, 410, TRUE);

   scheduler_start();
}

void task_application_intersection(uint16_t input) {
   counter++;
   P1OUT ^= 0x02;                                // toggle P1.1 for debug
   P4OUT ^= 0x20;                                // toggle P4.5 for debug
   P1OUT |= 0x04;P1OUT &= ~0x04;                 // pulse P1.2 for debug
   if (counter==0) {
      leds_circular_shift();                     // circular shift LEDs for debug
   }
   
   //get RAM space for packet
   testIntersection = openqueue_getFreePacketBuffer();
   //l1
   testIntersection->l1_channel  = DEFAULTCHANNEL;
   
   P1OUT ^= 0x02;                                // toggle P1.1 for debug
   
   magnetometer_get_measurement(mag_reading);
   mag_X = 0;
   mag_Y = 0;
   mag_Z = 0;
   
   mag_X |= mag_reading[0]<<8;
   mag_X |= mag_reading[1];

   mag_Y |= mag_reading[2]<<8;
   mag_Y |= mag_reading[3];
   
   mag_Z |= mag_reading[4]<<8;
   mag_Z |= mag_reading[5];
   
   //note: in the following I use functions for simple multiplications
   //and divisions for easy replacements in case the number of
   //instruction cycles is too large to be acceptable in this application
   mag_X = div_int(mag_X, 970);
   mag_Y = div_int(mag_Y, 970);
   mag_Z = div_int(mag_Z, 970);//970: look in HMC5843 datasheet

   XX = mul_int(mag_X,mag_X);
   YY = mul_int(mag_Y,mag_Y);
   ZZ = mul_int(mag_Z,mag_Z);
   
   mag_norm = XX + YY + ZZ; //no sqrt for faster execution
   filt_norm = LPF(mag_norm);
   
   //here we enter the state machine
   
   switch (state) {
   case NOCAR:
     if (filt_norm>=threshold){
       FSMcounter = 1;
       state = PERHAPS;
     }
     break; //else you break
     
   case PERHAPS:
     if (filt_norm >= threshold && FSMcounter < maxCount){
       FSMcounter++;
     }
     
     else if (filt_norm < threshold && FSMcounter >minCount)
       FSMcounter--;
     
     else if (filt_norm < threshold && FSMcounter <=minCount){
       state = NOCAR;
       FSMcounter=0;
       if(seenCar){
         seenCar=0;
         packetfunctions_reserveHeaderSize(testIntersection,1);
         testIntersection->payload[0] = seenCar;
         packetfunctions_reserveFooterSize(testIntersection,2);//space for radio to fill in CRC
         //send packet(noCar)
         radio_send(testIntersection);
       }
     }
     
     else if (filt_norm>=threshold && FSMcounter >=maxCount){
       state=CAR;
       if(!seenCar){
         seenCar=1;
         packetfunctions_reserveHeaderSize(testIntersection,1);
         testIntersection->payload[0] = seenCar;
         packetfunctions_reserveFooterSize(testIntersection,2);//space for radio to fill in CRC
         //send packet(Car)
         radio_send(testIntersection);
       }
     }
     break;
     
   case CAR:
     if (filt_norm < threshold){
       FSMcounter--;
       state = PERHAPS;
     }
     break;
   default:
     break;   
   }
   
}

void stupidmac_sendDone(OpenQueueEntry_t* pkt, error_t error) {
   openqueue_freePacketBuffer(pkt);
   //scheduler_push_task(ID_TASK_APPLICATION);     // ask the scheduler to do run task_application again
}

void radio_packet_received(OpenQueueEntry_t* packetReceived) {
   P1OUT ^= 0x02;                                // toggle P1.1 for debug
   leds_circular_shift();                        // circular-shift LEDs for debug
   openqueue_freePacketBuffer(packetReceived);
}

//======= INT timer fires

int mul_int(int x, int y)           //    routine for multiplying two integers
{
      return(x*y);
}

int div_int(int x, int y)                  //  routine for dividing two integers
{
      return(x/y);
}

float LPF(long in)
{
  //stage 0, type 1
  out[0] = delay[0] - in;
  delay[0] += -out[0]*alpha[0];
  
  //stage 4, type 1
  out[4] = delay[4] - delay[3];
  delay[4] += -out[4]*alpha[4];
  
  //stage3, type 3
  out[3] = -out[4] + (out[0] - out[4])*alpha[3];
  delay[3] = out[3] + (out[4] - out[0]);
  
  //stage 8, type 1
  out[8] = delay[8] - delay[7];
  delay[8] += -out[8]*alpha[8];
  
  //stage 7, type 4
  out[7] = out[3] - out[8];
  delay[7] = -out[8] -out[7]*alpha[7];
  
  //stage 2, type 1
  out[2] = delay[2] - delay[1];
  delay[2] += -out[2]*alpha[2];
  
  //stage 1, type 3
  out[1] = -out[2] + (in - out[2])*alpha[1];
  delay[1] = out[1] + (out[2] - in);
  
  //stage 6, type 1
  out[6] = delay[6] - delay[5];
  delay[6] += out[6]*alpha[6];
  
  //stage 5, type 4
  out[5] = out[1] - out[6];
  delay[5] = -out[6] -out[5]*alpha[5];
  
  return (0.5*(out[7] + out[5]));
  
}