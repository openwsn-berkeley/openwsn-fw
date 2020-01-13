#include "opendefs.h"
#include "msf.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "openrandom.h"
#include "idmanager.h"
#include "icmpv6rpl.h"
#include "IEEE802154E.h"
#include "openqueue.h"
#include "packetfunctions.h"

//=========================== definition =====================================

//=========================== variables =======================================

msf_vars_t msf_vars;

//=========================== prototypes ======================================

// sixtop callback
uint16_t msf_getMetadata(void);
metadata_t msf_translateMetadata(void);
void msf_handleRCError(uint8_t code, open_addr_t* address);

void msf_timer_housekeeping_cb(opentimers_id_t id);
void msf_timer_housekeeping_task(void);

void msf_timer_waitretry_cb(opentimers_id_t id);

void msf_timer_clear_task(void);
// msf private
void msf_trigger6pAdd(void);
void msf_trigger6pDelete(void);
void msf_housekeeping(void);

//=========================== public ==========================================

void msf_init(void) {

    open_addr_t     temp_neighbor;

    memset(&msf_vars,0,sizeof(msf_vars_t));
    msf_vars.numAppPacketsPerSlotFrame = 0;
    sixtop_setSFcallback(
        (sixtop_sf_getsfid_cbt)msf_getsfid,
        (sixtop_sf_getmetadata_cbt)msf_getMetadata,
        (sixtop_sf_translatemetadata_cbt)msf_translateMetadata,
        (sixtop_sf_handle_callback_cbt)msf_handleRCError
    );

    memset(&temp_neighbor,0,sizeof(temp_neighbor));
    temp_neighbor.type             = ADDR_ANYCAST;
    schedule_addActiveSlot(
        msf_hashFunction_getSlotoffset(idmanager_getMyID(ADDR_64B)),     // slot offset
        CELLTYPE_RX,                                                     // type of slot
        FALSE,                                                           // shared?
        TRUE,                                                            // auto cell?
        msf_hashFunction_getChanneloffset(idmanager_getMyID(ADDR_64B)),  // channel offset
        &temp_neighbor                                                   // neighbor
    );

    msf_vars.housekeepingTimerId = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_MSF);
    msf_vars.housekeepingPeriod  = HOUSEKEEPING_PERIOD;
    opentimers_scheduleIn(
        msf_vars.housekeepingTimerId,
        openrandom_getRandomizePeriod(msf_vars.housekeepingPeriod, msf_vars.housekeepingPeriod),
        TIME_MS,
        TIMER_PERIODIC,
        msf_timer_housekeeping_cb
    );
    msf_vars.waitretryTimerId    = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_MSF);
}

// called by schedule
void    msf_updateCellsElapsed(open_addr_t* neighbor, cellType_t type){

#ifdef MSF_ADAPTING_TO_TRAFFIC
    if (icmpv6rpl_isPreferredParent(neighbor)==FALSE){
        return;
    }

    // update numcellselapsed

    switch(type) {
    case CELLTYPE_TX:
        msf_vars.numCellsElapsed_tx++;
        break;
    case CELLTYPE_RX:
        msf_vars.numCellsElapsed_rx++;
        break;
    default:
        // not appliable
        return;
    }

    // addapt to upward traffic

    if (msf_vars.numCellsElapsed_tx == MAX_NUMCELLS){

        msf_vars.needAddTx       = FALSE;
        msf_vars.needDeleteTx    = FALSE;

        msf_vars.previousNumCellsUsed_tx = msf_vars.numCellsUsed_tx;

        if (msf_vars.numCellsUsed_tx > LIM_NUMCELLSUSED_HIGH){
            msf_vars.needAddTx    = TRUE;
            scheduler_push_task(msf_trigger6pAdd,TASKPRIO_MSF);
        }
        if (msf_vars.numCellsUsed_tx < LIM_NUMCELLSUSED_LOW){
            msf_vars.needDeleteTx = TRUE;
            scheduler_push_task(msf_trigger6pDelete,TASKPRIO_MSF);
        }
        msf_vars.numCellsElapsed_tx = 0;
        msf_vars.numCellsUsed_tx    = 0;
    }

    // adapt to downward traffic when there are negotiated Tx cells in schedule

    if (schedule_getNumberOfNegotiatedCells(neighbor, CELLTYPE_TX)==0){
        return;
    }

    // addapt to downward traffic

    if (msf_vars.numCellsElapsed_rx == MAX_NUMCELLS){

        msf_vars.needAddRx       = FALSE;
        msf_vars.needDeleteRx    = FALSE;

        msf_vars.previousNumCellsUsed_rx = msf_vars.numCellsUsed_rx;

        if (msf_vars.numCellsUsed_rx > LIM_NUMCELLSUSED_HIGH){
            msf_vars.needAddRx    = TRUE;
            scheduler_push_task(msf_trigger6pAdd,TASKPRIO_MSF);
        }
        if (msf_vars.numCellsUsed_rx < LIM_NUMCELLSUSED_LOW){
            msf_vars.needDeleteRx = TRUE;
            scheduler_push_task(msf_trigger6pDelete,TASKPRIO_MSF);
        }
        msf_vars.numCellsElapsed_rx = 0;
        msf_vars.numCellsUsed_rx    = 0;
    }
#endif
}

void    msf_updateCellsUsed(open_addr_t* neighbor, cellType_t type){

    if (icmpv6rpl_isPreferredParent(neighbor)==FALSE){
        return;
    }

    switch(type) {
    case CELLTYPE_TX:
        msf_vars.numCellsUsed_tx++;
        break;
    case CELLTYPE_RX:
        msf_vars.numCellsUsed_rx++;
        break;
    default:
        // not appliable
        return;
    }
}

//=========================== callback =========================================

uint8_t msf_getsfid(void){
    return IANA_6TISCH_SFID_MSF;
}

uint16_t msf_getMetadata(void){
    return SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
}

metadata_t msf_translateMetadata(void){
    return METADATA_TYPE_FRAMEID;
}

void msf_handleRCError(uint8_t code, open_addr_t* address){
    uint16_t waitDuration;

    if (
        code==IANA_6TOP_RC_RESET        ||
        code==IANA_6TOP_RC_LOCKED
    ){
        // waitretry
        msf_vars.waitretry = TRUE;
        waitDuration = WAITDURATION_MIN+openrandom_get16b()%WAITDURATION_RANDOM_RANGE;
        opentimers_scheduleIn(
            msf_vars.waitretryTimerId,
            waitDuration,
            TIME_MS,
            TIMER_ONESHOT,
            msf_timer_waitretry_cb
        );
    }

    if (
        code==IANA_6TOP_RC_ERROR        ||
        code==IANA_6TOP_RC_VER_ERR      ||
        code==IANA_6TOP_RC_SFID_ERR
    ){
        // quarantine
    }

    if (
        code==IANA_6TOP_RC_SEQNUM_ERR   ||
        code==IANA_6TOP_RC_CELLLIST_ERR
    ){
        // clear
        scheduler_push_task(msf_timer_clear_task,TASKPRIO_MSF);
    }

    if (code==IANA_6TOP_RC_BUSY){
        // mark neighbor f6NORES
        neighbors_setNeighborNoResource(address);
    }

    neighbors_updateSequenceNumber(address);
}

void msf_timer_waitretry_cb(opentimers_id_t id){
    msf_vars.waitretry = FALSE;
}

void msf_timer_housekeeping_cb(opentimers_id_t id){
    PORT_TIMER_WIDTH newDuration;

    // update the timer period
    newDuration = openrandom_getRandomizePeriod(msf_vars.housekeepingPeriod, msf_vars.housekeepingPeriod),
    opentimers_updateDuration(msf_vars.housekeepingTimerId, newDuration);

    // calling the task directly as the timer_cb function is executed in
    // task mode by opentimer already
    msf_timer_housekeeping_task();
}

//=========================== tasks ============================================

void msf_timer_housekeeping_task(void){

    msf_housekeeping();
}

void msf_timer_clear_task(void){
    open_addr_t    neighbor;
    bool           foundNeighbor;

    // get preferred parent
    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
    if (foundNeighbor==FALSE) {
        return;
    }

    sixtop_request(
        IANA_6TOP_CMD_CLEAR,       // code
        &neighbor,                 // neighbor
        NUMCELLS_MSF,              // number cells
        CELLOPTIONS_MSF,           // cellOptions (not used)
        NULL,                      // celllist to add (not used)
        NULL,                      // celllist to delete (not used)
        IANA_6TISCH_SFID_MSF,      // sfid
        0,                         // list command offset (not used)
        0                          // list command maximum celllist (not used)
    );
}

//=========================== private =========================================

void msf_trigger6pAdd(void){
    open_addr_t    neighbor;
    bool           foundNeighbor;
    cellInfo_ht    celllist_add[CELLLIST_MAX_LEN];

    uint8_t        cellOptions;

    if (ieee154e_isSynch()==FALSE) {
        return;
    }

    if (msf_vars.waitretry){
        return;
    }

    // get preferred parent

    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
    if (foundNeighbor==FALSE) {
        return;
    }

    // check what type of cell need to add

    if (msf_vars.needAddTx){
        cellOptions = CELLOPTIONS_TX;
    } else {
        if (msf_vars.needAddRx) {
            cellOptions = CELLOPTIONS_RX;
        } else {
            // no need to add cell
            return;
        }
    }

    if (msf_candidateAddCellList(celllist_add,NUMCELLS_MSF)==FALSE){
        // failed to get cell list to add
        return;
    }

    sixtop_request(
        IANA_6TOP_CMD_ADD,           // code
        &neighbor,                   // neighbor
        NUMCELLS_MSF,                // number cells
        cellOptions,                 // cellOptions
        celllist_add,                // celllist to add
        NULL,                        // celllist to delete (not used)
        IANA_6TISCH_SFID_MSF,        // sfid
        0,                           // list command offset (not used)
        0                            // list command maximum celllist (not used)
    );
}

void msf_trigger6pDelete(void){
    open_addr_t    neighbor;
    bool           foundNeighbor;
    cellInfo_ht    celllist_delete[CELLLIST_MAX_LEN];

    uint8_t        cellOptions;

    if (ieee154e_isSynch()==FALSE) {
        return;
    }

    if (msf_vars.waitretry){
        return;
    }

    // get preferred parent
    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
    if (foundNeighbor==FALSE) {
        return;
    }

    if (msf_vars.needDeleteTx) {
        if (schedule_getNumberOfNegotiatedCells(&neighbor, CELLTYPE_TX)<=1){
            // at least one negotiated Tx cell presents
            msf_vars.needDeleteTx = FALSE;
        }
    }

    // check what type of cell need to delete

    if (msf_vars.needDeleteTx){
        cellOptions = CELLOPTIONS_TX;
    } else {
        if (msf_vars.needDeleteRx) {
            cellOptions = CELLOPTIONS_RX;
        } else {
            // no need to delete cell
            return;
        }
    }

    if (msf_candidateRemoveCellList(celllist_delete,&neighbor,NUMCELLS_MSF, cellOptions)==FALSE){
        // failed to get cell list to delete
        return;
    }

    sixtop_request(
        IANA_6TOP_CMD_DELETE,   // code
        &neighbor,              // neighbor
        NUMCELLS_MSF,           // number cells
        cellOptions,            // cellOptions
        NULL,                   // celllist to add (not used)
        celllist_delete,        // celllist to delete
        IANA_6TISCH_SFID_MSF,   // sfid
        0,                      // list command offset (not used)
        0                       // list command maximum celllist (not used)
    );
}

void msf_appPktPeriod(uint8_t numAppPacketsPerSlotFrame){
    msf_vars.numAppPacketsPerSlotFrame = numAppPacketsPerSlotFrame;
}

bool msf_candidateAddCellList(
      cellInfo_ht* cellList,
      uint8_t      requiredCells
   ){
    uint8_t i;
    frameLength_t slotoffset;
    uint8_t numCandCells;

    memset(cellList,0,CELLLIST_MAX_LEN*sizeof(cellInfo_ht));
    numCandCells=0;
    for(i=0;i<CELLLIST_MAX_LEN;i++){
        slotoffset = openrandom_get16b()%schedule_getFrameLength();
        if(schedule_isSlotOffsetAvailable(slotoffset)==TRUE){
            cellList[numCandCells].slotoffset       = slotoffset;
            cellList[numCandCells].channeloffset    = openrandom_get16b()&0x0F;
            cellList[numCandCells].isUsed           = TRUE;
            numCandCells++;
        }
    }

    if (numCandCells<requiredCells || requiredCells==0) {
        return FALSE;
    } else {
        return TRUE;
    }
}

bool msf_candidateRemoveCellList(
      cellInfo_ht* cellList,
      open_addr_t* neighbor,
      uint8_t      requiredCells,
      uint8_t      cellOptions
){
    uint8_t              i;
    uint8_t              numCandCells;
    slotinfo_element_t   info;

    memset(cellList,0,CELLLIST_MAX_LEN*sizeof(cellInfo_ht));
    numCandCells    = 0;
    for(i=0;i<schedule_getFrameLength();i++){
        schedule_getSlotInfo(i,&info);
        if(
            packetfunctions_sameAddress(neighbor, &(info.address)) &&
            info.link_type  == cellOptions &&
            info.isAutoCell == FALSE
        ){
            cellList[numCandCells].slotoffset       = i;
            cellList[numCandCells].channeloffset    = info.channelOffset;
            cellList[numCandCells].isUsed           = TRUE;
            numCandCells++;
            if (numCandCells==CELLLIST_MAX_LEN){
                break;
            }
        }
    }

    if(numCandCells<requiredCells){
        return FALSE;
    }else{
        return TRUE;
    }
}

void msf_housekeeping(void){

    open_addr_t    parentNeighbor;
    open_addr_t    nonParentNeighbor;
    bool           foundNeighbor;
    cellInfo_ht    celllist_add[CELLLIST_MAX_LEN];
    cellInfo_ht    celllist_delete[CELLLIST_MAX_LEN];

    if (ieee154e_isSynch()==FALSE) {
        return;
    }

    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&parentNeighbor);
    if (foundNeighbor==FALSE) {
        return;
    }

    if (schedule_hasNegotiatedTxCellToNonParent(&parentNeighbor, &nonParentNeighbor)==TRUE){

        // send a clear request to the non-parent neighbor

        sixtop_request(
            IANA_6TOP_CMD_CLEAR,     // code
            &nonParentNeighbor,      // neighbor
            NUMCELLS_MSF,            // number cells
            CELLOPTIONS_MSF,         // cellOptions
            NULL,                    // celllist to add (not used)
            NULL,                    // celllist to delete (not used)
            IANA_6TISCH_SFID_MSF,    // sfid
            0,                       // list command offset (not used)
            0                        // list command maximum celllist (not used)
        );
        return;
    }

    if (schedule_getNumberOfNegotiatedCells(&parentNeighbor, CELLTYPE_TX)==0){
        msf_vars.needAddTx = TRUE;
        msf_trigger6pAdd();
        return;
    }

    if (msf_vars.waitretry){
        return;
    }

    if (schedule_isNumTxWrapped(&parentNeighbor)==FALSE){
        return;
    }

    memset(celllist_delete, 0, CELLLIST_MAX_LEN*sizeof(cellInfo_ht));
    if (schedule_getCellsToBeRelocated(&parentNeighbor, celllist_delete)){
        if (msf_candidateAddCellList(celllist_add,NUMCELLS_MSF)==FALSE){
            // failed to get cell list to add
            return;
        }
        sixtop_request(
            IANA_6TOP_CMD_RELOCATE,  // code
            &parentNeighbor,         // neighbor
            NUMCELLS_MSF,            // number cells
            CELLOPTIONS_MSF,         // cellOptions
            celllist_add,            // celllist to add
            celllist_delete,         // celllist to delete
            IANA_6TISCH_SFID_MSF,    // sfid
            0,                       // list command offset (not used)
            0                        // list command maximum celllist (not used)
        );
    }
}

uint16_t msf_hashFunction_getSlotoffset(open_addr_t* address){

    uint16_t moteId;

    moteId = (((uint16_t)(address->addr_64b[6]))<<8) + (uint16_t)(address->addr_64b[7]);

    return SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS + \
            (moteId%(SLOTFRAME_LENGTH-SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS));
}

uint8_t msf_hashFunction_getChanneloffset(open_addr_t* address){

    uint16_t moteId;

    moteId = (((uint16_t)(address->addr_64b[6]))<<8) + (uint16_t)(address->addr_64b[7]);

    return moteId%NUM_CHANNELS;
}

void    msf_setHashCollisionFlag(bool isCollision){
    msf_vars.f_hashCollision = isCollision;
}
bool    msf_getHashCollisionFlag(void){
    return msf_vars.f_hashCollision;
}

uint8_t msf_getPreviousNumCellsUsed(cellType_t cellType){
    switch(cellType){
    case CELLTYPE_TX:
        return msf_vars.previousNumCellsUsed_tx;
        break;
    case CELLTYPE_RX:
        return msf_vars.previousNumCellsUsed_rx;
        break;
    default:
        // not appliable
        return 0;
    }
}