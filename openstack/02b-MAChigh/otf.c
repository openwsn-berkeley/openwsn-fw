#include "opendefs.h"
#include "otf.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "openserial.h"
#include "openqueue.h"
#include "IEEE802154E.h"
#include "idmanager.h"


//#define _DEBUG_OTF_



//=========================== variables =======================================

//=========================== prototypes ======================================

void otf_addCell_task(void);
void otf_removeCell_task(void);

//=========================== public ==========================================

void otf_init(void) {
}


//When a cell is added/removed we have nothing to do!
void otf_notif_addedCell(void) {
   openserial_printError(
         COMPONENT_OTF,
         ERR_GENERIC,
         (errorparameter_t)10,
         (errorparameter_t)1);

   //scheduler_push_task(otf_addCell_task,TASKPRIO_OTF);
}

void otf_notif_removedCell(void) {
   openserial_printError(
         COMPONENT_OTF,
         ERR_GENERIC,
         (errorparameter_t)10,
         (errorparameter_t)2);

   //scheduler_push_task(otf_removeCell_task,TASKPRIO_OTF);
}

//=========================== private =========================================

void otf_addCell_task(void) {
   open_addr_t          neighbor;
   bool                 foundNeighbor;
   
   // get preferred parent
   foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
   if (foundNeighbor==FALSE) {
      return;
   }
   
   sixtop_setHandler(SIX_HANDLER_OTF);
   // call sixtop
   sixtop_addCells(
      &neighbor,
      1,
      sixtop_get_trackbesteffort()
   );
}

//TODO: handle tracks when monitoring the OTF requests to do

void otf_removeCell_task(void) {
   open_addr_t          neighbor;
   bool                 foundNeighbor;
   
   // get preferred parent
   foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
   if (foundNeighbor==FALSE) {
      return;
   }
   
   sixtop_setHandler(SIX_HANDLER_OTF);
   // call sixtop
   sixtop_removeCell(
      &neighbor
   );
}



//======= 6P Cells



//to reserve one cell toward my parent (for control packets) if none exists
bool otf_reserveParentCells(void){
   open_addr_t parent;
   track_t     linkReqTrack;
   uint8_t     nbCells;

   //Do I have a valid parent?
   neighbors_getPreferredParentEui64(&parent);
   if (parent.type == ADDR_NONE){
      openserial_printCritical(
               COMPONENT_OTF,ERR_UNKNOWN_NEIGHBOR,
               (errorparameter_t)parent.addr_64b[6],
               (errorparameter_t)parent.addr_64b[7]
            );
      return FALSE;
   }

   //the specific track for 6P Link Requests
   memcpy(&(linkReqTrack.owner), &(parent), sizeof(linkReqTrack.owner));
   linkReqTrack.instance = TRACK_PARENT_CONTROL;

   //how many cells for TRACK_PARENT_CONTROL?
   nbCells = schedule_getNbCellsWithTrack(linkReqTrack, &parent);

   //ask 6top to reserve one new cell if none exists
   if (nbCells == 0){
      sixtop_setHandler(SIX_HANDLER_OTF);
      sixtop_addCells(&(parent), 1, linkReqTrack);

#ifdef _DEBUG_OTF_
      char        str[150];
      sprintf(str, "OTF - TRACK_PARENT_CONTROL - Has to reserve one cell with the parent ");
      openserial_ncat_uint8_t_hex(str, parent.addr_64b[6], 150);
      openserial_ncat_uint8_t_hex(str, parent.addr_64b[7], 150);
      openserial_printf(COMPONENT_OTF, str, strlen(str));
#endif

      return(TRUE);
   }

   return(FALSE);
}



//======= SF-Theo (Scheduling Function)
// Convergence Condition: NBCells for one track >= Nb of packets in the queue for that track


//asks 6top to reserve a cell if we don't have enough for this packet
//returns the number of cells asked to 6top
uint8_t otf_reserve_agressive_for(OpenQueueEntry_t* msg){
#ifdef OTF_AGRESSIVE
   uint8_t nbCells_curr, nbCells_req, nbCells_toadd;


   //when 6top will have finished, otf will ask for bandwidth for this packet (if required)
   if (!sixtop_isIdle())
       return(0);

   // track 0 -> only periodical, no sixtop requests
   if (msg->l2_track.instance == TRACK_BESTEFFORT)
      return(0);

   // requested and current allocations
   nbCells_curr   = schedule_getNbCellsWithTrack(msg->l2_track, &(msg->l2_nextORpreviousHop));
   nbCells_req    = openqueue_count_track(msg->l2_track);

   //the current allocation is correct
   if (nbCells_curr >= nbCells_req)
      return(0);

#ifdef _DEBUG_OTF_
   char str[150];
   sprintf(str, "OTF required=");
   openserial_ncat_uint32_t(str, (uint32_t)nbCells_curr >= nbCells_req, 150);
   strncat(str, ", current=", 150);
   openserial_ncat_uint32_t(str, (uint32_t)nbCells_curr, 150);
   strncat(str, ", required=", 150);
   openserial_ncat_uint32_t(str, (uint32_t)nbCells_req, 150);
   strncat(str, ", track instance=", 150);
   openserial_ncat_uint32_t(str, (uint32_t)msg->l2_track.instance, 150);
   strncat(str, ", track owner=", 150);
   openserial_ncat_uint8_t_hex(str, msg->l2_track.owner.addr_64b[6], 150);
   openserial_ncat_uint8_t_hex(str, msg->l2_track.owner.addr_64b[7], 150);
   openserial_printf(COMPONENT_OTF, str, strlen(str));
#endif

   //request to sixtop
   nbCells_toadd = nbCells_req - nbCells_curr;

    //upper bound the nb of cells (at most SIXTOP_NBCELLS_INREQ in the sixtop request)
   if (nbCells_toadd > SIXTOP_NBCELLS_INREQ)
      nbCells_toadd = SIXTOP_NBCELLS_INREQ;

   //debug
#ifdef DEBUG_OTF
   openserial_printError(
         COMPONENT_OTF,
         ERR_OTF_INSUFFICIENT,
         (errorparameter_t)(uint16_t)(msg->l2_track.instance),
         (errorparameter_t)nbCells_toadd
   );
#endif

   //ask 6top the required number of cells
   sixtop_setHandler(SIX_HANDLER_OTF);
   sixtop_addCells(&(msg->l2_nextORpreviousHop), nbCells_toadd, msg->l2_track);
   return(nbCells_req - nbCells_curr);
#endif

   return(0);
}


//aggressive allocation: walks in openqueue and verifies enough cells are schedules to empty the queue during the slotframe
void otf_addCells_agressive(void){
#ifdef OTF_AGRESSIVE
   uint8_t  i;
   OpenQueueEntry_t* msg;

   //no ongoing 6top transaction
   if (!sixtop_isIdle())
       return;

   //only one request may be transmitted through sixtop.
   //This function will be called back when sixtop has finished its reservation later for the other messages in the queue
   for (i=0;i<QUEUELENGTH;i++){
      msg = openqueue_getPacket(i);

      if(msg->owner != COMPONENT_NULL)
          if(otf_reserve_agressive_for(msg) > 0)
             return;
  }
#endif
}


//======= TIMEOUTED (unused) Cells

//verifies that all the neighbors in CELL_TX are my parents
void otf_remove_obsolete_parents(void){
   scheduleEntry_t  *cell;
   neighborRow_t    *neigh;
   uint8_t          i;

#ifndef SIXTOP_REMOVE_OBSOLETE_PARENTS
   return;
#endif

   //no ongoing 6top transaction
   if (!sixtop_isIdle())
       return;

   //for each cell in the schedule
   for (i=0;i<MAXACTIVESLOTS;i++){
      cell = schedule_getCell(i);

      //if this cell is in TX mode, it must be toward my parent
      if (cell->type == CELLTYPE_TX) {
         neigh = neighbors_getNeighborInfo(&(cell->neighbor));

         //it is not anymore a parent (or even not anymore a neighbor)
         if (neigh == NULL || neigh->parentPreference < MAXPREFERENCE){

#ifdef _DEBUG_OTF_
            char str[150];
            sprintf(str, "OTF LinkRem(oldParent)=");
            openserial_ncat_uint8_t_hex(str, (uint8_t)cell->neighbor.addr_64b[6], 150);
            openserial_ncat_uint8_t_hex(str, (uint8_t)cell->neighbor.addr_64b[7], 150);
            strncat(str, ",slotOffset=", 150);
            openserial_ncat_uint32_t(str, (uint32_t)cell->slotOffset, 150);
            openserial_printf(COMPONENT_OTF, str, strlen(str));
#endif

            sixtop_setHandler(SIX_HANDLER_OTF);
            sixtop_removeCell(&(cell->neighbor));
            break;
         }
      }
   }
}

void otf_remove_unused_cells(void){

#ifndef SIXTOP_REMOVE_UNUSED_CELLS
   return;
#endif

   //no ongoing 6top transaction
   if (!sixtop_isIdle())
      return;

   scheduleEntry_t  *cell;
   uint8_t           i;
   uint16_t          timeout;

   //for each cell in the schedule
   for (i=0;i<MAXACTIVESLOTS;i++){
      cell = schedule_getCell(i);

      //different timeouts depending on the cell type (RX > TX to avoid inconsistencies)
      switch (cell->type){
         case CELLTYPE_TX:
            timeout = SIXTOP_CELL_TIMEOUT_TX;
            break;
         case CELLTYPE_RX:
            timeout = SIXTOP_CELL_TIMEOUT_RX;
            break;
         default:
            break;

      }
      switch (cell->type){
         case CELLTYPE_TX:
         case CELLTYPE_RX:

            //ASN in nb of slots, timeout in ms, slotduration in us
            if ((cell->track.instance != TRACK_PARENT_CONTROL) && (ieee154e_asnDiff(&(cell->lastUsedAsn)) > 1000 * timeout / TsSlotDuration)){

#ifdef _DEBUG_OTF_
               char str[150];
               sprintf(str, "OTF LinkRem(unused)=");
               openserial_ncat_uint8_t_hex(str, (uint8_t)cell->neighbor.addr_64b[6], 150);
               openserial_ncat_uint8_t_hex(str, (uint8_t)cell->neighbor.addr_64b[7], 150);
               strncat(str, ",slotOffset=", 150);
               openserial_ncat_uint32_t(str, (uint32_t)cell->slotOffset, 150);
               openserial_printf(COMPONENT_OTF, str, strlen(str));
#endif

               sixtop_setHandler(SIX_HANDLER_OTF);
               sixtop_removeCell(&(cell->neighbor));

               //at most one request at a time
               return;
            }
            break;

         default:
            break;

      }

   //TODO: remove also entries in the future -> means a reboot of the DAG and a resynchronization probably
   }

}





//======= Periodic or Event-triggered Verifications in OTF


//can a linkReq be generated or should we wait some conditions?
bool otf_verifPossible(void){
   open_addr_t parent;

   //I MUST be idle
   if (!sixtop_isIdle())
      return FALSE;


   //I am the DAGrooot -> nothing to prepare
   if (idmanager_getIsDAGroot()==TRUE)
      return (TRUE);

   // I must have a valid parent
   neighbors_getPreferredParentEui64(&parent);
    if (parent.type == ADDR_NONE)
      return FALSE;

   // to reserve one cell for 6P toward my parent if none was allocated
#if (TRACK_MGMT == TRACK_MGMT_6P_ISOLATION)
   if (otf_reserveParentCells())
      return FALSE;

#endif
   return TRUE;
}


//updates the schedule
void otf_verifSchedule(void){

   //must some actions be triggered before reserving new cells?
   if (!otf_verifPossible())
      return;

#if (TRACK_MGMT > TRACK_MGMT_NO)

#ifdef OTF_AGRESSIVE
   otf_addCells_agressive();
#endif

   otf_remove_obsolete_parents();
   otf_remove_unused_cells();
#endif
}



//a packet is pushed to the MAC layer -> OTF notification
void otf_notif_pktTx(OpenQueueEntry_t* msg){

   //must some actions be triggered before reserving new cells?
   if (!otf_verifPossible())
      return;


#if (TRACK_MGMT > TRACK_MGMT_NO)
#ifdef OTF_AGRESSIVE
   otf_reserve_agressive_for(msg);
#endif
#endif
}




//the parent has changed, must now remove the corresponding cells
void otf_notif_parentRemoved(open_addr_t *parent){
#ifndef SIXTOP_REMOVE_OBSOLETE_PARENTS
   return;
#endif

   //cannot remove an old parent if we have an on-going 6top negotiation
   if (!sixtop_isIdle()){
      char str[150];
       sprintf(str, "cannot remove cells to old parent ");
       openserial_ncat_uint8_t_hex(str, (uint8_t)parent->addr_64b[6], 150);
       openserial_ncat_uint8_t_hex(str, (uint8_t)parent->addr_64b[7], 150);
       strncat(str, ": I am not idle", 150);
       openserial_printf(COMPONENT_OTF, str, strlen(str));

      return;
   }


#if (TRACK_MGMT > TRACK_MGMT_NO)
#ifdef _DEBUG_OTF_
   char str[150];
   sprintf(str, "remove cells to old parent ");
   openserial_ncat_uint8_t_hex(str, (uint8_t)parent->addr_64b[6], 150);
   openserial_ncat_uint8_t_hex(str, (uint8_t)parent->addr_64b[7], 150);
   openserial_printf(COMPONENT_OTF, str, strlen(str));
#endif

   sixtop_setHandler(SIX_HANDLER_OTF);
   sixtop_removeCell(parent);
#endif
}




