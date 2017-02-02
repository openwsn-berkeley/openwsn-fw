/**
\brief A mechanism to calculate the slotoffset and channel offset of shared 
       cell installed at runtime based on a hash table.
\author: Tengfei Chang <tengfei.chang@inria.fr>, February 2017
*/

#include "packetfunctions.h"
#include "sfy_hashtable.h"
#include "schedule.h"
#include "idmanager.h"

//=========================== definition ======================================

//=========================== variable ========================================

sfy_hashtable_vars_t sfy_hashtable_vars;

//=========================== public ==========================================

void sfy_hashtable_init(){
    uint16_t slotOffset;
    uint8_t  channelOffset;
    open_addr_t temp_neighbor;
    
    memset(&sfy_hashtable_vars,0,sizeof(sfy_hashtable_vars_t));
    // calculate my shared slot offset
    sfy_hashtable_getCellInfo(idmanager_getMyID(ADDR_16B),&slotOffset,&channelOffset);
    sfy_hashtable_vars.sharedSlot_me = slotOffset;
    
    memset(&temp_neighbor,0,sizeof(temp_neighbor));
    temp_neighbor.type             = ADDR_ANYCAST;
    
    schedule_addActiveSlot(
         slotOffset,                 // slot offset
         CELLTYPE_TXRX,                      // type of slot
         TRUE,                               // shared?
         channelOffset,    // channel offset
         &temp_neighbor                      // neighbor
    );
}

bool sfy_hashtable_getCellInfo(
    open_addr_t* address,
    uint16_t* slotOffset,
    uint8_t*  channelOffset
    ){
    uint16_t moteId;
    if (address->type != ADDR_16B){
        // I need a 16b type address
        return FALSE;
    }
    moteId = address->addr_16b[0]*256 + address->addr_16b[1];
    if (moteId%SLOTFRAME_LENGTH<SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS+NUMSERIALRX){
        *slotOffset    = SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS+NUMSERIALRX;
    } else {
        *slotOffset    = moteId%SLOTFRAME_LENGTH;
    }
    // it may do something advanced on channel offset selection
    // e.g. channelOffset = moteId%16, then there will be a problem that
    // pair of node (parent&child) has same slotOffset but different channelOffset
    // for now, just use unique number
    *channelOffset = 0;
    return TRUE;
}

void sfy_hashtable_updateSharedslotParent(open_addr_t* address){
    uint16_t    slotOffset;
    uint8_t     channelOffset;
    open_addr_t temp_neighbor_16b;
    
    packetfunctions_mac64bToMac16b(address,&temp_neighbor_16b);
    
    sfy_hashtable_getCellInfo(&temp_neighbor_16b,&slotOffset,&channelOffset);
    
    if (slotOffset==sfy_hashtable_vars.sharedSlot_me){
        // the shared cells to parent and mine are the same
        sfy_hashtable_vars.sharedSlot_parent = slotOffset;
        return;
    }
    if (slotOffset==sfy_hashtable_vars.sharedSlot_parent){
        // new parent use the same slot as previous one
        // do nothing
    } else {
        if (schedule_addActiveSlot(
                slotOffset,
                CELLTYPE_TXRX,
                TRUE,
                channelOffset,  
                address  
            )==E_SUCCESS
        ){
            if (sfy_hashtable_vars.sharedSlot_parent!=0){
                // remove previous parent shared cell
                schedule_removeActiveSlot(
                    sfy_hashtable_vars.sharedSlot_parent,
                    &sfy_hashtable_vars.previousParent
                );
            } else {
                // this is the first time having parent.
                // no cell to remove.
            }
            // update previous parent as current parent and slotoffset
            memcpy(&sfy_hashtable_vars.previousParent,address,sizeof(open_addr_t));
            sfy_hashtable_vars.sharedSlot_parent = slotOffset;
        } else {
            // this slotoffset may be occupied by tx cell reserved to previous
            // parent, but the schedule house keeping will remove them. Finallly
            // this slot will be available. so nothing to do here.
        }
    }
}

uint16_t sfy_hashtable_getSharedSlotMe(){
    return sfy_hashtable_vars.sharedSlot_me;
}

uint16_t sfy_hashtable_getSharedSlotParent(){
    return sfy_hashtable_vars.sharedSlot_parent;
}