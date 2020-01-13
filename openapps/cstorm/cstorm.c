#include "opendefs.h"
#include "cstorm.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"
//#include "ADC_Channel.h"
#include "IEEE802154E.h"
#include "schedule.h"
#include "icmpv6rpl.h"
#include "idmanager.h"

//=========================== defines =========================================

const uint8_t cstorm_path0[]    = "storm";
const uint8_t cstorm_payload[]  = "OpenWSN";
static const uint8_t dst_addr[] = {
   0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

//=========================== variables =======================================

cstorm_vars_t cstorm_vars;

//=========================== prototypes ======================================

owerror_t cstorm_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_incomingOptions,
   coap_option_iht*  coap_outgoingOptions,
   uint8_t*          coap_outgoingOptionsLen);

void cstorm_timer_cb(opentimers_id_t id);
void cstorm_task_cb(void);
void cstorm_sendDone(OpenQueueEntry_t* msg, owerror_t error);

//=========================== public ==========================================

void cstorm_init(void) {

    // register to OpenCoAP module
    cstorm_vars.desc.path0len              = sizeof(cstorm_path0)-1;
    cstorm_vars.desc.path0val              = (uint8_t*)(&cstorm_path0);
    cstorm_vars.desc.path1len              = 0;
    cstorm_vars.desc.path1val              = NULL;
    cstorm_vars.desc.componentID           = COMPONENT_CSTORM;
    cstorm_vars.desc.securityContext       = NULL;
    cstorm_vars.desc.discoverable          = TRUE;
    cstorm_vars.desc.callbackRx            = &cstorm_receive;
    cstorm_vars.desc.callbackSendDone      = &cstorm_sendDone;
    opencoap_register(&cstorm_vars.desc);

    //start a periodic timer
    //comment : not running by default
    /*
    cstorm_vars.period           = 6553;

    cstorm_vars.timerId          = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_COAP);
    opentimers_scheduleIn(
        cstorm_vars.timerId,
        cstorm_vars.period,
        TIME_MS,
        TIMER_PERIODIC,
        cstorm_timer_cb
    );
    */
}

//=========================== private =========================================

owerror_t cstorm_receive(
        OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen
   ) {
   owerror_t outcome;

   switch (coap_header->Code) {

      case COAP_CODE_REQ_GET:

         // reset packet payload
         msg->payload             = &(msg->packet[127]);
         msg->length              = 0;

         // add CoAP payload
         packetfunctions_reserveHeaderSize(msg, 2);
         // return as big endian
         msg->payload[0]          = (uint8_t)(cstorm_vars.period >> 8);
         msg->payload[1]          = (uint8_t)(cstorm_vars.period & 0xff);

         // set the CoAP header
         coap_header->Code        = COAP_CODE_RESP_CONTENT;

         outcome                  = E_SUCCESS;
         break;

      case COAP_CODE_REQ_PUT:

         if (msg->length!=2) {
            outcome               = E_FAIL;
            coap_header->Code     = COAP_CODE_RESP_BADREQ;
         }

         // read the new period
         cstorm_vars.period     = 0;
         cstorm_vars.period    |= (msg->payload[0] << 8);
         cstorm_vars.period    |= msg->payload[1];

         /*
         // stop and start again only if period > 0
         opentimers_cancel(cstorm_vars.timerId);

         if(cstorm_vars.period > 0) {
               opentimers_scheduleIn(
                   cstorm_vars.timerId,
                   cstorm_vars.period,
                   TIME_MS,
                   TIMER_PERIODIC,
                   cstorm_timer_cb
               );
         }
         */

         // reset packet payload
         msg->payload             = &(msg->packet[127]);
         msg->length              = 0;

         // set the CoAP header
         coap_header->Code        = COAP_CODE_RESP_CHANGED;

         outcome                  = E_SUCCESS;
         break;

      default:
         outcome = E_FAIL;
         break;
   }

   return outcome;
}

void cstorm_timer_cb(opentimers_id_t id) {
    // calling the task directly as the timer_cb function is executed in
    // task mode by opentimer already
    cstorm_task_cb();
}

void cstorm_task_cb(void) {
    OpenQueueEntry_t*    pkt;
    owerror_t            outcome;
    coap_option_iht      options[2];
    uint8_t              medType;

    open_addr_t          parentNeighbor;
    bool                 foundNeighbor;

    // don't run if not synch
    if (ieee154e_isSynch() == FALSE) {
        return;
    }

    // don't run on dagroot
    if (idmanager_getIsDAGroot()) {
        opentimers_destroy(cstorm_vars.timerId);
        return;
    }

    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&parentNeighbor);
    if (foundNeighbor==FALSE) {
        return;
    }

    if (schedule_hasNegotiatedCellToNeighbor(&parentNeighbor, CELLTYPE_TX) == FALSE) {
        return;
    }

    if (cstorm_vars.busySendingCstorm==TRUE) {
        // don't continue if I'm still sending a previous cstorm
        return;
    }

    if(cstorm_vars.period == 0) {
      // stop the periodic timer
      opentimers_cancel(cstorm_vars.timerId);
      return;
    }

    // if you get here, send a packet

    // get a packet
    pkt = openqueue_getFreePacketBuffer(COMPONENT_CSTORM);
    if (pkt==NULL) {
        openserial_printError(COMPONENT_CSTORM,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
        return;
    }

    // take ownership over that packet
    pkt->creator    = COMPONENT_CSTORM;
    pkt->owner      = COMPONENT_CSTORM;

    //The contents of the message are written in reverse order : the payload first
    //packetfunctions_reserveHeaderSize moves the index pkt->payload

    // add payload
    packetfunctions_reserveHeaderSize(pkt,sizeof(cstorm_payload)-1);
    memcpy(&pkt->payload[0],cstorm_payload,sizeof(cstorm_payload)-1);

    // location-path option
    options[0].type = COAP_OPTION_NUM_URIPATH;
    options[0].length = sizeof(cstorm_path0) - 1;
    options[0].pValue = (uint8_t *) cstorm_path0;


    // content-type option
    medType = COAP_MEDTYPE_APPOCTETSTREAM;
    options[1].type = COAP_OPTION_NUM_CONTENTFORMAT;
    options[1].length = 1;
    options[1].pValue = &medType;

    // metadata
    pkt->l4_destination_port = WKP_UDP_COAP;
    pkt->l3_destinationAdd.type = ADDR_128B;
    memcpy(&pkt->l3_destinationAdd.addr_128b[0],&dst_addr,16);

    // send
    outcome = opencoap_send(
        pkt,
        COAP_TYPE_NON,
        COAP_CODE_REQ_PUT,
        1, // token len
        options,
        2, // options len
        &cstorm_vars.desc
    );

    // avoid overflowing the queue if fails
    if (outcome==E_FAIL) {
        openqueue_freePacketBuffer(pkt);
    } else {
        cstorm_vars.busySendingCstorm=FALSE;
    }
}

void cstorm_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
    // free the packet buffer entry
    openqueue_freePacketBuffer(msg);

    // allow to send next cstorm packet
    cstorm_vars.busySendingCstorm=FALSE;
}






