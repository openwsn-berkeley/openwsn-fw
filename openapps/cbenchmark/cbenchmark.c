/**
\brief App that generates packets as instructed over serial.
*/

#include "opendefs.h"
#include "opencoap.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"
#include "idmanager.h"
#include "eui64.h"
#include "neighbors.h"
#include "cbenchmark.h"

//=========================== defines =========================================

const uint8_t cbenchmark_path0[] = "b";

//=========================== variables =======================================

cbenchmark_vars_t cbenchmark_vars;

//=========================== prototypes ======================================

owerror_t cbenchmark_receive(OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen);
void cbenchmark_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void cbenchmark_sendPacket(open_addr_t *, bool, uint8_t, uint8_t*, uint8_t, uint8_t);

//=========================== public ==========================================

void cbenchmark_init() {
   // prepare the resource descriptor for the /b path
   cbenchmark_vars.desc.path0len                        = sizeof(cbenchmark_path0)-1;
   cbenchmark_vars.desc.path0val                        = (uint8_t*)(&cbenchmark_path0);
   cbenchmark_vars.desc.path1len                        = 0;
   cbenchmark_vars.desc.path1val                        = NULL;
   cbenchmark_vars.desc.componentID                     = COMPONENT_CBENCHMARK;
   cbenchmark_vars.desc.securityContext                 = NULL;
   cbenchmark_vars.desc.discoverable                    = TRUE;
   cbenchmark_vars.desc.callbackRx                      = &cbenchmark_receive;
   cbenchmark_vars.desc.callbackSendDone                = &cbenchmark_sendDone;

   cbenchmark_vars.noResponse                           = 26; // RFC7967 flag to suppress all responses

   opencoap_register(&cbenchmark_vars.desc);

   // Register sendPacket handler callback
   openserial_registerSendPacketCb(&cbenchmark_sendPacket);

   cbenchmark_vars.timerId = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_COAP);

}

//=========================== private =========================================
owerror_t cbenchmark_receive(OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen) {

    // TODO Parse the OpenBenchmark token in the payload
    // TODO log packet received event

    // in case the request contains a No Response option, it will be handled by the
    // underlying CoAP lib
    return E_SUCCESS;
}

void cbenchmark_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
    openqueue_freePacketBuffer(msg);
}

void cbenchmark_sendPacket(open_addr_t *dest, bool con, uint8_t numPackets, uint8_t* token, uint8_t tokenLen, uint8_t payloadLen) {

    OpenQueueEntry_t*            pkt;
    coap_option_iht              options[1];
    owerror_t                    outcome;
    uint8_t                      numOptions;

    numOptions = 0;

    // create a CoAP packet
    pkt = openqueue_getFreePacketBuffer(COMPONENT_CBENCHMARK);
    if (pkt==NULL) {
        openserial_printError(
            COMPONENT_CBENCHMARK,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        // TODO log error
    }

    // take ownership over that packet
    pkt->creator                   = COMPONENT_CBENCHMARK;
    pkt->owner                     = COMPONENT_CBENCHMARK;

    // location-path option
    options[numOptions].type = COAP_OPTION_NUM_URIPATH;
    options[numOptions].length = sizeof(cbenchmark_path0)-1;
    options[numOptions].pValue = (uint8_t *)cbenchmark_path0;
    numOptions++;

    // metadata
    pkt->l4_destination_port       = WKP_UDP_COAP;
    // construct destination address
    packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), dest, &(pkt->l3_destinationAdd));

    // copy the token
    packetfunctions_reserveHeaderSize(pkt, tokenLen);
    memcpy(pkt->payload, token, tokenLen);

    // zero-out the payload after the token
    packetfunctions_reserveHeaderSize(pkt, payloadLen);
    memset(pkt->payload, 0x00, payloadLen);

    // confirmable semantics of openbenchmark denotes whether there is an application-layer
    // acknowledgment going back. we implement this behavior by sending or not the CoAP response
    if (!con) {
        // to avoid the server generating a response, we use the No Response option specified
        // in RFC7967
        options[numOptions].type = COAP_OPTION_NUM_NORESPONSE;
        options[numOptions].length = 1;
        options[numOptions].pValue = &cbenchmark_vars.noResponse;
        numOptions++;
    }

    // send
    outcome = opencoap_send(
        pkt,
        COAP_TYPE_NON,
        COAP_CODE_REQ_POST,
        1, // token len
        options,
        numOptions,
        &cbenchmark_vars.desc
    );

    // avoid overflowing the queue if fails
    if (outcome==E_FAIL) {
        openqueue_freePacketBuffer(pkt);
        // TODO log error
    }

    // TODO log packet sent event
}

