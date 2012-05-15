#include "openwsn.h"
#include "rex.h"
#include "opencoap.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "scheduler.h"
#include "ADC_Channel.h"
#include <math.h>
#include "debugpins.h"
#include "leds.h"

//=========================== defines =========================================

/// inter-packet period (in ms)
#define REXPERIOD    2000
#define PAYLOADLEN    62

/// ADC Parameters
#define N_SAMPLES     16
#define ADC_PERIOD    10
#define N_RUNNING_AVG 5
#define WATTS_PER     1//85 //52

const uint8_t rex_path0[] = "rex";

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t  timerId;
} rex_vars_t;

rex_vars_t rex_vars;

// ADC vars
uint8_t adc_count;
uint16_t adc_samples[N_SAMPLES];
opentimer_id_t  adc_timerId;
uint8_t timer_running;
//uint32_t adc_run[N_RUNNING_AVG];
uint32_t adc_run;
uint8_t adc_run_cnt;

//=========================== prototypes ======================================

error_t rex_receive(OpenQueueEntry_t* msg,
                    coap_header_iht*  coap_header,
                    coap_option_iht*  coap_options);
void    rex_timer_cb();
void    rex_ADC_timer_cb();
void    rex_ADC_read_cb();
void    rex_task_cb();
void    rex_sendPacket();
void    rex_sendDone(OpenQueueEntry_t* msg,
                       error_t error);
void    rex_start_timer();
void    rex_ADC_start_timer();

//=========================== public ==========================================

void rex_init() {
  
  adc_count = 0;
  timer_running = 0;
   
   // prepare the resource descriptor for the /rex path
   rex_vars.desc.path0len             = sizeof(rex_path0)-1;
   rex_vars.desc.path0val             = (uint8_t*)(&rex_path0);
   rex_vars.desc.path1len             = 0;
   rex_vars.desc.path1val             = NULL;
   rex_vars.desc.componentID          = COMPONENT_REX;
   rex_vars.desc.callbackRx           = &rex_receive;
   rex_vars.desc.callbackSendDone     = &rex_sendDone;
   
   
   opencoap_register(&rex_vars.desc);
   rex_vars.timerId    = opentimers_start(REXPERIOD,
                                                TIMER_PERIODIC,
                                                rex_timer_cb);
}

//=========================== private =========================================

error_t rex_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   return E_FAIL;
}

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void rex_timer_cb(){  //every REXPERIOD
   scheduler_push_task(rex_task_cb,TASKPRIO_COAP);
}

void rex_task_cb(){ //every REXPERIOD
  leds_error_off();  

  //Enable single sampling
   ADC_enable();
   ADC_getvoltage(adc_samples);
   ADC_disable();
   rex_sendPacket();
//   scheduler_push_task(rex_sendPacket_cb,TASKPRIO_COAP);
}

void rex_sendPacket(){
       OpenQueueEntry_t* pkt;
     error_t           outcome;
     uint8_t           numOptions;
     uint8_t           i;
     
     // create a CoAP RD packet
     pkt = openqueue_getFreePacketBuffer();
     if (pkt==NULL) {
       openserial_printError(COMPONENT_REX,ERR_NO_FREE_PACKET_BUFFER,
                             (errorparameter_t)0,
                             (errorparameter_t)0);
       openqueue_freePacketBuffer(pkt);
       return;
     }
     // take ownership over that packet
     pkt->creator    = COMPONENT_REX;
     pkt->owner      = COMPONENT_REX;
     // CoAP payload
     packetfunctions_reserveHeaderSize(pkt,PAYLOADLEN);
     for (i=0;i<PAYLOADLEN;i++) {
       pkt->payload[i] = i;
     }
     
     // Report raw data
     for (i=0;i<N_SAMPLES;i++) {
       pkt->payload[2*i]   = (adc_samples[i]>>8)&0xff;
       pkt->payload[2*i+1] = (adc_samples[i]>>0)&0xff; 
 }
     
     // Report Power
//     uint16_t offset = 2115;
//     uint32_t sum = 0;
//     for (int j = 0; j < N_SAMPLES; j++){
//       if (adc_samples[j] > offset){
//         adc_samples[j] -= offset;
//       }
//       else {
//        adc_samples[j] = offset - adc_samples[j]; 
//       }
//       uint32_t sq = adc_samples[j];
//       sum += sq * sq;
//     }
//     uint32_t avg = sum / N_SAMPLES;
//     
//     if (avg > adc_run){       
//       adc_run += (avg - adc_run) / N_RUNNING_AVG;
//     }
//     else {
//       adc_run -= (adc_run - avg) / N_RUNNING_AVG;
//     }
//     avg = adc_run / WATTS_PER;
//     
//     pkt->payload[0] = (avg>>8)&0xff;
//     pkt->payload[1] = (avg>>0)&0xff; 
     
     
     
     numOptions = 0;
     // location-path option
     packetfunctions_reserveHeaderSize(pkt,sizeof(rex_path0)-1);
     memcpy(&pkt->payload[0],&rex_path0,sizeof(rex_path0)-1);
     packetfunctions_reserveHeaderSize(pkt,1);
     pkt->payload[0]                  = (COAP_OPTION_LOCATIONPATH-COAP_OPTION_CONTENTTYPE) << 4 |
       sizeof(rex_path0)-1;
     numOptions++;
     // content-type option
     packetfunctions_reserveHeaderSize(pkt,2);
     pkt->payload[0]                  = COAP_OPTION_CONTENTTYPE << 4 |
       1;
     pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
     numOptions++;
     // metadata
//     pkt->l4_destination_port         = WKP_UDP_COAP;
     pkt->l4_destination_port         = 5126;
     pkt->l3_destinationORsource.type = ADDR_128B;
     //memcpy(&pkt->l3_destinationORsource.addr_128b[0],&ipAddr_motesEecs,16);
     memcpy(&pkt->l3_destinationORsource.addr_128b[0],&ipAddr_local,16);
     // send
     outcome = opencoap_send(pkt,
                             COAP_TYPE_NON,
                             COAP_CODE_REQ_PUT,
                             numOptions,
                             &rex_vars.desc);
     // avoid overflowing the queue if fails
     if (outcome==E_FAIL) {
       openqueue_freePacketBuffer(pkt);
     }
}

void rex_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}
