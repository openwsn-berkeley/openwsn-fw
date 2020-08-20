/**
\brief CoAP infrared application.

\author Tengfei Chang <tengfei.chang@inria.fr>, December, 2017
*/

#include "config.h"

#if OPENWSN_CINFRARED_C

#include "opendefs.h"
#include "cinfrared.h"
#include "idmanager.h"
#include "openqueue.h"
#include "neighbors.h"
#include "pwm.h"
#include "opentimers.h"
#include "scheduler.h"

//=========================== defines =========================================

const uint8_t cinfrared_path0[] = "ir";

//=========================== variables =======================================

cinfrared_vars_t cinfrared_vars;

//=========================== prototypes ======================================

owerror_t cinfrared_receive(OpenQueueEntry_t *msg,
                            coap_header_iht *coap_header,
                            coap_option_iht *coap_incomingOptions,
                            coap_option_iht *coap_outgoingOptions,
                            uint8_t *coap_outgoingOptionsLen
);

void cinfrared_sendDone(
        OpenQueueEntry_t *msg,
        owerror_t error
);

void cinrared_turnOnOrOff(uint8_t turnOnOrOff);

//=========================== public ==========================================

void cinfrared_init(void) {
    if (idmanager_getIsDAGroot() == TRUE) return;

    // prepare the resource descriptor for the /6t path
    cinfrared_vars.desc.path0len = sizeof(cinfrared_path0) - 1;
    cinfrared_vars.desc.path0val = (uint8_t * )(&cinfrared_path0);
    cinfrared_vars.desc.path1len = 0;
    cinfrared_vars.desc.path1val = NULL;
    cinfrared_vars.desc.componentID = COMPONENT_CINFRARED;
    cinfrared_vars.desc.securityContext = NULL;
    cinfrared_vars.desc.discoverable = TRUE;
    cinfrared_vars.desc.callbackRx = &cinfrared_receive;
    cinfrared_vars.desc.callbackSendDone = &cinfrared_sendDone;

    coap_register(&cinfrared_vars.desc);

    cinfrared_vars.timerId = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_COAP);
}

//=========================== private =========================================

/**
\brief Receives a command and a list of items to be used by the command.

\param[in] msg          The received message. CoAP header and options already
   parsed.
\param[in] coap_header  The CoAP header contained in the message.
\param[in] coap_options The CoAP options contained in the message.

\return Whether the response is prepared successfully.
*/
owerror_t cinfrared_receive(OpenQueueEntry_t *msg,
                            coap_header_iht *coap_header,
                            coap_option_iht *coap_incomingOptions,
                            coap_option_iht *coap_outgoingOptions,
                            uint8_t *coap_outgoingOptionsLen
) {
    owerror_t outcome;

    switch (coap_header->Code) {
        case COAP_CODE_REQ_PUT:

            cinfrared_vars.state = APP_STATE_START;
            // change the infrared's state
            cinrared_turnOnOrOff(msg->payload[0]);

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

void cinfrared_sendDone(OpenQueueEntry_t *msg, owerror_t error) {
    openqueue_freePacketBuffer(msg);
}

void cinrared_turnOnOrOff(uint8_t turnOnOrOff) {
    switch (cinfrared_vars.state) {
        case APP_STATE_START:
            pwm_enable();
            cinfrared_vars.startOfSlot = opentimers_getCurrentCompareValue();
            if (turnOnOrOff) {
                opentimers_scheduleAbsolute(
                        cinfrared_vars.timerId,
                        TURNON_STATE_1,
                        cinfrared_vars.startOfSlot,
                        TIME_TICS,
                        cinrared_turnOnOrOff
                );
            } else {
                opentimers_scheduleAbsolute(
                        cinfrared_vars.timerId,
                        TURNOFF_STATE_1,
                        cinfrared_vars.startOfSlot,
                        TIME_TICS,
                        cinrared_turnOnOrOff
                );
            }
            break;
        case APP_STATE_1:
            pwm_disable();
            if (turnOnOrOff) {
                opentimers_scheduleAbsolute(
                        cinfrared_vars.timerId,
                        TURNON_STATE_2,
                        cinfrared_vars.startOfSlot,
                        TIME_TICS,
                        cinrared_turnOnOrOff
                );
            } else {
                opentimers_scheduleAbsolute(
                        cinfrared_vars.timerId,
                        TURNOFF_STATE_2,
                        cinfrared_vars.startOfSlot,
                        TIME_TICS,
                        cinrared_turnOnOrOff
                );
            }
            break;
        case APP_STATE_2:
            pwm_enable();
            if (turnOnOrOff) {
                opentimers_scheduleAbsolute(
                        cinfrared_vars.timerId,
                        TURNON_STATE_3,
                        cinfrared_vars.startOfSlot,
                        TIME_TICS,
                        cinrared_turnOnOrOff
                );
            } else {
                opentimers_scheduleAbsolute(
                        cinfrared_vars.timerId,
                        TURNOFF_STATE_3,
                        cinfrared_vars.startOfSlot,
                        TIME_TICS,
                        cinrared_turnOnOrOff
                );
            }
            break;
        case APP_STATE_3:
            pwm_disable();
            if (turnOnOrOff) {
                opentimers_scheduleAbsolute(
                        cinfrared_vars.timerId,
                        TURNON_STATE_4,
                        cinfrared_vars.startOfSlot,
                        TIME_TICS,
                        cinrared_turnOnOrOff
                );
            } else {
                opentimers_scheduleAbsolute(
                        cinfrared_vars.timerId,
                        TURNOFF_STATE_4,
                        cinfrared_vars.startOfSlot,
                        TIME_TICS,
                        cinrared_turnOnOrOff
                );
            }
            break;
        case APP_STATE_4:
            pwm_enable();
            if (turnOnOrOff) {
                opentimers_scheduleAbsolute(
                        cinfrared_vars.timerId,
                        TURNON_STATE_END,
                        cinfrared_vars.startOfSlot,
                        TIME_TICS,
                        cinrared_turnOnOrOff
                );
            } else {
                opentimers_scheduleAbsolute(
                        cinfrared_vars.timerId,
                        TURNOFF_STATE_END,
                        cinfrared_vars.startOfSlot,
                        TIME_TICS,
                        cinrared_turnOnOrOff
                );
            }
            break;
        case APP_STATE_END:
            pwm_disable();
            break;
    }

    if (cinfrared_vars.state == APP_STATE_END) {
        cinfrared_vars.state = APP_STATE_START;
    } else {
        cinfrared_vars.state++;
    }
}

#endif /* OPENWSN_CINFRARED_C */
