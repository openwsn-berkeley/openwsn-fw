/**
\brief A CoAP resource which indicates the board its running on.
*/

#include "config.h"

#if OPENWSN_CINFO_C

#include "opendefs.h"
#include "cinfo.h"
#include "coap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "board.h"
#include "idmanager.h"

//=========================== defines =========================================

const uint8_t cinfo_path0[] = "i";

//=========================== variables =======================================

cinfo_vars_t cinfo_vars;

//=========================== prototypes ======================================

owerror_t cinfo_receive(
        OpenQueueEntry_t *msg,
        coap_header_iht *coap_header,
        coap_option_iht *coap_incomingOptions,
        coap_option_iht *coap_outgoingOptions,
        uint8_t *coap_outgoingOptionsLen
);

void cinfo_sendDone(
        OpenQueueEntry_t *msg,
        owerror_t error
);

//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void cinfo_init(void) {
    // do not run if DAGroot
    if (idmanager_getIsDAGroot() == TRUE) return;

    // prepare the resource descriptor for the /i path
    cinfo_vars.desc.path0len = sizeof(cinfo_path0) - 1;
    cinfo_vars.desc.path0val = (uint8_t * )(&cinfo_path0);
    cinfo_vars.desc.path1len = 0;
    cinfo_vars.desc.path1val = NULL;
    cinfo_vars.desc.componentID = COMPONENT_CINFO;
    cinfo_vars.desc.securityContext = NULL;
    cinfo_vars.desc.discoverable = TRUE;
    cinfo_vars.desc.callbackRx = &cinfo_receive;
    cinfo_vars.desc.callbackSendDone = &cinfo_sendDone;

    // register with the CoAP module
    coap_register(&cinfo_vars.desc);
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
owerror_t cinfo_receive(
        OpenQueueEntry_t *msg,
        coap_header_iht *coap_header,
        coap_option_iht *coap_incomingOptions,
        coap_option_iht *coap_outgoingOptions,
        uint8_t *coap_outgoingOptionsLen
) {
    owerror_t outcome;

    switch (coap_header->Code) {
        case COAP_CODE_REQ_GET:
            //=== reset packet payload (we will reuse this packetBuffer)
            msg->payload = &(msg->packet[127]);
            msg->length = 0;

            //=== prepare  CoAP response

            // radio name
            if (packetfunctions_reserveHeader(&msg, sizeof(infoRadioName) - 1) == E_FAIL) {
                openqueue_freePacketBuffer(msg);
                return E_FAIL;
            }
            memcpy(&msg->payload[0], &infoRadioName, sizeof(infoRadioName) - 1);

            // uC name
            if (packetfunctions_reserveHeader(&msg, 1) == E_FAIL) {
                openqueue_freePacketBuffer(msg);
                return E_FAIL;
            }
            msg->payload[0] = '\n';

            if (packetfunctions_reserveHeader(&msg, sizeof(infouCName) - 1) == E_FAIL) {
                openqueue_freePacketBuffer(msg);
                return E_FAIL;
            }
            memcpy(&msg->payload[0], &infouCName, sizeof(infouCName) - 1);

            // board name
            if (packetfunctions_reserveHeader(&msg, 1) == E_FAIL) {
                openqueue_freePacketBuffer(msg);
                return E_FAIL;
            }
            msg->payload[0] = '\n';

            if (packetfunctions_reserveHeader(&msg, sizeof(infoBoardname) - 1) == E_FAIL) {
                openqueue_freePacketBuffer(msg);
                return E_FAIL;
            }
            memcpy(&msg->payload[0], &infoBoardname, sizeof(infoBoardname) - 1);

            // stack name and version
            if (packetfunctions_reserveHeader(&msg, 1) == E_FAIL) {
                openqueue_freePacketBuffer(msg);
                return E_FAIL;
            }
            msg->payload[0] = '\n';

            if (packetfunctions_reserveHeader(&msg, sizeof(infoStackName)) == E_FAIL) {
                openqueue_freePacketBuffer(msg);
                return E_FAIL;
            }
            memcpy(&msg->payload[0], &infoStackName, sizeof(infoStackName) - 1);

            msg->payload[sizeof(infoStackName) - 1 + 6 - 6] = '0' + OPENWSN_VERSION_MAJOR;
            msg->payload[sizeof(infoStackName) - 1 + 6 - 5] = '.';
            msg->payload[sizeof(infoStackName) - 1 + 6 - 4] = '0' + OPENWSN_VERSION_MINOR / 10;
            msg->payload[sizeof(infoStackName) - 1 + 6 - 3] = '0' + OPENWSN_VERSION_MINOR % 10;
            msg->payload[sizeof(infoStackName) - 1 + 6 - 2] = '.';
            msg->payload[sizeof(infoStackName) - 1 + 6 - 1] = '0' + OPENWSN_VERSION_PATCH;

            // set the CoAP header
            coap_header->Code = COAP_CODE_RESP_CONTENT;

            outcome = E_SUCCESS;
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
void cinfo_sendDone(OpenQueueEntry_t *msg, owerror_t error) {
    openqueue_freePacketBuffer(msg);
}

#endif /* OPENWSN_CINFO_C */
