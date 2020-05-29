/**
\brief A CoAP resource which allows an application to GET/SET the state of the
   error LED.
*/

#include "config.h"

#if defined(OPENWSN_CLED_C)

#include "opendefs.h"
#include "cled.h"
#include "coap.h"
#include "packetfunctions.h"
#include "leds.h"
#include "openqueue.h"

//=========================== variables =======================================

cled_vars_t cled_vars;

const uint8_t cled_path0[] = "l";

//=========================== prototypes ======================================

owerror_t cled_receive(OpenQueueEntry_t *msg,
                       coap_header_iht *coap_header,
                       coap_option_iht *coap_incomingOptions,
                       coap_option_iht *coap_outgoingOptions,
                       uint8_t *coap_outgoingOptionsLen);

void cled_sendDone(OpenQueueEntry_t *msg, owerror_t error);

//=========================== public ==========================================

void cled_init(void) {

    // prepare the resource descriptor for the /l path
    cled_vars.desc.path0len = sizeof(cled_path0) - 1;
    cled_vars.desc.path0val = (uint8_t * )(&cled_path0);
    cled_vars.desc.path1len = 0;
    cled_vars.desc.path1val = NULL;
    cled_vars.desc.componentID = COMPONENT_CLEDS;
    cled_vars.desc.securityContext = NULL;
    cled_vars.desc.discoverable = TRUE;
    cled_vars.desc.callbackRx = &cled_receive;
    cled_vars.desc.callbackSendDone = &cled_sendDone;

    // register with the CoAP module
    coap_register(&cled_vars.desc);
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
owerror_t cled_receive(OpenQueueEntry_t *msg,
                       coap_header_iht *coap_header,
                       coap_option_iht *coap_incomingOptions,
                       coap_option_iht *coap_outgoingOptions,
                       uint8_t *coap_outgoingOptionsLen) {
    owerror_t outcome;

    switch (coap_header->Code) {
        case COAP_CODE_REQ_GET:
            // reset packet payload
            msg->payload = &(msg->packet[127]);
            msg->length = 0;

            // add CoAP payload
            packetfunctions_reserveHeaderSize(msg, 1);

            if (leds_error_isOn() == 1) {
                msg->payload[0] = '1';
            } else {
                msg->payload[0] = '0';
            }

            // set the CoAP header
            coap_header->Code = COAP_CODE_RESP_CONTENT;

            outcome = E_SUCCESS;
            break;

        case COAP_CODE_REQ_PUT:

            // change the LED's state
            if (msg->payload[0] == '1') {
                leds_error_on();
            } else if (msg->payload[0] == '2') {
                leds_error_toggle();
            } else {
                leds_error_off();
            }

            // reset packet payload
            msg->payload = &(msg->packet[127]);
            msg->length = 0;

            // set the CoAP header
            coap_header->Code = COAP_CODE_RESP_CHANGED;

            outcome = E_SUCCESS;
            break;

        default:
            outcome = E_FAIL;
            break;
    }

    return outcome;
}

/**
\brief The stack indicates that the packet was sent.

\param[in] msg The CoAP message just sent.
\param[in] error The outcome of sending it.
*/
void cled_sendDone(OpenQueueEntry_t *msg, owerror_t error) {
    openqueue_freePacketBuffer(msg);
}

#endif /* OPENWSN_CLED_C */
