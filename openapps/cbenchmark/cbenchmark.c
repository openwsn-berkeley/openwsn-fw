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
#include "IEEE802154E.h"
#include "iphc.h"

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
void cbenchmark_sendPacket(uint8_t *, uint8_t);
owerror_t cbenchmark_parse_sendPacket(uint8_t *buf, uint8_t bufLen, cbenchmark_sendPacket_t *request);
void cbenchmark_printSerial_packetSentReceived(uint8_t eventType, uint8_t *timestamp, uint8_t *packetToken, open_addr_t *dest, uint8_t hopLimit);

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

    uint8_t   token[5];
    uint8_t   timestamp[5];
    open_addr_t source;
    open_addr_t prefix; // dummy

    // get the current ASN as soon as possible
    ieee154e_getAsn(timestamp);

    // get the source of the packet
    packetfunctions_ip128bToMac64b(&msg->l3_sourceAdd, &prefix, &source);

    // check if this is a request
    if (
         coap_header->Code==COAP_CODE_REQ_POST) {
        // sanity check
        if (msg->length < 5) {
            // log formatting error
                openserial_printError(
                    COMPONENT_CBENCHMARK,
                    ERR_MSG_UNKNOWN_TYPE,
                    (errorparameter_t)msg->length,
                    (errorparameter_t)msg->length
                );
                return E_FAIL;
        }

        // get the token from the first 5 bytes of payload
        memcpy(token, msg->payload, 5);

        // in case the request contains a No Response option, it will be handled by the
        // underlying CoAP lib
        cbenchmark_printSerial_packetSentReceived(STATUS_BENCHMARK_PACKETRECEIVED, timestamp, token, &source, msg->l3_hopLimit);
        // reset packet payload
        msg->payload                     = &(msg->packet[127]);
        msg->length                      = 0;
        // set the response CoAP header
        coap_header->Code                = COAP_CODE_RESP_CHANGED;
    }
    else { // else this is a response
        // FIXME should this packet be logged?
    }
    return E_SUCCESS;
}

void cbenchmark_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
    openqueue_freePacketBuffer(msg);
}

void cbenchmark_sendPacket(uint8_t *buf, uint8_t bufLen) {

    OpenQueueEntry_t*            pkt;
    cbenchmark_sendPacket_t      request;
    coap_option_iht              options[2];
    owerror_t                    outcome;
    uint8_t                      numOptions;
    uint8_t                      i;
    uint8_t                      timestamp[5];

    numOptions = 0;
    i = 0;

    if (cbenchmark_parse_sendPacket(buf, bufLen, &request) != E_SUCCESS) {
        return;
    };


    for (i = 0; i < request.numPackets; i++) {
        // create a CoAP packet
        pkt = openqueue_getFreePacketBuffer(COMPONENT_CBENCHMARK);
        if (pkt==NULL) {
            openserial_printError(
                COMPONENT_CBENCHMARK,
                ERR_NO_FREE_PACKET_BUFFER,
                (errorparameter_t)0,
                (errorparameter_t)0
            );
            return;
        }

        // take ownership over that packet
        pkt->creator                   = COMPONENT_CBENCHMARK;
        pkt->owner                     = COMPONENT_CBENCHMARK;

        // location-path option
        options[numOptions].type = COAP_OPTION_NUM_URIPATH;
        options[numOptions].length = sizeof(cbenchmark_path0)-1;
        options[numOptions].pValue = (uint8_t *)cbenchmark_path0;
        numOptions++;

        // confirmable semantics of openbenchmark denotes whether there is an application-layer
        // acknowledgment going back. we implement this behavior by sending or not the CoAP response
        if (!request.con) {
            // to avoid the server generating a response, we use the No Response option specified
            // in RFC7967
            options[numOptions].type = COAP_OPTION_NUM_NORESPONSE;
            options[numOptions].length = 1;
            options[numOptions].pValue = &cbenchmark_vars.noResponse;
            numOptions++;
        }

        // metadata
        pkt->l4_destination_port       = CBENCHMARK_OPENBENCHMARK_COAP_PORT;
        // construct destination address
        packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), &request.dest, &(pkt->l3_destinationAdd));

        // zero-out the payload
        packetfunctions_reserveHeaderSize(pkt, request.payloadLen);
        memset(pkt->payload, 0x00, request.payloadLen);

        // copy the token as given by OpenBenchmark
        packetfunctions_reserveHeaderSize(pkt, CBENCHMARK_PACKETTOKEN_LEN);
        memcpy(pkt->payload, &request.token, CBENCHMARK_PACKETTOKEN_LEN);

        // set the first byte of the token to packet counter
        pkt->payload[0] = i;

        // get the current ASN
        ieee154e_getAsn(timestamp);

        // send
        outcome = opencoap_send(
            pkt,
            COAP_TYPE_NON,
            COAP_CODE_REQ_POST,
            1, // CoAP token len
            options,
            numOptions,
            &cbenchmark_vars.desc
        );

        // avoid overflowing the queue if fails
        if (outcome==E_FAIL) {
            openqueue_freePacketBuffer(pkt);
            openserial_printError(
                COMPONENT_CBENCHMARK,
                ERR_NO_FREE_PACKET_BUFFER,
                (errorparameter_t)0,
                (errorparameter_t)0
            );
        }
        else {
            cbenchmark_printSerial_packetSentReceived(STATUS_BENCHMARK_PACKETSENT, timestamp, request.token, &request.dest, IPHC_DEFAULT_HOP_LIMIT);
        }
    }
}

owerror_t cbenchmark_parse_sendPacket(uint8_t *buf, uint8_t bufLen, cbenchmark_sendPacket_t *request) {

    uint8_t *tmp;

    if (bufLen != 16) {
        return E_FAIL;
    }

    tmp = buf;

    // parse the command payload
    // EUI64 (8B) || CON (1B) || NUMPACKETS (1B) || TOKEN (4B) || PAYLOADLEN (1B)
    packetfunctions_readAddress(buf, ADDR_64B, &request->dest, FALSE);
    tmp += 8;  // skip 8 bytes for EUI-64

    request->con = (bool) *tmp;
    tmp++; // skip 1 byte for CON

    request->numPackets = *tmp;
    tmp++; // skip 1 byte for number of packets in the burst

    memcpy(request->token, tmp, CBENCHMARK_PACKETTOKEN_LEN);
    tmp += CBENCHMARK_PACKETTOKEN_LEN; // skip token len bytes for token

    request->payloadLen = *tmp;
    tmp++;

    return E_SUCCESS;
}

void cbenchmark_printSerial_packetSentReceived(uint8_t eventType, uint8_t *timestamp, uint8_t *packetToken, open_addr_t *dest, uint8_t hopLimit) {
    uint8_t output[5 + CBENCHMARK_PACKETTOKEN_LEN + LENGTH_ADDR64b + 1];
    uint8_t *tmp;

    tmp = output;

    memcpy(tmp, timestamp, 5);
    tmp += 5;

    memcpy(tmp, packetToken, CBENCHMARK_PACKETTOKEN_LEN);
    tmp += CBENCHMARK_PACKETTOKEN_LEN;

    memcpy(tmp, dest->addr_64b, LENGTH_ADDR64b);
    tmp += LENGTH_ADDR64b;

    *tmp = hopLimit;
    tmp++;

    openserial_printStatus(eventType, output, tmp - output);
}

