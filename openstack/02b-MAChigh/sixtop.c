#include "opendefs.h"
#include "sixtop.h"
#include "openserial.h"
#include "openqueue.h"
#include "neighbors.h"
#include "IEEE802154E.h"
#include "frag.h"
#include "packetfunctions.h"
#include "openrandom.h"
#include "scheduler.h"
#include "opentimers.h"
#include "debugpins.h"
#include "leds.h"
#include "IEEE802154.h"
#include "IEEE802154_security.h"
#include "idmanager.h"
#include "schedule.h"
#include "msf.h"

//=========================== define ==========================================

// in seconds: sixtop maintaince is called every 30 seconds
#define MAINTENANCE_PERIOD        30
/**
 Drop the 6P request if number of 6P response with RC RESET in queue is larger
    than MAX6PRESPONSE. Value 0 means that alway drop 6P response when the node
    is in a 6P transcation.
*/
#define MAX6PRESPONSE             1

//=========================== variables =======================================

sixtop_vars_t sixtop_vars;
//=========================== prototypes ======================================

// send internal
owerror_t     sixtop_send_internal(
   OpenQueueEntry_t*    msg,
   bool                 payloadIEPresent
);

// timer interrupt callbacks
void          sixtop_maintenance_timer_cb(opentimers_id_t id);
void          sixtop_timeout_timer_cb(opentimers_id_t id);
void          sixtop_sendingEb_timer_cb(opentimers_id_t id);

//=== EB/KA task

void          timer_sixtop_sendEb_fired(void);
void          timer_sixtop_management_fired(void);
void          sixtop_sendEB(void);
void          sixtop_sendKA(void);

//=== six2six task

void          timer_sixtop_six2six_timeout_fired(void);
void          sixtop_six2six_sendDone(
   OpenQueueEntry_t*    msg,
   owerror_t            error
);
bool          sixtop_processIEs(
   OpenQueueEntry_t*    pkt,
   uint16_t*            lenIE
);
void sixtop_six2six_notifyReceive(
   uint8_t              version,
   uint8_t              type,
   uint8_t              code,
   uint8_t              sfId,
   uint8_t              seqNum,
   uint8_t              ptr,
   uint8_t              length,
   OpenQueueEntry_t*    pkt
);

//=== helper functions

bool          sixtop_addCells(
   uint8_t              slotframeID,
   cellInfo_ht*         cellList,
   open_addr_t*         previousHop,
   uint8_t              cellOptions
);
bool          sixtop_removeCells(
   uint8_t              slotframeID,
   cellInfo_ht*         cellList,
   open_addr_t*         previousHop,
   uint8_t              cellOptions
);
bool          sixtop_areAvailableCellsToBeScheduled(
    uint8_t              frameID,
    uint8_t              numOfCells,
    cellInfo_ht*         cellList
);
bool sixtop_areAvailableCellsToBeRemoved(
    uint8_t      frameID,
    uint8_t      numOfCells,
    cellInfo_ht* cellList,
    open_addr_t* neighbor,
    uint8_t      cellOptions
);

//=========================== public ==========================================

void sixtop_init(void) {

    sixtop_vars.periodMaintenance  = 872 +(openrandom_get16b()&0xff);
    sixtop_vars.busySendingKA      = FALSE;
    sixtop_vars.busySendingEB      = FALSE;
    sixtop_vars.ebCounter          = 0;
    sixtop_vars.dsn                = 0;
    sixtop_vars.mgtTaskCounter     = 0;
    sixtop_vars.kaPeriod           = MAXKAPERIOD;
    sixtop_vars.isResponseEnabled  = TRUE;
    sixtop_vars.six2six_state      = SIX_STATE_IDLE;

    sixtop_vars.ebSendingTimerId   = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_SIXTOP);
    opentimers_scheduleIn(
        sixtop_vars.ebSendingTimerId,
        SLOTFRAME_LENGTH*(SLOTDURATION_MS),
        TIME_MS,
        TIMER_PERIODIC,
        sixtop_sendingEb_timer_cb
    );

    sixtop_vars.maintenanceTimerId   = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_SIXTOP);
    opentimers_scheduleIn(
        sixtop_vars.maintenanceTimerId,
        sixtop_vars.periodMaintenance,
        TIME_MS,
        TIMER_PERIODIC,
        sixtop_maintenance_timer_cb
    );

    sixtop_vars.timeoutTimerId      =  opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_SIXTOP);
}

void sixtop_setKaPeriod(uint16_t kaPeriod) {
    if(kaPeriod > MAXKAPERIOD) {
        sixtop_vars.kaPeriod = MAXKAPERIOD;
    } else {
        sixtop_vars.kaPeriod = kaPeriod;
    }
}

void sixtop_setEBPeriod(uint8_t ebPeriod) {
    if(ebPeriod != 0) {
        // convert parameter to miliseconds
        sixtop_vars.ebPeriod = ebPeriod;
    }
}

void  sixtop_setSFcallback(
    sixtop_sf_getsfid_cbt           cb0,
    sixtop_sf_getmetadata_cbt       cb1,
    sixtop_sf_translatemetadata_cbt cb2,
    sixtop_sf_handle_callback_cbt   cb3
){
   sixtop_vars.cb_sf_getsfid            = cb0;
   sixtop_vars.cb_sf_getMetadata        = cb1;
   sixtop_vars.cb_sf_translateMetadata  = cb2;
   sixtop_vars.cb_sf_handleRCError      = cb3;
}

//======= scheduling

owerror_t sixtop_request(
    uint8_t      code,
    open_addr_t* neighbor,
    uint8_t      numCells,
    uint8_t      cellOptions,
    cellInfo_ht* celllist_toBeAdded,
    cellInfo_ht* celllist_toBeDeleted,
    uint8_t      sfid,
    uint16_t     listingOffset,
    uint16_t     listingMaxNumCells
    ){
    OpenQueueEntry_t* pkt;
    uint8_t           i;
    uint8_t           len;
    uint16_t          length_groupid_type;
    uint8_t           sequenceNumber;
    owerror_t         outcome;

    // filter parameters: handler, status and neighbor
    if(
        sixtop_vars.six2six_state != SIX_STATE_IDLE   ||
        neighbor                  == NULL
    ){
        // neighbor can't be none or previous transcation doesn't finish yet
        return E_FAIL;
    }

    if (openqueue_getNum6PReq(neighbor)>0){
        // remove previous request as it's not sent out
        openqueue_remove6PrequestToNeighbor(neighbor);
    }

    // get a free packet buffer
    pkt = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP_RES);
    if (pkt==NULL) {
        openserial_printError(
            COMPONENT_SIXTOP_RES,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return E_FAIL;
    }

    // take ownership
    pkt->creator = COMPONENT_SIXTOP_RES;
    pkt->owner   = COMPONENT_SIXTOP_RES;

    memcpy(&(pkt->l2_nextORpreviousHop),neighbor,sizeof(open_addr_t));
    if (celllist_toBeDeleted != NULL){
        memcpy(sixtop_vars.celllist_toDelete,celllist_toBeDeleted,CELLLIST_MAX_LEN*sizeof(cellInfo_ht));
    }
    sixtop_vars.cellOptions = cellOptions;

    len  = 0;
    if (
        code == IANA_6TOP_CMD_ADD      ||
        code == IANA_6TOP_CMD_DELETE   ||
        code == IANA_6TOP_CMD_RELOCATE
    ){
        // append 6p celllists
        if (code == IANA_6TOP_CMD_ADD || code == IANA_6TOP_CMD_RELOCATE){
            for(i=0;i<CELLLIST_MAX_LEN;i++) {
                if(celllist_toBeAdded[i].isUsed){
                    packetfunctions_reserveHeaderSize(pkt,4);
                    pkt->payload[0] = (uint8_t)(celllist_toBeAdded[i].slotoffset         & 0x00FF);
                    pkt->payload[1] = (uint8_t)((celllist_toBeAdded[i].slotoffset        & 0xFF00)>>8);
                    pkt->payload[2] = (uint8_t)(celllist_toBeAdded[i].channeloffset      & 0x00FF);
                    pkt->payload[3] = (uint8_t)((celllist_toBeAdded[i].channeloffset     & 0xFF00)>>8);
                    len += 4;
                }
            }
        }
        if (code == IANA_6TOP_CMD_DELETE || code == IANA_6TOP_CMD_RELOCATE){
            for(i=0;i<CELLLIST_MAX_LEN;i++) {
                if(celllist_toBeDeleted[i].isUsed){
                    packetfunctions_reserveHeaderSize(pkt,4);
                    pkt->payload[0] = (uint8_t)(celllist_toBeDeleted[i].slotoffset       & 0x00FF);
                    pkt->payload[1] = (uint8_t)((celllist_toBeDeleted[i].slotoffset      & 0xFF00)>>8);
                    pkt->payload[2] = (uint8_t)(celllist_toBeDeleted[i].channeloffset    & 0x00FF);
                    pkt->payload[3] = (uint8_t)((celllist_toBeDeleted[i].channeloffset   & 0xFF00)>>8);
                    len += 4;
                }
            }
        }
        // append 6p numberCells
        packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
        *((uint8_t*)(pkt->payload)) = numCells;
        len += 1;
    }

    if (code == IANA_6TOP_CMD_LIST){
        // append 6p max number of cells
        packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
        *((uint8_t*)(pkt->payload))   = (uint8_t)(listingMaxNumCells & 0x00FF);
        *((uint8_t*)(pkt->payload+1)) = (uint8_t)(listingMaxNumCells & 0xFF00)>>8;
        len +=2;
        // append 6p listing offset
        packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
        *((uint8_t*)(pkt->payload))   = (uint8_t)(listingOffset & 0x00FF);
        *((uint8_t*)(pkt->payload+1)) = (uint8_t)(listingOffset & 0xFF00)>>8;
        len += 2;
        // append 6p Reserved field
        packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
        *((uint8_t*)(pkt->payload)) = 0;
        len += 1;
    }

    if (code != IANA_6TOP_CMD_CLEAR){
        // append 6p celloptions
        packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
        *((uint8_t*)(pkt->payload)) = cellOptions;
        len+=1;
    } else {
        // record the neighbor in case no response  for clear
        memcpy(&sixtop_vars.neighborToClearCells,neighbor,sizeof(open_addr_t));
        sixtop_vars.neighborRadioToClearCells = (cellOptions & SIXTOP_CELLREQUEST_RADIOSETTING_MASK)>>5;
    }

    // append 6p metadata
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
    pkt->payload[0] = (uint8_t)( sixtop_vars.cb_sf_getMetadata() & 0x00FF);
    pkt->payload[1] = (uint8_t)((sixtop_vars.cb_sf_getMetadata() & 0xFF00)>>8);
    len += 2;

    // append 6p Seqnum and schedule Generation
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
    sequenceNumber              = neighbors_getSequenceNumber(neighbor);
    *((uint8_t*)(pkt->payload)) = sequenceNumber;
    len += 1;

    // append 6p sfid
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
    *((uint8_t*)(pkt->payload)) = sfid;
    len += 1;

    // append 6p code
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
    *((uint8_t*)(pkt->payload)) = code;
    // record the code to determine the action after 6p senddone
    pkt->l2_sixtop_command      = code;
    len += 1;

    // append 6p version, T(type) and  R(reserved)
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
    *((uint8_t*)(pkt->payload)) = IANA_6TOP_6P_VERSION | IANA_6TOP_TYPE_REQUEST;
    len += 1;

    // append 6p subtype id
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
    *((uint8_t*)(pkt->payload)) = IANA_6TOP_SUBIE_ID;
    len += 1;

    // append IETF IE header (length_groupid_type)
    packetfunctions_reserveHeaderSize(pkt, sizeof(uint16_t));
    length_groupid_type  = len;
    length_groupid_type |= (IANA_IETF_IE_GROUP_ID  | IANA_IETF_IE_TYPE);
    pkt->payload[0]      = length_groupid_type        & 0xFF;
    pkt->payload[1]      = (length_groupid_type >> 8) & 0xFF;

    // indicate IEs present
    pkt->l2_payloadIEpresent = TRUE;
    // record this packet as sixtop request message
    pkt->l2_sixtop_messageType    = SIXTOP_CELL_REQUEST;

    // send packet
    outcome = sixtop_send(pkt);

    if (outcome == E_SUCCESS){
        //update states
        switch(code){
        case IANA_6TOP_CMD_ADD:
            sixtop_vars.six2six_state = SIX_STATE_WAIT_ADDREQUEST_SENDDONE;
            break;
        case IANA_6TOP_CMD_DELETE:
            sixtop_vars.six2six_state = SIX_STATE_WAIT_DELETEREQUEST_SENDDONE;
            break;
        case IANA_6TOP_CMD_RELOCATE:
            sixtop_vars.six2six_state = SIX_STATE_WAIT_RELOCATEREQUEST_SENDDONE;
            break;
        case IANA_6TOP_CMD_COUNT:
            sixtop_vars.six2six_state = SIX_STATE_WAIT_COUNTREQUEST_SENDDONE;
            break;
        case IANA_6TOP_CMD_LIST:
            sixtop_vars.six2six_state = SIX_STATE_WAIT_LISTREQUEST_SENDDONE;
            break;
        case IANA_6TOP_CMD_CLEAR:
            sixtop_vars.six2six_state = SIX_STATE_WAIT_CLEARREQUEST_SENDDONE;
            break;
        }
    } else {
        openqueue_freePacketBuffer(pkt);
    }
    return outcome;
}

//======= from upper layer

owerror_t sixtop_send(OpenQueueEntry_t *msg) {

    // set metadata
    msg->owner        = COMPONENT_SIXTOP;
    msg->l2_frameType = IEEE154_TYPE_DATA;

    // set l2-security attributes
    msg->l2_securityLevel   = IEEE802154_security_getSecurityLevel(msg);
    msg->l2_keyIdMode       = IEEE802154_SECURITY_KEYIDMODE;
    msg->l2_keyIndex        = IEEE802154_security_getDataKeyIndex();

    if (msg->l2_payloadIEpresent == FALSE) {
        return sixtop_send_internal(
            msg,
            FALSE
        );
    } else {
        return sixtop_send_internal(
            msg,
            TRUE
        );
    }
}

//======= from lower layer

void task_sixtopNotifSendDone(void) {
    OpenQueueEntry_t* msg;

    // get recently-sent packet from openqueue
    msg = openqueue_sixtopGetSentPacket();
    if (msg==NULL) {
        openserial_printCritical(
            COMPONENT_SIXTOP,
            ERR_NO_SENT_PACKET,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }

    // take ownership
    msg->owner = COMPONENT_SIXTOP;

    // update neighbor statistics
    if (msg->l2_sendDoneError==E_SUCCESS) {
        neighbors_indicateTx(
            &(msg->l2_nextORpreviousHop),
            msg->l2_cellRadioSetting,
            msg->l2_numTxAttempts,
            msg->l2_sendOnTxCell,
            TRUE,
            &msg->l2_asn
        );
    } else {
        neighbors_indicateTx(
             &(msg->l2_nextORpreviousHop),
             msg->l2_cellRadioSetting,
             msg->l2_numTxAttempts,
             msg->l2_sendOnTxCell,
             FALSE,
             &msg->l2_asn
        );
    }

    // send the packet to where it belongs
    switch (msg->creator) {
        case COMPONENT_SIXTOP:
            if (msg->l2_frameType==IEEE154_TYPE_BEACON) {
                
                // this is a EB
                // this decrement should never end up less than 0 if protocol is running correctly.
                sixtop_vars.ebCounter --;
                
                // not busy sending EB anymore
                sixtop_vars.busySendingEB = FALSE;
            } else {
                // this is a KA

                // not busy sending KA anymore
                sixtop_vars.busySendingKA = FALSE;
            }
            // discard packets
            openqueue_freePacketBuffer(msg);
            break;
        case COMPONENT_SIXTOP_RES:
            sixtop_six2six_sendDone(msg,msg->l2_sendDoneError);
            break;
        default:
            // send the rest up the stack
            frag_sendDone(msg,msg->l2_sendDoneError);
            break;
    }
}

void task_sixtopNotifReceive(void) {
    OpenQueueEntry_t* msg;
    uint16_t          lenIE;
    // get received packet from openqueue
    msg = openqueue_sixtopGetReceivedPacket();
    if (msg==NULL) {
        openserial_printCritical(
            COMPONENT_SIXTOP,
            ERR_NO_RECEIVED_PACKET,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }

    // take ownership
    msg->owner = COMPONENT_SIXTOP;

    // update neighbor statistics
    neighbors_indicateRx(
        &(msg->l2_nextORpreviousHop),
        msg->l2_cellRadioSetting,
        msg->l1_rssi,
        &msg->l2_asn,
        msg->l2_joinPriorityPresent,
        msg->l2_joinPriority,
        msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_NOSEC ? TRUE : FALSE
    );

    // process the header IEs
    lenIE=0;
    if(
        msg->l2_frameType              == IEEE154_TYPE_DATA  &&
        msg->l2_payloadIEpresent       == TRUE               &&
        sixtop_processIEs(msg, &lenIE) == FALSE
    ) {
        // free the packet's RAM memory
        openqueue_freePacketBuffer(msg);
        //log error
        return;
    }

    // toss the header IEs
    packetfunctions_tossHeader(msg,lenIE);

    // reset it to avoid race conditions with this var.
    msg->l2_joinPriorityPresent = FALSE;

    // send the packet up the stack, if it qualifies
    switch (msg->l2_frameType) {
    case IEEE154_TYPE_BEACON:
    case IEEE154_TYPE_DATA:
    case IEEE154_TYPE_CMD:
        if (msg->length>0) {
            if (msg->l2_frameType == IEEE154_TYPE_BEACON){
                // I have one byte frequence field, no useful for upper layer
                // free up the RAM
                openqueue_freePacketBuffer(msg);
                break;
            }
            // send to upper layer
            frag_receive(msg);
        } else {
            // free up the RAM
            openqueue_freePacketBuffer(msg);
        }
        break;
    case IEEE154_TYPE_ACK:
    default:
        // free the packet's RAM memory
        openqueue_freePacketBuffer(msg);
        // log the error
        openserial_printError(
            COMPONENT_SIXTOP,
            ERR_MSG_UNKNOWN_TYPE,
            (errorparameter_t)msg->l2_frameType,
            (errorparameter_t)0
        );
        break;
    }
}

//======= debugging

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_myDAGrank(void) {
    uint16_t output;

    output = 0;
    output = icmpv6rpl_getMyDAGrank();
    openserial_printStatus(STATUS_DAGRANK,(uint8_t*)&output,sizeof(uint16_t));
    return TRUE;
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_kaPeriod(void) {
    uint16_t output;

    output = sixtop_vars.kaPeriod;
    openserial_printStatus(
        STATUS_KAPERIOD,
        (uint8_t*)&output,
        sizeof(output)
    );
    return TRUE;
}

void sixtop_setIsResponseEnabled(bool isEnabled){
    sixtop_vars.isResponseEnabled = isEnabled;
}

//=========================== private =========================================

/**
\brief Transfer packet to MAC.

This function adds a IEEE802.15.4 header to the packet and leaves it the
OpenQueue buffer. The very last thing it does is assigning this packet to the
virtual component COMPONENT_SIXTOP_TO_IEEE802154E. Whenever it gets a change,
IEEE802154E will handle the packet.

\param[in] msg The packet to the transmitted
\param[in] payloadIEPresent Indicates wheter an Information Element is present in the
   packet.

\returns E_SUCCESS iff successful.
*/
owerror_t sixtop_send_internal(
    OpenQueueEntry_t* msg,
    bool    payloadIEPresent) {

    // assign a number of retries
    if (
        packetfunctions_isBroadcastMulticast(&(msg->l2_nextORpreviousHop))==TRUE
    ) {
        msg->l2_retriesLeft = 1;
    } else {
        msg->l2_retriesLeft = TXRETRIES + 1;
    }
    // record this packet's dsn (for matching the ACK)
    msg->l2_dsn = sixtop_vars.dsn++;
    // this is a new packet which I never attempted to send
    msg->l2_numTxAttempts = 0;
    // transmit with the default TX power
    
    msg->l1_txPower = TX_POWER;
    // add a IEEE802.15.4 header
    ieee802154_prependHeader(
        msg,
        msg->l2_frameType,
        payloadIEPresent,
        msg->l2_dsn,
        &(msg->l2_nextORpreviousHop)
    );
    // change owner to IEEE802154E fetches it from queue
    msg->owner  = COMPONENT_SIXTOP_TO_IEEE802154E;

    if (
        packetfunctions_isBroadcastMulticast(&(msg->l2_nextORpreviousHop))   == FALSE &&
        schedule_hasNegotiatedCellToNeighbor(&(msg->l2_nextORpreviousHop), CELLTYPE_TX) == FALSE &&
        schedule_hasAutoTxCellToNeighbor(&(msg->l2_nextORpreviousHop))       == FALSE
    ){
        // the frame source address is not broadcast/multicast
        // no negotiated tx cell to that neighbor
        // no auto tx cell to that neighbor

        schedule_addActiveSlot(
            msf_hashFunction_getSlotoffset(&(msg->l2_nextORpreviousHop)),    // slot offset
            CELLTYPE_TX,                                                     // type of slot
            TRUE,                                                            // shared?
            TRUE,                                                            // auto cell?
            CELLRADIOSETTING_FALLBACK,                                       // default radio setting for first time neogotiation
            msf_hashFunction_getChanneloffset(&(msg->l2_nextORpreviousHop)), // channel offset
            &(msg->l2_nextORpreviousHop)                                     // neighbor
        );
    }
    return E_SUCCESS;
}

/**
\brief sixtop sendingEb timer callback function.

\note This timer callback function is executed in task mode by opentimer
    already. No need to push a task again.
*/
void sixtop_sendingEb_timer_cb(opentimers_id_t id){
  // if multiple minimal cells are allocated, call the EB firing mechanism for
  // each cell
  for (uint8_t i =0;i<SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS;i++){
    timer_sixtop_sendEb_fired();
  }
}

/**
\brief sixtop maintenance timer callback function.

\note This timer callback function is executed in task mode by opentimer
    already. No need to push a task again.
*/
void sixtop_maintenance_timer_cb(opentimers_id_t id) {
    timer_sixtop_management_fired();
}

/**
\brief sixtop timeout timer callback function.

\note This timer callback function is executed in task mode by opentimer
    already. No need to push a task again.
*/
void sixtop_timeout_timer_cb(opentimers_id_t id) {
    timer_sixtop_six2six_timeout_fired();
}

//======= EB/KA task

void timer_sixtop_sendEb_fired(void) {

    if(openrandom_get16b()<(0xffff/EB_PORTION)){
        sixtop_sendEB();
    }
}

/**
\brief Timer handlers which triggers MAC management task.

This function is called in task context by the scheduler after the RES timer
has fired. This timer is set to fire every second, on average.

The body of this function executes one of the MAC management task.
*/
void timer_sixtop_management_fired(void) {

    sixtop_vars.mgtTaskCounter = (sixtop_vars.mgtTaskCounter+1)%MAINTENANCE_PERIOD;

    switch (sixtop_vars.mgtTaskCounter) {
        case 0:
            // called every MAINTENANCE_PERIOD seconds
            neighbors_removeOld();
            break;
        default:
            // called every second, except once every MAINTENANCE_PERIOD seconds
            sixtop_sendKA();
            break;
    }
}

/**
\brief Send an EB.

This is one of the MAC management tasks. This function inlines in the
timers_res_fired() function, but is declared as a separate function for better
readability of the code.

The general logic of this function is organized as follows:
  - Ensure you are in the right conditions to send an EB internally
  - Get a free packet buffer
  - Compose eb payload (e.g. shared cell(s) offset, their respective channels and link options etc.) 
  - Compose l2 attributes
  - Send the EB internally (pass it to 802154e). 

*/
port_INLINE void sixtop_sendEB(void) {
    OpenQueueEntry_t* eb;

    uint8_t     i;
    uint8_t     eb_len;
    uint16_t    temp16b;
    open_addr_t addressToWrite;

    memset(&addressToWrite,0,sizeof(open_addr_t));

//=========================== ensure you are in the right conditions ==========

    if (
        (ieee154e_isSynch()==FALSE)                     ||
        (IEEE802154_security_isConfigured()==FALSE)     ||
        (icmpv6rpl_getMyDAGrank()==DEFAULTDAGRANK)      ||
        icmpv6rpl_daoSent()==FALSE
    ) {
        // I'm not sync'ed, or did not join, or did not acquire a DAGrank or did not send out a DAO
        // before starting to advertize the network, we need to make sure that we are reachable downwards,
        // thus, the condition if DAO was sent

        // delete packets genereted by this module (EB and KA) from openqueue
        openqueue_removeAllCreatedBy(COMPONENT_SIXTOP);

        // I'm not busy sending an EB or KA
        sixtop_vars.busySendingEB = FALSE;
        sixtop_vars.busySendingKA = FALSE;
        sixtop_vars.ebCounter     = 0;

        // stop here
        return;
    }

    if (sixtop_vars.busySendingEB==TRUE) {
        // don't continue if I'm still sending a previous EB
        return;
    }

    // if I get here, I will send an EB

//=========================== get a free packet buffer ========================
    
    eb = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP);
    if (eb==NULL) {
        openserial_printError(
            COMPONENT_SIXTOP,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }

    // declare ownership over that packet
    eb->creator = COMPONENT_SIXTOP;
    eb->owner   = COMPONENT_SIXTOP;
    
//=========================== Compose eb payload ==============================

    // in case we none default number of shared cells defined in minimal configuration
    if (ebIEsBytestream[EB_SLOTFRAME_NUMLINK_OFFSET]>1){
        for (i=ebIEsBytestream[EB_SLOTFRAME_NUMLINK_OFFSET]-1;i>0;i--){
            packetfunctions_reserveHeaderSize(eb,5);
            eb->payload[0]   = i;    // slot offset
            eb->payload[1]   = 0x00;
            eb->payload[2]   = 0x00; // channel offset
            eb->payload[3]   = 0x00;
            eb->payload[4]   = cellLinkOptions[i]; // link options
        }
    }

    // reserve space for EB IEs
    packetfunctions_reserveHeaderSize(eb,EB_IE_LEN);
    for (i=0;i<EB_IE_LEN;i++){
        eb->payload[i]   = ebIEsBytestream[i];
    }

    if (ebIEsBytestream[EB_SLOTFRAME_NUMLINK_OFFSET]>1){
        // reconstruct the MLME IE header since length changed
        eb_len = EB_IE_LEN-2+5*(ebIEsBytestream[EB_SLOTFRAME_NUMLINK_OFFSET]-1);
        temp16b = eb_len | IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME | IEEE802154E_PAYLOAD_DESC_TYPE_MLME;
        eb->payload[0] = (uint8_t)(temp16b & 0x00ff);
        eb->payload[1] = (uint8_t)((temp16b & 0xff00)>>8);
    }

    eb->payload[EB_SLOTFRAME_LEN_OFFSET]   = (uint8_t)(0x00FF & (schedule_getFrameLength()));
    eb->payload[EB_SLOTFRAME_LEN_OFFSET+1] = (uint8_t)(0x00FF & (schedule_getFrameLength()>>8));
    
//=========================== Compose l2 attributes ===========================
    
    // Keep a pointer to where the ASN will be
    // Note: the actual value of the current ASN and JP will be written by the
    //    IEEE802.15.4e when transmitting
    eb->l2_ASNpayload               = &eb->payload[EB_ASN0_OFFSET];

    // some l2 information about this packet
    eb->l2_frameType                     = IEEE154_TYPE_BEACON;
    eb->l2_nextORpreviousHop.type        = ADDR_16B;
    eb->l2_nextORpreviousHop.addr_16b[0] = 0xff;
    eb->l2_nextORpreviousHop.addr_16b[1] = 0xff;

    //I has an IE in my payload
    eb->l2_payloadIEpresent = TRUE;

    // set l2-security attributes
    eb->l2_securityLevel   = IEEE802154_SECURITY_LEVEL_BEACON;
    eb->l2_keyIdMode       = IEEE802154_SECURITY_KEYIDMODE;
    eb->l2_keyIndex        = IEEE802154_security_getBeaconKeyIndex();
    
//=========================== Send ============================================
    
    // put in queue for MAC to handle
    sixtop_send_internal(eb,eb->l2_payloadIEpresent);

    //update ebCounter
    sixtop_vars.ebCounter++;    

    if (sixtop_vars.ebCounter >= SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS){
      // I have created enough EB packets for all minimal cells. 
      // I'm now busy sending an EB
      sixtop_vars.busySendingEB = TRUE;
    }

}

/**
\brief Send an keep-alive message, if necessary.

This is one of the MAC management tasks. This function inlines in the
timers_res_fired() function, but is declared as a separate function for better
readability of the code.
*/
port_INLINE void sixtop_sendKA(void) {
    OpenQueueEntry_t* kaPkt;
    open_addr_t*      kaNeighAddr;

    if (ieee154e_isSynch()==FALSE) {
        // I'm not sync'ed

        // delete packets genereted by this module (EB and KA) from openqueue
        openqueue_removeAllCreatedBy(COMPONENT_SIXTOP);

        // I'm not busy sending an EB or KA
        sixtop_vars.ebCounter     = 0;
        sixtop_vars.busySendingEB = FALSE;
        sixtop_vars.busySendingKA = FALSE;

        // stop here
        return;
    }

    if (sixtop_vars.busySendingKA==TRUE) {
        // don't proceed if I'm still sending a KA
        return;
    }

    kaNeighAddr = neighbors_getKANeighbor(sixtop_vars.kaPeriod);
    if (kaNeighAddr==NULL) {
        // don't proceed if I have no neighbor I need to send a KA to
        return;
    }

    if (schedule_hasNegotiatedCellToNeighbor(kaNeighAddr, CELLTYPE_TX) == FALSE){
        // delete packets genereted by this module (EB and KA) from openqueue
        openqueue_removeAllCreatedBy(COMPONENT_SIXTOP);

        // I'm not busy sending an EB or KA
        sixtop_vars.ebCounter     = 0;
        sixtop_vars.busySendingEB = FALSE;
        sixtop_vars.busySendingKA = FALSE;

        return;
    }

    // if I get here, I will send a KA

    // get a free packet buffer
    kaPkt = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP);
    if (kaPkt==NULL) {
        openserial_printError(
            COMPONENT_SIXTOP,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)1,
            (errorparameter_t)0
        );
        return;
   }

    // declare ownership over that packet
    kaPkt->creator = COMPONENT_SIXTOP;
    kaPkt->owner   = COMPONENT_SIXTOP;

    // some l2 information about this packet
    kaPkt->l2_frameType = IEEE154_TYPE_DATA;
    memcpy(&(kaPkt->l2_nextORpreviousHop),kaNeighAddr,sizeof(open_addr_t));

    // set l2-security attributes
    kaPkt->l2_securityLevel   = IEEE802154_SECURITY_LEVEL; // do not exchange KAs with
    kaPkt->l2_keyIdMode       = IEEE802154_SECURITY_KEYIDMODE;
    kaPkt->l2_keyIndex        = IEEE802154_security_getDataKeyIndex();

    // put in queue for MAC to handle
    sixtop_send_internal(kaPkt,FALSE);

    // I'm now busy sending a KA
    sixtop_vars.busySendingKA = TRUE;

#ifdef OPENSIM
    debugpins_ka_set();
    debugpins_ka_clr();
#endif
}

//======= six2six task

void timer_sixtop_six2six_timeout_fired(void) {

    if (sixtop_vars.six2six_state == SIX_STATE_WAIT_CLEARRESPONSE){
        // no response for the 6p clear, just clear locally
        schedule_removeAllNegotiatedCellsToNeighbor(
            sixtop_vars.cb_sf_getMetadata(),
            &sixtop_vars.neighborToClearCells,
            sixtop_vars.neighborRadioToClearCells
        );
        neighbors_resetSequenceNumber(&sixtop_vars.neighborToClearCells, sixtop_vars.neighborRadioToClearCells);
        memset(&sixtop_vars.neighborToClearCells,0,sizeof(open_addr_t));
    }
    // timeout timer fired, reset the state of sixtop to idle
    sixtop_vars.six2six_state = SIX_STATE_IDLE;
    opentimers_cancel(sixtop_vars.timeoutTimerId);
}

void sixtop_six2six_sendDone(OpenQueueEntry_t* msg, owerror_t error){

    msg->owner = COMPONENT_SIXTOP_RES;

    // if this is a request send done
    if (msg->l2_sixtop_messageType == SIXTOP_CELL_REQUEST){
        if(error == E_FAIL) {
            // max retries, without ack
            switch (sixtop_vars.six2six_state) {

            case SIX_STATE_WAIT_CLEARREQUEST_SENDDONE:
                sixtop_vars.six2six_state = SIX_STATE_WAIT_CLEARRESPONSE;
                timer_sixtop_six2six_timeout_fired();
                break;
            default:
                // reset handler and state if the request is failed to send out
                sixtop_vars.six2six_state = SIX_STATE_IDLE;
                break;
            }
        } else {
            // the packet has been sent out successfully
            switch (sixtop_vars.six2six_state) {
            case SIX_STATE_WAIT_ADDREQUEST_SENDDONE:
                sixtop_vars.six2six_state = SIX_STATE_WAIT_ADDRESPONSE;
                break;
            case SIX_STATE_WAIT_DELETEREQUEST_SENDDONE:
                sixtop_vars.six2six_state = SIX_STATE_WAIT_DELETERESPONSE;
                break;
            case SIX_STATE_WAIT_RELOCATEREQUEST_SENDDONE:
                sixtop_vars.six2six_state = SIX_STATE_WAIT_RELOCATERESPONSE;
                break;
            case SIX_STATE_WAIT_LISTREQUEST_SENDDONE:
                sixtop_vars.six2six_state = SIX_STATE_WAIT_LISTRESPONSE;
                break;
            case SIX_STATE_WAIT_COUNTREQUEST_SENDDONE:
                sixtop_vars.six2six_state = SIX_STATE_WAIT_COUNTRESPONSE;
                break;
            case SIX_STATE_WAIT_CLEARREQUEST_SENDDONE:
                sixtop_vars.six2six_state = SIX_STATE_WAIT_CLEARRESPONSE;
                break;
            default:
                // should never happen
                break;
            }
            // start timeout timer if I am waiting for a response
            opentimers_scheduleIn(
                sixtop_vars.timeoutTimerId,
                SIX2SIX_TIMEOUT_MS,
                TIME_MS,
                TIMER_ONESHOT,
                sixtop_timeout_timer_cb
            );
        }
    }

    // if this is a response send done
    if (msg->l2_sixtop_messageType == SIXTOP_CELL_RESPONSE){
        if(error == E_SUCCESS) {
            neighbors_updateSequenceNumber(&(msg->l2_nextORpreviousHop), msg->l2_cellRadioSetting );
            // in case a response is sent out, check the return code
            if (msg->l2_sixtop_returnCode == IANA_6TOP_RC_SUCCESS){
                if (msg->l2_sixtop_command == IANA_6TOP_CMD_ADD){
                    sixtop_addCells(
                        msg->l2_sixtop_frameID,
                        msg->l2_sixtop_celllist_add,
                        &(msg->l2_nextORpreviousHop),
                        msg->l2_sixtop_cellOptions
                    );
                }

                if (msg->l2_sixtop_command == IANA_6TOP_CMD_DELETE){
                    sixtop_removeCells(
                      msg->l2_sixtop_frameID,
                        msg->l2_sixtop_celllist_delete,
                      &(msg->l2_nextORpreviousHop),
                        msg->l2_sixtop_cellOptions
                    );
                }

                if ( msg->l2_sixtop_command == IANA_6TOP_CMD_RELOCATE){
                    sixtop_removeCells(
                        msg->l2_sixtop_frameID,
                        msg->l2_sixtop_celllist_delete,
                        &(msg->l2_nextORpreviousHop),
                        msg->l2_sixtop_cellOptions
                    );
                    sixtop_addCells(
                        msg->l2_sixtop_frameID,
                        msg->l2_sixtop_celllist_add,
                        &(msg->l2_nextORpreviousHop),
                        msg->l2_sixtop_cellOptions
                    );
                }

                if ( msg->l2_sixtop_command == IANA_6TOP_CMD_CLEAR){
                    schedule_removeAllNegotiatedCellsToNeighbor(
                        msg->l2_sixtop_frameID,
                        &(msg->l2_nextORpreviousHop),
                        msg->l2_cellRadioSetting
                    );
                    neighbors_resetSequenceNumber(&(msg->l2_nextORpreviousHop),msg->l2_cellRadioSetting);
                }
            } else {
                // the return code doesn't end up with SUCCESS
                // The return code will be processed on request side.
            }
        } else {
            // doesn't receive the ACK of response packet from request side after maximum retries.

            // if the response is for CLEAR command, remove all the cells and reset seqnum regardless NO ack received.
            if ( msg->l2_sixtop_command == IANA_6TOP_CMD_CLEAR){
                schedule_removeAllNegotiatedCellsToNeighbor(
                    msg->l2_sixtop_frameID,
                    &(msg->l2_nextORpreviousHop),
                    msg->l2_cellRadioSetting
                );
                neighbors_resetSequenceNumber(&(msg->l2_nextORpreviousHop),msg->l2_cellRadioSetting);
            }
        }
    }
    // free the buffer
    openqueue_freePacketBuffer(msg);
}

port_INLINE bool sixtop_processIEs(OpenQueueEntry_t* pkt, uint16_t * lenIE) {
    uint8_t ptr;
    uint8_t temp_8b;
    uint8_t subtypeid,code,sfid,version,type,seqNum;
    uint16_t temp_16b,len,headerlen;

    ptr         = 0;
    headerlen   = 0;

    //candidate IE header  if type ==0 header IE if type==1 payload IE
    temp_8b     = *((uint8_t*)(pkt->payload)+ptr);
    ptr++;
    temp_16b    = temp_8b + ((*((uint8_t*)(pkt->payload)+ptr))<<8);
    ptr++;
    *lenIE += 2;
    // check ietf ie group id, type
    if ((temp_16b & IEEE802154E_DESC_LEN_PAYLOAD_ID_TYPE_MASK) != (IANA_IETF_IE_GROUP_ID | IANA_IETF_IE_TYPE)){
        // wrong IE ID or type, record and drop the packet
        openserial_printError(COMPONENT_SIXTOP,ERR_UNSUPPORTED_FORMAT,0,0);
        return FALSE;
    }
    len = temp_16b & IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK;
    *lenIE += len;

    // check 6p subtype Id
    subtypeid  = *((uint8_t*)(pkt->payload)+ptr);
    ptr += 1;
    if (subtypeid != IANA_6TOP_SUBIE_ID){
        // wrong subtypeID, record and drop the packet
        openserial_printError(COMPONENT_SIXTOP,ERR_UNSUPPORTED_FORMAT,1,0);
        return FALSE;
    }
    headerlen += 1;

    // check 6p version
    temp_8b = *((uint8_t*)(pkt->payload)+ptr);
    ptr += 1;
    // 6p doesn't define type 3
    if (temp_8b>>IANA_6TOP_TYPE_SHIFT == 3){
        // wrong type, record and drop the packet
        openserial_printError(COMPONENT_SIXTOP,ERR_UNSUPPORTED_FORMAT,2,0);
        return FALSE;
    }
    version    = temp_8b &  IANA_6TOP_VESION_MASK;
    type       = temp_8b >> IANA_6TOP_TYPE_SHIFT;
    headerlen += 1;

    // get 6p code
    code       = *((uint8_t*)(pkt->payload)+ptr);
    ptr       += 1;
    headerlen += 1;
    // get 6p sfid
    sfid       = *((uint8_t*)(pkt->payload)+ptr);
    ptr       += 1;
    headerlen += 1;
    // get 6p seqNum and GEN
    seqNum     =  *((uint8_t*)(pkt->payload)+ptr) & 0xff;
    ptr       += 1;
    headerlen += 1;

    // give six2six to process
    sixtop_six2six_notifyReceive(version,type,code,sfid,seqNum,ptr,len-headerlen,pkt);
    *lenIE     = len+2;
    return TRUE;
}

void sixtop_six2six_notifyReceive(
    uint8_t           version,
    uint8_t           type,
    uint8_t           code,
    uint8_t           sfId,
    uint8_t           seqNum,
    uint8_t           ptr,
    uint8_t           length,
    OpenQueueEntry_t* pkt
){
    uint8_t           returnCode        = -1;
    uint16_t          metadata          = -1;
    uint8_t           cellOptions       = -1;
    cellRadioSetting_t cellRadioSetting;
    uint8_t           cellOptions_transformed;
    uint16_t          offset;
    uint16_t          length_groupid_type;
    uint16_t          startingOffset;
    uint8_t           maxNumCells;
    uint16_t          i;
    uint16_t          slotoffset;
    uint16_t          channeloffset;
    uint16_t          numCells;
    uint16_t          temp16;
    OpenQueueEntry_t* response_pkt;
    uint8_t           pktLen            = length;
    uint8_t           response_pktLen   = 0;
    cellInfo_ht       celllist_list[CELLLIST_MAX_LEN];

    if (type == SIXTOP_CELL_REQUEST){
        // if this is a 6p request message

        // drop the packet if there are too many 6P response in the queue
        if (openqueue_getNum6PResp()>=MAX6PRESPONSE) {
            return;
        }

        // get a free packet buffer
        response_pkt = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP_RES);
        if (response_pkt==NULL) {
            openserial_printError(
                COMPONENT_SIXTOP_RES,
                ERR_NO_FREE_PACKET_BUFFER,
                (errorparameter_t)0,
                (errorparameter_t)0
            );
            return;
        }

        // take ownership
        response_pkt->creator = COMPONENT_SIXTOP_RES;
        response_pkt->owner   = COMPONENT_SIXTOP_RES;

        memcpy(&(response_pkt->l2_nextORpreviousHop),
               &(pkt->l2_nextORpreviousHop),
               sizeof(open_addr_t)
        );

        // the follow while loop only execute once
        do{
            // version check
            if (version != IANA_6TOP_6P_VERSION){
                returnCode = IANA_6TOP_RC_VER_ERR;
                break;
            }
            // sfid check
            if (sfId != sixtop_vars.cb_sf_getsfid()){
                returnCode = IANA_6TOP_RC_SFID_ERR;
                break;
            }
            // sequenceNumber check
            if (seqNum != neighbors_getSequenceNumber(&(pkt->l2_nextORpreviousHop)) && code != IANA_6TOP_CMD_CLEAR){
                returnCode = IANA_6TOP_RC_SEQNUM_ERR;
                break;
            }
            // previous 6p transcation check
            if (sixtop_vars.six2six_state != SIX_STATE_IDLE){
                returnCode = IANA_6TOP_RC_RESET;
                break;
            }
            // metadata meaning check
            if (sixtop_vars.cb_sf_translateMetadata()!=METADATA_TYPE_FRAMEID){
                openserial_printError(
                    COMPONENT_SIXTOP,
                    ERR_UNSUPPORTED_METADATA,
                    sixtop_vars.cb_sf_translateMetadata(),
                    0
                );
                returnCode = IANA_6TOP_RC_ERROR;
                break;
            }

            // commands check

            // get metadata, metadata indicates frame id
            metadata  = *((uint8_t*)(pkt->payload)+ptr);
            metadata |= *((uint8_t*)(pkt->payload)+ptr+1)<<8;
            ptr      += 2;
            pktLen   -= 2;

            // clear command
            if (code == IANA_6TOP_CMD_CLEAR){
                // the cells will be removed when the repsonse sendone successfully
                // don't clear cells here
                returnCode = IANA_6TOP_RC_SUCCESS;
                break;
            }

            cellOptions  = *((uint8_t*)(pkt->payload)+ptr);
            
            // extract radio setting
            cellRadioSetting = (cellRadioSetting_t)((cellOptions & SIXTOP_CELLREQUEST_RADIOSETTING_MASK)>>5);
            
            // keep only link options bits
            // cellOptions = cellOptions & SIXTOP_CELLREQUEST_LINKOPTIONS_MASK;
            
            ptr         += 1;
            pktLen      -= 1;

            // list command
            if (code == IANA_6TOP_CMD_LIST){
                ptr += 1; // skip the one byte reserved field
                offset  = *((uint8_t*)(pkt->payload)+ptr);
                offset |= *((uint8_t*)(pkt->payload)+ptr+1)<<8;
                ptr += 2;
                maxNumCells  = *((uint8_t*)(pkt->payload)+ptr);
                maxNumCells |= *((uint8_t*)(pkt->payload)+ptr+1)<<8;
                ptr += 2;

                returnCode = IANA_6TOP_RC_SUCCESS;
                startingOffset = offset;
                if ((cellOptions & (CELLOPTIONS_TX | CELLOPTIONS_RX)) != (CELLOPTIONS_TX | CELLOPTIONS_RX)){
                    cellOptions_transformed = cellOptions ^ (CELLOPTIONS_TX | CELLOPTIONS_RX);
                } else {
                    cellOptions_transformed = cellOptions;
                }
                for(i=0; i<maxNumCells; i++) {
                    if (
                        schedule_getOneCellAfterOffset(
                            metadata,
                            startingOffset,
                            &(pkt->l2_nextORpreviousHop),
                            cellOptions_transformed,
                            &slotoffset,
                            &channeloffset)
                    ){
                        // found one cell after slot offset+i
                        packetfunctions_reserveHeaderSize(response_pkt,4);
                        response_pkt->payload[0] = slotoffset     & 0x00FF;
                        response_pkt->payload[1] = (slotoffset    & 0xFF00)>>8;
                        response_pkt->payload[2] = channeloffset  & 0x00FF;
                        response_pkt->payload[3] = (channeloffset & 0xFF00)>>8;
                        response_pktLen         += 4;
                        startingOffset           = slotoffset+1;
                    } else {
                        // no more cell after offset
                        returnCode = IANA_6TOP_RC_EOL;
                        break;
                    }
                }
                if (
                    schedule_getOneCellAfterOffset(
                        metadata,
                        startingOffset,
                        &(pkt->l2_nextORpreviousHop),
                        cellOptions_transformed,
                        &slotoffset,
                        &channeloffset) == FALSE
                ){
                    returnCode = IANA_6TOP_RC_EOL;
                }

                break;
            }

            // count command
            if (code == IANA_6TOP_CMD_COUNT){
                numCells = 0;
                startingOffset = 0;
                if ((cellOptions & (CELLOPTIONS_TX | CELLOPTIONS_RX)) != (CELLOPTIONS_TX | CELLOPTIONS_RX)){
                    cellOptions_transformed = cellOptions ^ (CELLOPTIONS_TX | CELLOPTIONS_RX);
                } else {
                    cellOptions_transformed = cellOptions;
                }
                for(i=0; i<schedule_getFrameLength(); i++) {
                    if (
                        schedule_getOneCellAfterOffset(
                            metadata,
                            startingOffset,
                            &(pkt->l2_nextORpreviousHop),
                            cellOptions_transformed,
                            &slotoffset,
                            &channeloffset)
                    ){
                        // found one cell after slot i
                        numCells++;
                        startingOffset = slotoffset+1;
                    }
                }
                returnCode = IANA_6TOP_RC_SUCCESS;
                packetfunctions_reserveHeaderSize(response_pkt,sizeof(uint16_t));
                response_pkt->payload[0] =  numCells & 0x00FF;
                response_pkt->payload[1] = (numCells & 0xFF00)>>8;
                response_pktLen         += 2;
                break;
            }

            numCells = *((uint8_t*)(pkt->payload)+ptr);
            ptr     += 1;
            pktLen  -= 1;

            // add command
            if (code == IANA_6TOP_CMD_ADD){
                if (schedule_getNumberOfFreeEntries() < numCells){
                    returnCode = IANA_6TOP_RC_BUSY;
                    break;
                }
                // retrieve cell list
                i = 0;
                memset(response_pkt->l2_sixtop_celllist_add,0,sizeof(response_pkt->l2_sixtop_celllist_add));
                while(pktLen>0){
                    response_pkt->l2_sixtop_celllist_add[i].slotoffset           =  *((uint8_t*)(pkt->payload)+ptr);
                    response_pkt->l2_sixtop_celllist_add[i].slotoffset          |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
                    response_pkt->l2_sixtop_celllist_add[i].channeloffset        =  *((uint8_t*)(pkt->payload)+ptr+2);
                    response_pkt->l2_sixtop_celllist_add[i].channeloffset       |= (*((uint8_t*)(pkt->payload)+ptr+3))<<8;
                    response_pkt->l2_sixtop_celllist_add[i].cellRadioSetting     =  cellRadioSetting;
                    response_pkt->l2_sixtop_celllist_add[i].isUsed               = TRUE;
                    ptr    += 4;
                    pktLen -= 4;
                    i++;
                }
                if (sixtop_areAvailableCellsToBeScheduled(metadata,numCells,response_pkt->l2_sixtop_celllist_add)){
                    for(i=0;i<CELLLIST_MAX_LEN;i++) {
                        if(response_pkt->l2_sixtop_celllist_add[i].isUsed){
                            packetfunctions_reserveHeaderSize(response_pkt,4);
                            response_pkt->payload[0] = (uint8_t)(response_pkt->l2_sixtop_celllist_add[i].slotoffset         & 0x00FF);
                            response_pkt->payload[1] = (uint8_t)((response_pkt->l2_sixtop_celllist_add[i].slotoffset        & 0xFF00)>>8);
                            response_pkt->payload[2] = (uint8_t)(response_pkt->l2_sixtop_celllist_add[i].channeloffset      & 0x00FF);
                            response_pkt->payload[3] = (uint8_t)((response_pkt->l2_sixtop_celllist_add[i].channeloffset     & 0xFF00)>>8);
                            response_pktLen += 4;
                        }
                    }
                }
                returnCode = IANA_6TOP_RC_SUCCESS;
                break;
            }

            // delete command
            if (code == IANA_6TOP_CMD_DELETE){
                i = 0;
                memset(response_pkt->l2_sixtop_celllist_delete,0,sizeof(response_pkt->l2_sixtop_celllist_delete));
                while(pktLen>0){
                    response_pkt->l2_sixtop_celllist_delete[i].slotoffset     =  *((uint8_t*)(pkt->payload)+ptr);
                    response_pkt->l2_sixtop_celllist_delete[i].slotoffset    |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
                    response_pkt->l2_sixtop_celllist_delete[i].channeloffset  =  *((uint8_t*)(pkt->payload)+ptr+2);
                    response_pkt->l2_sixtop_celllist_delete[i].channeloffset |= (*((uint8_t*)(pkt->payload)+ptr+3))<<8;
                    response_pkt->l2_sixtop_celllist_delete[i].isUsed         = TRUE;
                    ptr    += 4;
                    pktLen -= 4;
                    i++;
                }
                if ((cellOptions & (CELLOPTIONS_TX | CELLOPTIONS_RX)) != (CELLOPTIONS_TX | CELLOPTIONS_RX)){
                    cellOptions_transformed = cellOptions ^ (CELLOPTIONS_TX | CELLOPTIONS_RX);
                } else {
                    cellOptions_transformed = cellOptions;
                }
                if (sixtop_areAvailableCellsToBeRemoved(metadata,numCells,response_pkt->l2_sixtop_celllist_delete,&(pkt->l2_nextORpreviousHop),cellOptions_transformed)){
                    returnCode = IANA_6TOP_RC_SUCCESS;
                    for(i=0;i<CELLLIST_MAX_LEN;i++) {
                        if(response_pkt->l2_sixtop_celllist_delete[i].isUsed){
                            packetfunctions_reserveHeaderSize(response_pkt,4);
                            response_pkt->payload[0] = (uint8_t)(response_pkt->l2_sixtop_celllist_delete[i].slotoffset         & 0x00FF);
                            response_pkt->payload[1] = (uint8_t)((response_pkt->l2_sixtop_celllist_delete[i].slotoffset        & 0xFF00)>>8);
                            response_pkt->payload[2] = (uint8_t)(response_pkt->l2_sixtop_celllist_delete[i].channeloffset      & 0x00FF);
                            response_pkt->payload[3] = (uint8_t)((response_pkt->l2_sixtop_celllist_delete[i].channeloffset     & 0xFF00)>>8);
                            response_pktLen += 4;
                        }
                    }
                } else {
                    returnCode = IANA_6TOP_RC_CELLLIST_ERR;
                }
                    break;
            }

            // relocate command
            if (code == IANA_6TOP_CMD_RELOCATE){
                // retrieve cell list to be relocated
                i = 0;
                memset(response_pkt->l2_sixtop_celllist_delete,0,sizeof(response_pkt->l2_sixtop_celllist_delete));
                temp16 = numCells;
                while(temp16>0){
                    response_pkt->l2_sixtop_celllist_delete[i].slotoffset        =  *((uint8_t*)(pkt->payload)+ptr);
                    response_pkt->l2_sixtop_celllist_delete[i].slotoffset       |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
                    response_pkt->l2_sixtop_celllist_delete[i].channeloffset     =  *((uint8_t*)(pkt->payload)+ptr+2);
                    response_pkt->l2_sixtop_celllist_delete[i].channeloffset    |= (*((uint8_t*)(pkt->payload)+ptr+3))<<8;
                    response_pkt->l2_sixtop_celllist_delete[i].cellRadioSetting  = cellRadioSetting;
                    response_pkt->l2_sixtop_celllist_delete[i].isUsed         = TRUE;
                    ptr    += 4;
                    pktLen -= 4;
                    temp16--;
                    i++;
                }
                if ((cellOptions & (CELLOPTIONS_TX | CELLOPTIONS_RX)) != (CELLOPTIONS_TX | CELLOPTIONS_RX)){
                    cellOptions_transformed = cellOptions ^ (CELLOPTIONS_TX | CELLOPTIONS_RX);
                } else {
                    cellOptions_transformed = cellOptions;
                }
                if (sixtop_areAvailableCellsToBeRemoved(metadata,numCells,response_pkt->l2_sixtop_celllist_delete,&(pkt->l2_nextORpreviousHop),cellOptions_transformed)==FALSE){
                    returnCode = IANA_6TOP_RC_CELLLIST_ERR;
                    break;
                }
                // retrieve cell list to be relocated
                i = 0;
                memset(response_pkt->l2_sixtop_celllist_add,0,sizeof(response_pkt->l2_sixtop_celllist_add));
                while(pktLen>0){
                    response_pkt->l2_sixtop_celllist_add[i].slotoffset           =  *((uint8_t*)(pkt->payload)+ptr);
                    response_pkt->l2_sixtop_celllist_add[i].slotoffset          |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
                    response_pkt->l2_sixtop_celllist_add[i].channeloffset        =  *((uint8_t*)(pkt->payload)+ptr+2);
                    response_pkt->l2_sixtop_celllist_add[i].channeloffset       |= (*((uint8_t*)(pkt->payload)+ptr+3))<<8;
                    response_pkt->l2_sixtop_celllist_add[i].cellRadioSetting     = cellRadioSetting;
                    response_pkt->l2_sixtop_celllist_add[i].isUsed               = TRUE;
                    ptr    += 4;
                    pktLen -= 4;
                    i++;
                }
                if (sixtop_areAvailableCellsToBeScheduled(metadata,numCells,response_pkt->l2_sixtop_celllist_add)){
                    for(i=0;i<CELLLIST_MAX_LEN;i++) {
                        if(response_pkt->l2_sixtop_celllist_add[i].isUsed){
                            packetfunctions_reserveHeaderSize(response_pkt,4);
                            response_pkt->payload[0] = (uint8_t)(response_pkt->l2_sixtop_celllist_add[i].slotoffset         & 0x00FF);
                            response_pkt->payload[1] = (uint8_t)((response_pkt->l2_sixtop_celllist_add[i].slotoffset        & 0xFF00)>>8);
                            response_pkt->payload[2] = (uint8_t)(response_pkt->l2_sixtop_celllist_add[i].channeloffset      & 0x00FF);
                            response_pkt->payload[3] = (uint8_t)((response_pkt->l2_sixtop_celllist_add[i].channeloffset     & 0xFF00)>>8);
                            response_pktLen += 4;
                        }
                    }
                }
                returnCode = IANA_6TOP_RC_SUCCESS;
                break;
            }
        } while(0);

        // record code, returnCode, frameID and cellOptions. They will be used when 6p repsonse senddone
        response_pkt->l2_sixtop_command     = code;
        response_pkt->l2_sixtop_returnCode  = returnCode;
        response_pkt->l2_sixtop_frameID     = metadata;
        
        // fill the radio setting back into the cellOptions. 
        response_pkt->l2_sixtop_cellOptions = cellOptions | cellRadioSetting;
        
        // revert tx and rx link option bits 
        if ((cellOptions & (CELLOPTIONS_TX | CELLOPTIONS_RX)) != (CELLOPTIONS_TX | CELLOPTIONS_RX)){
            response_pkt->l2_sixtop_cellOptions = cellOptions ^ (CELLOPTIONS_TX | CELLOPTIONS_RX);
        } else {
            response_pkt->l2_sixtop_cellOptions = cellOptions;
        }

        // append 6p Seqnum
        packetfunctions_reserveHeaderSize(response_pkt,sizeof(uint8_t));
        *((uint8_t*)(response_pkt->payload)) = seqNum;
        response_pktLen += 1;

        // append 6p sfid
        packetfunctions_reserveHeaderSize(response_pkt,sizeof(uint8_t));
        *((uint8_t*)(response_pkt->payload)) = sixtop_vars.cb_sf_getsfid();
        response_pktLen += 1;

        // append 6p code
        packetfunctions_reserveHeaderSize(response_pkt,sizeof(uint8_t));
        *((uint8_t*)(response_pkt->payload)) = returnCode;
        response_pktLen += 1;

        // append 6p version, T(type) and  R(reserved)
        packetfunctions_reserveHeaderSize(response_pkt,sizeof(uint8_t));
        *((uint8_t*)(response_pkt->payload)) = IANA_6TOP_6P_VERSION | IANA_6TOP_TYPE_RESPONSE;
        response_pktLen += 1;

        // append 6p subtype id
        packetfunctions_reserveHeaderSize(response_pkt,sizeof(uint8_t));
        *((uint8_t*)(response_pkt->payload)) = IANA_6TOP_SUBIE_ID;
        response_pktLen += 1;

        // append IETF IE header (length_groupid_type)
        packetfunctions_reserveHeaderSize(response_pkt, sizeof(uint16_t));
        length_groupid_type  = response_pktLen;
        length_groupid_type |= (IANA_IETF_IE_GROUP_ID  | IANA_IETF_IE_TYPE);
        response_pkt->payload[0]      = length_groupid_type        & 0xFF;
        response_pkt->payload[1]      = (length_groupid_type >> 8) & 0xFF;

        // indicate IEs present
        response_pkt->l2_payloadIEpresent = TRUE;
        // record this packet as sixtop request message
        response_pkt->l2_sixtop_messageType    = SIXTOP_CELL_RESPONSE;

        if (sixtop_vars.isResponseEnabled){
            // send packet
            sixtop_send(response_pkt);
        } else {
            openqueue_freePacketBuffer(response_pkt);
        }
    }

    if (type == SIXTOP_CELL_RESPONSE) {
       // this is a 6p response message
        
      // if the code is SUCCESS
        if (code == IANA_6TOP_RC_SUCCESS || code == IANA_6TOP_RC_EOL){
            
            // extract radio setting
            cellRadioSetting = (cellRadioSetting_t)(sixtop_vars.cellOptions & SIXTOP_CELLREQUEST_RADIOSETTING_MASK)>>5;
            
            switch(sixtop_vars.six2six_state){
            case SIX_STATE_WAIT_ADDRESPONSE:
                i = 0;
                memset(pkt->l2_sixtop_celllist_add,0,sizeof(pkt->l2_sixtop_celllist_add));
                while(pktLen>0){
                    pkt->l2_sixtop_celllist_add[i].slotoffset            =  *((uint8_t*)(pkt->payload)+ptr);
                    pkt->l2_sixtop_celllist_add[i].slotoffset           |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
                    pkt->l2_sixtop_celllist_add[i].channeloffset         =  *((uint8_t*)(pkt->payload)+ptr+2);
                    pkt->l2_sixtop_celllist_add[i].channeloffset        |= (*((uint8_t*)(pkt->payload)+ptr+3))<<8;
                    pkt->l2_sixtop_celllist_add[i].cellRadioSetting      = cellRadioSetting ;
                    pkt->l2_sixtop_celllist_add[i].isUsed         = TRUE;
                    ptr    += 4;
                    pktLen -= 4;
                    i++;
                }
                sixtop_addCells(
                    sixtop_vars.cb_sf_getMetadata(),     // frame id
                    pkt->l2_sixtop_celllist_add,  // celllist to be added
                    &(pkt->l2_nextORpreviousHop), // neighbor that cells to be added to
                    sixtop_vars.cellOptions       // cell options
                );
                neighbors_updateSequenceNumber(&(pkt->l2_nextORpreviousHop),pkt->l2_cellRadioSetting);
                break;
            case SIX_STATE_WAIT_DELETERESPONSE:
                i = 0;
                memset(pkt->l2_sixtop_celllist_delete,0,sizeof(pkt->l2_sixtop_celllist_delete));
                while(pktLen>0){
                    pkt->l2_sixtop_celllist_delete[i].slotoffset     =  *((uint8_t*)(pkt->payload)+ptr);
                    pkt->l2_sixtop_celllist_delete[i].slotoffset    |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
                    pkt->l2_sixtop_celllist_delete[i].channeloffset  =  *((uint8_t*)(pkt->payload)+ptr+2);
                    pkt->l2_sixtop_celllist_delete[i].channeloffset |= (*((uint8_t*)(pkt->payload)+ptr+3))<<8;
                    pkt->l2_sixtop_celllist_delete[i].cellRadioSetting = cellRadioSetting;
                    pkt->l2_sixtop_celllist_delete[i].isUsed         = TRUE;
                    ptr    += 4;
                    pktLen -= 4;
                    i++;
                }
                sixtop_removeCells(
                    sixtop_vars.cb_sf_getMetadata(),
                    pkt->l2_sixtop_celllist_delete,
                    &(pkt->l2_nextORpreviousHop),
                    sixtop_vars.cellOptions
                );
                neighbors_updateSequenceNumber(&(pkt->l2_nextORpreviousHop),pkt->l2_cellRadioSetting);
                break;
            case SIX_STATE_WAIT_RELOCATERESPONSE:
                i = 0;
                memset(pkt->l2_sixtop_celllist_add,0,sizeof(pkt->l2_sixtop_celllist_add));
                while(pktLen>0){
                    pkt->l2_sixtop_celllist_add[i].slotoffset            =  *((uint8_t*)(pkt->payload)+ptr);
                    pkt->l2_sixtop_celllist_add[i].slotoffset           |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
                    pkt->l2_sixtop_celllist_add[i].channeloffset         =  *((uint8_t*)(pkt->payload)+ptr+2);
                    pkt->l2_sixtop_celllist_add[i].channeloffset        |= (*((uint8_t*)(pkt->payload)+ptr+3))<<8;
                    pkt->l2_sixtop_celllist_add[i].cellRadioSetting      = cellRadioSetting;
                    pkt->l2_sixtop_celllist_add[i].isUsed                = TRUE;
                    ptr    += 4;
                    pktLen -= 4;
                    i++;
                }
                sixtop_removeCells(
                    sixtop_vars.cb_sf_getMetadata(),
                    sixtop_vars.celllist_toDelete,
                    &(pkt->l2_nextORpreviousHop),
                    sixtop_vars.cellOptions
                );
                sixtop_addCells(
                    sixtop_vars.cb_sf_getMetadata(),     // frame id
                    pkt->l2_sixtop_celllist_add,  // celllist to be added
                    &(pkt->l2_nextORpreviousHop), // neighbor that cells to be added to
                    sixtop_vars.cellOptions       // cell options
                );
                neighbors_updateSequenceNumber(&(pkt->l2_nextORpreviousHop),pkt->l2_cellRadioSetting);
                break;
            case SIX_STATE_WAIT_COUNTRESPONSE:
                numCells  = *((uint8_t*)(pkt->payload)+ptr);
                numCells |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
                ptr += 2;
                openserial_printInfo(
                    COMPONENT_SIXTOP,
                    ERR_SIXTOP_COUNT,
                    (errorparameter_t)numCells,
                    (errorparameter_t)sixtop_vars.six2six_state
                );
                neighbors_updateSequenceNumber(&(pkt->l2_nextORpreviousHop),pkt->l2_cellRadioSetting);
                break;
            case SIX_STATE_WAIT_LISTRESPONSE:
                i = 0;
                memset(celllist_list,0,CELLLIST_MAX_LEN*sizeof(cellInfo_ht));
                while(pktLen>0){
                    celllist_list[i].slotoffset     =  *((uint8_t*)(pkt->payload)+ptr);
                    celllist_list[i].slotoffset    |= (*((uint8_t*)(pkt->payload)+ptr+1))<<8;
                    celllist_list[i].channeloffset  =  *((uint8_t*)(pkt->payload)+ptr+2);
                    celllist_list[i].channeloffset |= (*((uint8_t*)(pkt->payload)+ptr+3))<<8;
                    celllist_list[i].isUsed         = TRUE;
                    ptr    += 4;
                    pktLen -= 4;
                    i++;
                }
                // print out first two cells in the list
                openserial_printInfo(
                    COMPONENT_SIXTOP,
                    ERR_SIXTOP_LIST,
                    (errorparameter_t)celllist_list[0].slotoffset,
                    (errorparameter_t)celllist_list[1].slotoffset
                );
                neighbors_updateSequenceNumber(&(pkt->l2_nextORpreviousHop),pkt->l2_cellRadioSetting);
                break;
            case SIX_STATE_WAIT_CLEARRESPONSE:
                schedule_removeAllNegotiatedCellsToNeighbor(
                    sixtop_vars.cb_sf_getMetadata(),
                    &(pkt->l2_nextORpreviousHop),
                      cellRadioSetting
                );
                neighbors_resetSequenceNumber(&(pkt->l2_nextORpreviousHop),pkt->l2_cellRadioSetting);
                break;
            default:
                // The sixtop response arrived after 6P TIMEOUT, or
                // it's a duplicated response. Remove 6P request if I have.

                openqueue_remove6PrequestToNeighbor(&(pkt->l2_nextORpreviousHop));

                break;
            }
        } else {
            sixtop_vars.cb_sf_handleRCError(code, &(pkt->l2_nextORpreviousHop), pkt->l2_cellRadioSetting);
        }
        openserial_printInfo(
            COMPONENT_SIXTOP,
            ERR_SIXTOP_RETURNCODE,
            (errorparameter_t)code,
            (errorparameter_t)sixtop_vars.six2six_state
        );
        memset(&sixtop_vars.neighborToClearCells,0,sizeof(open_addr_t));
        sixtop_vars.six2six_state   = SIX_STATE_IDLE;
        opentimers_cancel(sixtop_vars.timeoutTimerId);
    }
}

//======= helper functions

bool sixtop_addCells(
    uint8_t      slotframeID,
    cellInfo_ht* cellList,
    open_addr_t* previousHop,
    uint8_t      cellOptions
){
    uint8_t     i;
    bool        isShared;
    open_addr_t temp_neighbor;
    cellType_t  type;
    bool        hasCellsAdded;

    // translate cellOptions to cell type

    if ((cellOptions & SIXTOP_CELLREQUEST_LINKOPTIONS_MASK) == CELLOPTIONS_TX){
        type     = CELLTYPE_TX;
        isShared = FALSE;
    }
    if ((cellOptions & SIXTOP_CELLREQUEST_LINKOPTIONS_MASK) == CELLOPTIONS_RX){
        type     = CELLTYPE_RX;
        isShared = FALSE;
    }
    if ((cellOptions & SIXTOP_CELLREQUEST_LINKOPTIONS_MASK) == (CELLOPTIONS_TX | CELLOPTIONS_RX | CELLOPTIONS_SHARED)){
        type     = CELLTYPE_TXRX;
        isShared = TRUE;
    }

    memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));

    hasCellsAdded = FALSE;
    // add cells to schedule
    for(i = 0;i<CELLLIST_MAX_LEN;i++){
        if (cellList[i].isUsed){
            hasCellsAdded = TRUE;
            schedule_addActiveSlot(
                cellList[i].slotoffset,
                type,
                isShared,
                FALSE,
                cellList[i].cellRadioSetting,
                cellList[i].channeloffset,
                &temp_neighbor
            );
         }
    }

    return hasCellsAdded;
}

bool sixtop_removeCells(
    uint8_t      slotframeID,
    cellInfo_ht* cellList,
    open_addr_t* previousHop,
    uint8_t      cellOptions
){
    uint8_t     i;
    bool        isShared;
    open_addr_t temp_neighbor;
    cellType_t  type;
    bool        hasCellsRemoved;

    // translate cellOptions to cell type
    if ((cellOptions & SIXTOP_CELLREQUEST_LINKOPTIONS_MASK) == CELLOPTIONS_TX){
        type     = CELLTYPE_TX;
        isShared = FALSE;
    }
    if ((cellOptions & SIXTOP_CELLREQUEST_LINKOPTIONS_MASK) == CELLOPTIONS_RX){
        type     = CELLTYPE_RX;
        isShared = FALSE;
    }
    if ((cellOptions & SIXTOP_CELLREQUEST_LINKOPTIONS_MASK) == (CELLOPTIONS_TX | CELLOPTIONS_RX | CELLOPTIONS_SHARED)){
        type     = CELLTYPE_TXRX;
        isShared = TRUE;
    }

    memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));

    hasCellsRemoved = FALSE;
    // delete cells from schedule
    for(i=0;i<CELLLIST_MAX_LEN;i++){
        if (cellList[i].isUsed){
            hasCellsRemoved = TRUE;
            schedule_removeActiveSlot(
                cellList[i].slotoffset,
                type,
                isShared,
                &temp_neighbor
            );
        }
    }

    return hasCellsRemoved;
}

bool sixtop_areAvailableCellsToBeScheduled(
      uint8_t      frameID,
      uint8_t      numOfCells,
      cellInfo_ht* cellList
){
    uint8_t i;
    uint8_t numbOfavailableCells;
    bool    available;

    i          = 0;
    numbOfavailableCells = 0;
    available  = FALSE;

    if(numOfCells == 0 || numOfCells>CELLLIST_MAX_LEN){
        // log wrong parameter error TODO

        available = FALSE;
    } else {
        do {
            if(schedule_isSlotOffsetAvailable(cellList[i].slotoffset) == TRUE){
                numbOfavailableCells++;
            } else {
                // mark the cell
                cellList[i].isUsed = FALSE;
            }
            i++;
        }while(i<CELLLIST_MAX_LEN && numbOfavailableCells!=numOfCells);

        if(numbOfavailableCells>0){
            // there are more than one cell can be added.
            // the rest cells in the list will not be used
            while(i<CELLLIST_MAX_LEN){
                cellList[i].isUsed = FALSE;
                i++;
            }
            available = TRUE;
        } else {
            // No cell in the list is able to be added
            available = FALSE;
        }
    }
    return available;
}

bool sixtop_areAvailableCellsToBeRemoved(
    uint8_t      frameID,
    uint8_t      numOfCells,
    cellInfo_ht* cellList,
    open_addr_t* neighbor,
    uint8_t      cellOptions
){
    uint8_t              i;
    uint8_t              numOfavailableCells;
    bool                 available;
    slotinfo_element_t   info;
    cellType_t           type;
    open_addr_t          anycastAddr;

    i          = 0;
    numOfavailableCells = 0;
    available           = TRUE;

    // translate cellOptions to cell type
    if ((cellOptions & SIXTOP_CELLREQUEST_LINKOPTIONS_MASK) == CELLOPTIONS_TX){
        type = CELLTYPE_TX;
    }
    if ((cellOptions & SIXTOP_CELLREQUEST_LINKOPTIONS_MASK) == CELLOPTIONS_RX){
        type = CELLTYPE_RX;
    }
    if ((cellOptions & SIXTOP_CELLREQUEST_LINKOPTIONS_MASK) == (CELLOPTIONS_TX | CELLOPTIONS_RX | CELLOPTIONS_SHARED)){
        type = CELLTYPE_TXRX;
        memset(&anycastAddr,0,sizeof(open_addr_t));
        anycastAddr.type = ADDR_ANYCAST;
    }

    if(numOfCells == 0 || numOfCells>CELLLIST_MAX_LEN){
        // log wrong parameter error TODO
        available = FALSE;
    } else {
        do {
            if (cellList[i].isUsed){
                memset(&info,0,sizeof(slotinfo_element_t));
                if (type==CELLTYPE_TXRX){
                    schedule_getSlotInfo(cellList[i].slotoffset,&info);
                } else {
                    schedule_getSlotInfo(cellList[i].slotoffset,&info);
                }
                if(info.link_type != type){
                    available = FALSE;
                    break;
                } else {
                    numOfavailableCells++;
                }
            }
            i++;
        }while(i<CELLLIST_MAX_LEN && numOfavailableCells<numOfCells);

        if(numOfavailableCells==numOfCells && available == TRUE){
            //the rest link will not be scheduled, mark them as off type
            while(i<CELLLIST_MAX_LEN){
                cellList[i].isUsed = FALSE;
                i++;
            }
        } else {
            // local schedule can't satisfy the bandwidth of cell request
            available = FALSE;
        }
   }
   return available;
}
