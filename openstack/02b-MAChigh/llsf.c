#include "llsf.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "openqueue.h"
#include "openrandom.h"

//=========================== define ==========================================

#define MAX_SCHEDULED_SLOT 10
#define MAXTRIES 5

//=========================== variables =======================================

typedef struct {
    uint16_t slotoffset;
    uint16_t distance;
} slot_distance_t;

typedef struct {
    slot_distance_t sd[MAX_SCHEDULED_SLOT];
} llsf_vars_t;

llsf_vars_t llsf_vars;

//=========================== prototypes ======================================

bool llsf_generateLowLantencySlots_add(
     uint16_t * slotsList,
     cellInfo_ht* cellList
);
bool llsf_generateLowLantencySlots_remove(
     uint16_t * txSlotsList,
     uint16_t * rxSlotsList,
     cellInfo_ht* cellList,
     open_addr_t* neighbor,
     uint16_t numOfCells
);

// private
bool llsf_isSlotAvialable(uint16_t slot);

// helper
void array_sort(uint16_t * array);
void stack_push(cellInfo_ht* cellList,uint16_t slot);

//=========================== public ==========================================

// admin
void llsf_init() {
    memset(&llsf_vars,0,sizeof(llsf_vars_t));
}

bool llsf_candidateAddCellList(
      uint8_t*     type,
      uint8_t*     frameID,
      uint8_t*     flag,
      cellInfo_ht* cellList
    ){
    bool returnVal = FALSE;
    uint16_t rxSlotsInSchedule[MAX_SCHEDULED_SLOT];
    
    *type = 1;
    *frameID = schedule_getFrameHandle();
    *flag = 1; // the cells listed in cellList are available to be schedule.
    
    memset(&rxSlotsInSchedule[0],0xff,sizeof(rxSlotsInSchedule));
    // get list of slot for receiving
    schedule_getScheduledSlots(&rxSlotsInSchedule[0],CELLTYPE_RX); 
    returnVal = llsf_generateLowLantencySlots_add(&rxSlotsInSchedule[0],cellList);
    return returnVal;
}
    
bool llsf_candidateRemoveCellList(
     uint8_t*             type,
     uint8_t*             frameID,
     uint8_t*             flag,
     cellInfo_ht*         cellList,
     open_addr_t*         neighbor,
     uint16_t             numOfCells 
    ){
    bool returnVal = FALSE;
    uint16_t txSlotsInSchedule[MAX_SCHEDULED_SLOT];
    uint16_t rxSlotsInSchedule[MAX_SCHEDULED_SLOT];

    *type           = 1;
    *frameID        = schedule_getFrameHandle();
    *flag           = 1;
    
    memset(&txSlotsInSchedule[0],0xff,sizeof(txSlotsInSchedule));
    memset(&rxSlotsInSchedule[0],0xff,sizeof(rxSlotsInSchedule));
    // get list of slot for receiving and transmitting 
    schedule_getScheduledSlots(&txSlotsInSchedule[0],CELLTYPE_TX);
    schedule_getScheduledSlots(&rxSlotsInSchedule[0],CELLTYPE_RX);
    returnVal = llsf_generateLowLantencySlots_remove(
                    txSlotsInSchedule,
                    rxSlotsInSchedule,
                    cellList,
                    neighbor,
                    numOfCells);
    return returnVal;
}
//================================== private ===================================
bool llsf_generateLowLantencySlots_add(uint16_t * slotsList, cellInfo_ht* cellList){
    uint8_t numCandCells;
    uint16_t i,j;
    frameLength_t slotframe_length;
    uint8_t counter;
    slot_distance_t temp;
    
    memset(cellList,0,sizeof(cellList));
    
    slotframe_length = schedule_getFrameLength();
    array_sort(slotsList);
    
    numCandCells=0;
    if (slotsList[0] == 0xffff) { // no dedicated cell was scheduled
        printf("LLSF: random select\n");
        for (counter=0;counter<SCHEDULEIEMAXNUMCELLS;counter++){
            i  = openrandom_get16b() % (slotframe_length \
                                       -SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS \
                                       -NUMSERIALRX);
            i += SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS + NUMSERIALRX;
            if (schedule_isSlotOffsetAvailable(i)==TRUE){
                // check whether the new cell is already in cellList
                for (j=0;j<numCandCells;j++) {
                     if (i == cellList[j].tsNum) {
                         break;
                     }
                }
                if (j<numCandCells) {
                    // slot i is already in cellList
                    continue;
                }
                cellList[numCandCells].tsNum       = i;
                cellList[numCandCells].choffset    = 0;
                cellList[numCandCells].linkoptions = CELLTYPE_TX;
                numCandCells++;
            }
        }
        if (numCandCells==0) {
           return FALSE;
        } else {
           return TRUE;
        }
    } else {
        printf("LLSF: Low Lantecy select\n");
        // there are dedicated cells in schedule, 
        // choose the one with lowest lantency (max in distance)
        
        // get the maxDistance of each pair adjacent slot A and B and
        // select the slots right after slot B, which has highest possiblity 
        // to be the first slots available after packet generated
        for (j=1;j<MAX_SCHEDULED_SLOT;j++){
            if (slotsList[j] == 0xffff) {
                i = slotsList[0]+1;
                while(schedule_isSlotOffsetAvailable(i)==FALSE || \
                    llsf_isSlotAvialable(i)==FALSE){
                    i++;
                    if (j!=1){
                        if (i==slotsList[1]){
                            break;
                        }
                    } else {
                        if (i==SLOTFRAME_LENGTH){
                            i = SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS + NUMSERIALRX;
                        }
                        if (i==slotsList[0]){
                            break;
                        }
                    }
                }
                if ((j==1 && i!=slotsList[0]) || (j!=1 && i!=slotsList[1])){
                    llsf_vars.sd[numCandCells].slotoffset = i;
                    llsf_vars.sd[numCandCells].distance   = slotframe_length+slotsList[0]-slotsList[j-1];
                    numCandCells++;
                    // this is the end of slotsList, break
                    break;
                }
            } else {
                i = slotsList[j]+1;
                while(schedule_isSlotOffsetAvailable(i)==FALSE || \
                      llsf_isSlotAvialable(i)==FALSE){
                    i++;
                    if (j+1 != MAX_SCHEDULED_SLOT){
                        if (i==slotsList[j+1]){
                            break;
                        }
                    } else {
                        if (i==SLOTFRAME_LENGTH){
                            i = SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS + NUMSERIALRX;
                        }
                        if (i==slotsList[0]){
                            break;
                        }
                    }
                }
                if (i!=slotsList[j+1] && i!=slotsList[0]){
                    llsf_vars.sd[numCandCells].slotoffset = i;
                    llsf_vars.sd[numCandCells].distance   = slotsList[j]-slotsList[j-1];
                    numCandCells++;
                }
            }
        }
        if (j==MAX_SCHEDULED_SLOT){
            i = slotsList[0]+1;
            while(schedule_isSlotOffsetAvailable(i)==FALSE || \
                llsf_isSlotAvialable(i)==FALSE){
                i++;
                if (i==slotsList[1]){
                    break;
                }
            }
            if (i!=slotsList[1]){
                llsf_vars.sd[numCandCells].slotoffset = i;
                llsf_vars.sd[numCandCells].distance   = slotframe_length+slotsList[0]-slotsList[j-1];
                numCandCells++;
            }
        }
        
        // order the candidate slot by distance
        for (i=0;i<MAX_SCHEDULED_SLOT;i++){
            if (llsf_vars.sd[i].distance == 0){
                // no cells
                break;
            }
            for (j=1;j<MAX_SCHEDULED_SLOT-i;j++) {
                if (llsf_vars.sd[j].distance>llsf_vars.sd[j-1].distance){
                    memcpy(&temp,&llsf_vars.sd[j-1],sizeof(slot_distance_t));
                    memcpy(&llsf_vars.sd[j-1],&llsf_vars.sd[j],sizeof(slot_distance_t));
                    memcpy(&llsf_vars.sd[j],&temp,sizeof(slot_distance_t));
                }
            }
        }
        // move candidate slot to cellList
        for (i=0;i<MAX_SCHEDULED_SLOT;i++){
            if (llsf_vars.sd[i].distance == 0){
                break;
            }
            cellList[i].tsNum = llsf_vars.sd[i].slotoffset;
            cellList[i].choffset = 0;
            cellList[i].linkoptions = CELLTYPE_TX;
        }
        
        printf("LLSF : rxSlotList\n");
        for (i=0;i<MAX_SCHEDULED_SLOT;i++){
            printf("%d ",slotsList[i]);
        }
        printf("\n");
        printf("LLSF: cellList\n");
        for (i=0;i<SCHEDULEIEMAXNUMCELLS;i++){
            printf("%d ",cellList[i].tsNum);
        }
        printf("\n");
        return TRUE;
    }
}

bool llsf_generateLowLantencySlots_remove(
     uint16_t * txSlotsList, 
     uint16_t * rxSlotsList, 
     cellInfo_ht* cellList,
     open_addr_t* neighbor,
     uint16_t numOfCells 
){
    uint16_t i,j,temp;
    frameLength_t slotframe_length;
    uint8_t numCandidateCells;
    uint16_t minDistance[MAX_SCHEDULED_SLOT];
    uint16_t maxDistance;
    
    memset(cellList,0,sizeof(cellList));
    memset(minDistance,0xff,sizeof(minDistance));
    
    slotframe_length = schedule_getFrameLength();
    array_sort(txSlotsList);
    array_sort(rxSlotsList);
    
    numCandidateCells=0;
    if (rxSlotsList[0] == 0xffff) {
        // there is no rx cell in the schedule (no children)
        // any tx cell can be removed
        for(i=0;i<MAX_SCHEDULED_SLOT;i++){
            if (txSlotsList[i]!=0xffff){
                cellList[numCandidateCells].tsNum = txSlotsList[i];
                cellList[numCandidateCells].choffset = 0;
                cellList[numCandidateCells].linkoptions = CELLTYPE_TX;
                numCandidateCells++;
            }
            if (numCandidateCells == SCHEDULEIEMAXNUMCELLS ||\
                numCandidateCells == numOfCells){
                break;
            }
        }
     
        if(numCandidateCells==0){
           return FALSE;
        }else{
           return TRUE;
        }
    } else {
        // remove the tx cell which has the longest distance 
        // after the nearest previous rx cell. 
        maxDistance = 0;
        // step1: find the minimal distance for each tx slot
        for (i=0;i<MAX_SCHEDULED_SLOT;i++){
            if (txSlotsList[i]==0xffff){
                // no tx cell any more
                break;
            }
            for (j=0;j<MAX_SCHEDULED_SLOT;j++){
                if (txSlotsList[i]<rxSlotsList[j]){
                    if (slotframe_length+txSlotsList[i]-rxSlotsList[j]<minDistance[i]){
                        minDistance[i] = slotframe_length+txSlotsList[i]-rxSlotsList[j];
                    }
                } else {
                    if (txSlotsList[i]-rxSlotsList[j]<minDistance[i]){
                        minDistance[i] = txSlotsList[i]-rxSlotsList[j];
                    }
                }
            }
        }
        // step2: choose the candidate slots to be removed, which has the 
        // largest minimal distance.
        for (i=0;i<MAX_SCHEDULED_SLOT;i++){
            if (minDistance[i]==0xffff){
                break;
            }
            maxDistance = 0;
            for (j=0;j<MAX_SCHEDULED_SLOT;j++){
                if (minDistance[j] > maxDistance && minDistance[j]!=0xffff){
                    temp = j;
                    maxDistance = minDistance[j];
                }
            }
            minDistance[temp] = 0xffff;
            cellList[numCandidateCells].tsNum = txSlotsList[temp];
            cellList[numCandidateCells].choffset = 0;
            cellList[numCandidateCells].linkoptions = CELLTYPE_TX;
            numCandidateCells++;
            if (numCandidateCells == SCHEDULEIEMAXNUMCELLS ||\
                numCandidateCells == numOfCells){
                break;
            }
        }
        if(numCandidateCells==0){
           return FALSE;
        }else{
           return TRUE;
        }
    }
}

//=================================== private ==================================
bool llsf_isSlotAvialable(uint16_t slot){
    uint8_t i;
    bool returnVal;
    
    returnVal = TRUE;
    for (i=0;i<MAX_SCHEDULED_SLOT;i++){
        if (llsf_vars.sd[i].distance != 0){
            if (slot == llsf_vars.sd[i].slotoffset){
                returnVal = FALSE;
                break;
            }
        }
    }
    return returnVal;
}

//=================================== helper ===================================
void array_sort(uint16_t * array){
    uint16_t i,j; 
    uint16_t temp;
    // bubble  sort: array[0...n] -> small slotoffset...large slotoffset,0..0
    for (i=0;i<sizeof(array);i++){
        for (j=1;j<sizeof(array)-i;j++){
            if (array[j]<array[j-1]){
                temp = array[j-1];
                array[j-1] = array[j];
                array[j] = temp;
            }
        }
    }
}

void stack_push(cellInfo_ht* cellList,uint16_t slot){
    uint8_t i,j;
    i = SCHEDULEIEMAXNUMCELLS-1;
    if (cellList[i].linkoptions != CELLTYPE_OFF) {
        // the cellList is full
        // log error
        return;
    }
    while(i>0){
        i -= 1;
        if (cellList[i].linkoptions != CELLTYPE_OFF) {
            for(j=0;j<=i;j++){
                cellList[i-j+1].tsNum       = cellList[i-j].tsNum;
                cellList[i-j+1].choffset    = cellList[i-j].choffset;
                cellList[i-j+1].linkoptions = cellList[i-j].linkoptions;
            }
            cellList[0].tsNum = slot;
            cellList[0].choffset = 0;
            cellList[0].linkoptions = CELLTYPE_TX;
            break;
        }
    }
    if (cellList[0].linkoptions == CELLTYPE_OFF){
        // no cell in the list yet
        cellList[0].tsNum = slot;
        cellList[0].choffset = 0;
        cellList[0].linkoptions = CELLTYPE_TX;
    }
}