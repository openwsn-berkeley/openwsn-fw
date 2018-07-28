/**
\brief A protected CoAP resource, accessed over a dynamically-created OSCORE channel.
*/

#include "opendefs.h"
#include "cprotected.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "board.h"
#include "idmanager.h"
#include "icmpv6rpl.h"
#include "cborencoder.h"

//=========================== defines =========================================

const uint8_t cprotected_path0[] = "resource1";

//=========================== variables =======================================

cprotected_vars_t cprotected_vars;

//=========================== prototypes ======================================

owerror_t cprotected_receive(
        OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen
);
void cprotected_sendDone(
        OpenQueueEntry_t* msg,
        owerror_t error
);

uint8_t cprotected_get_as_info(uint8_t* buf);

//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void cprotected_init(void) {
   // do not run if DAGroot
   if(idmanager_getIsDAGroot()==TRUE) return;

   // prepare the resource descriptor
   cprotected_vars.desc.path0len             = sizeof(cprotected_path0)-1;
   cprotected_vars.desc.path0val             = (uint8_t*)(&cprotected_path0);
   cprotected_vars.desc.path1len             = 0;
   cprotected_vars.desc.path1val             = NULL;
   cprotected_vars.desc.componentID          = COMPONENT_CPROTECTED;
   cprotected_vars.desc.securityContext      = NULL;
   cprotected_vars.desc.discoverable         = TRUE;
   cprotected_vars.desc.callbackRx           = &cprotected_receive;
   cprotected_vars.desc.callbackSendDone     = &cprotected_sendDone;

   // register with the CoAP module
   opencoap_register(&cprotected_vars.desc);
}

//=========================== private =========================================

/**
\brief Called when a CoAP message is received for this resource.

\param[in] msg          The received message. CoAP header and options already
   parsed.
\param[in] coap_header  The CoAP header contained in the message.
\param[in] coap_options The CoAP options contained in the message.

\return Whether the response is prepared successfully.
*/
owerror_t cprotected_receive(
        OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen
) {
    owerror_t outcome;
    uint8_t i;
    bool found;
    uint8_t asInfo[60];    // buffer holding CBOR serialization of AS info object
    uint8_t asInfoLen;
    const uint8_t text[] = " __ _  ______\n(_ |_)/ \\|(_\n__)|  \\_/|__)";

    i = 0;
    asInfoLen = 0;
    found = FALSE;

    while (coap_incomingOptions[i].type != COAP_OPTION_NONE) {
        if (coap_incomingOptions[i].type == COAP_OPTION_NUM_OBJECTSECURITY) {
            found = TRUE;
        }
        i++;
    }

    // if received over unprotected channel (no OSCORE), return unauthorized
    if (!found) {
        //=== reset packet payload (we will reuse this packetBuffer)
        msg->payload                     = &(msg->packet[127]);
        msg->length                      = 0;

        //=== prepare  CoAP response
        asInfoLen = cprotected_get_as_info(asInfo);

        if (asInfoLen) {
            packetfunctions_reserveHeaderSize(msg, asInfoLen);
            memcpy(&msg->payload[0],asInfo, asInfoLen);

            cprotected_vars.medType = COAP_MEDTYPE_APPCBOR;
            coap_outgoingOptions[0].type = COAP_OPTION_NUM_CONTENTFORMAT;
            coap_outgoingOptions[0].length = 1;
            coap_outgoingOptions[0].pValue = &cprotected_vars.medType;

            *coap_outgoingOptionsLen = 1;
        }

        // set the CoAP header
        coap_header->Code                = COAP_CODE_RESP_UNAUTHORIZED;

        return E_SUCCESS;
    }

    // else: message was received over an OSCORE channel

    switch (coap_header->Code) {
        case COAP_CODE_REQ_GET:
            //=== reset packet payload (we will reuse this packetBuffer)
            msg->payload                     = &(msg->packet[127]);
            msg->length                      = 0;

            // copy payload
            packetfunctions_reserveHeaderSize(msg,sizeof(text)-1);
            memcpy(&msg->payload[0],&text,sizeof(text)-1);

            //=== prepare  CoAP response

            // set the CoAP header
            coap_header->Code                = COAP_CODE_RESP_CONTENT;

            outcome                          = E_SUCCESS;
            break;
        default:
            // return an error message
            outcome = E_FAIL;
    }

   return outcome;
}

/**
\brief The stack indicates that the packet was sent.

\param[in] msg The CoAP message just sent.
\param[in] error The outcome of sending it.
*/
void cprotected_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

uint8_t cprotected_get_as_info(uint8_t* buf) {
    static char hex [] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' ,'a', 'b', 'c', 'd', 'e', 'f' };
    open_addr_t JRCaddress;
    uint8_t i;
    uint8_t len;
    uint16_t dig16b;
    uint8_t uri[39];  // worst case string encoding of IPv6 address
    uint8_t uriLen;
    const uint8_t scheme[] = "coap://";
    const uint8_t token[] = "/token";

    i = 0;
    len = 0;
    uriLen = 0;

    // add JRC's IPv6 address to payload for the Client to obtain an access token
    JRCaddress.type = ADDR_128B;
    if (icmpv6rpl_getRPLDODAGid(JRCaddress.addr_128b) == E_SUCCESS) {

        // create an absolute URI string

        // Step 1. Add scheme "coap://
        memcpy(&uri[uriLen], scheme, sizeof(scheme)-1);
        uriLen += sizeof(scheme)-1;

        // Step 2. Add IPv6 address within []
        uri[uriLen++] = '[';
        // convert IPv6 byte string to hex string
        for (i = 0; i < 16; i = i+2) {

            dig16b = packetfunctions_ntohs(&JRCaddress.addr_128b[i]);

            if (i != 0) {
                uri[uriLen++] = ':';
            }

            if (dig16b) {
                uri[uriLen++] = hex[(0xf0 & JRCaddress.addr_128b[i]) >> 4];
                uri[uriLen++] = hex[(0x0f & JRCaddress.addr_128b[i]) >> 0];
                uri[uriLen++] = hex[(0xf0 & JRCaddress.addr_128b[i+1]) >> 4];
                uri[uriLen++] = hex[(0x0f & JRCaddress.addr_128b[i+1]) >> 0];
            } else {
                uri[uriLen++] = '0';
            }
        }
        uri[uriLen++] = ']';

        // Step 3. Add URI-Path of the /token resource at AS
        memcpy(&uri[uriLen], token, sizeof(token)-1);
        uriLen += sizeof(token)-1;

        // Now, create CBOR AS Info object
        len += cborencoder_put_map(&buf[len], 1);
        len += cborencoder_put_unsigned(&buf[len], (uint8_t) AS_INFO_LABEL_AS);
        len += cborencoder_put_text(&buf[len], (char *) uri, uriLen);

        return len;
    }

    return 0;
}

