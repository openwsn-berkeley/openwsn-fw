#include "opendefs.h"
#include "sfloc.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "idmanager.h"
#include "openapps.h"
#include "openserial.h"
#include "neighbors.h"
#include "openqueue.h"
#include "packetfunctions.h"

//=========================== definition =====================================


#define _DEBUG_SFLOC_       1


//=========================== variables =======================================

sfloc_vars_t sfloc_vars;

//=========================== prototypes ======================================

void sfloc_addCell_task(void);
void sfloc_removeCell_task(void);
void sfloc_bandwidthEstimate_task(void);

//=========================== public ==========================================

void sfloc_init(void) {
    memset(&sfloc_vars,0,sizeof(sfloc_vars_t));
    sfloc_vars.numAppPacketsPerSlotFrame = 0;
}

void sfloc_notif_addedCell(void) {
    openserial_printError(
            COMPONENT_SFLOC,
            ERR_GENERIC,
            (errorparameter_t)10,
            (errorparameter_t)1);

    //   scheduler_push_task(sfloc_addCell_task,TASKPRIO_SF0);
}

void sfloc_notif_removedCell(void) {
    openserial_printError(
            COMPONENT_SFLOC,
            ERR_GENERIC,
            (errorparameter_t)10,
            (errorparameter_t)2);

    //   scheduler_push_task(sfloc_removeCell_task,TASKPRIO_SF0);
}

// this function is called once per slotframe. 
void sfloc_notifyNewSlotframe(void) {
    //scheduler_push_task(sfloc_verifSchedule,TASKPRIO_SF0);
}

//=========================== private =========================================



//to reserve one cell toward my parent (for control packets) if none exists
//returns TRUE if a reservation was triggered, FALSE if nothing was required
bool sfloc_reserveParentCells_controlTrack(void){
   open_addr_t parent;
   track_t      sixtopTrack;
   uint8_t      nbCells;
   uint8_t      i;

#if (TRACK_MGMT != TRACK_MGMT_6P_ISOLATION)
   return(FALSE);
#endif

   //when 6top will have finished, sfloc will ask for bandwidth for this packet (if required)
   if (!sixtop_isIdle())
      return(0);

   //Do I have a valid parent?
   if (!icmpv6rpl_getPreferredParentIndex(&i)){
       openserial_printCritical(
               COMPONENT_SFLOC,ERR_UNKNOWN_NEIGHBOR,
               (errorparameter_t)parent.addr_64b[6],
               (errorparameter_t)parent.addr_64b[7]
       );
       return FALSE;
   }

   //DAGroot
   if (idmanager_getIsDAGroot())
       return FALSE;

   //parent addr
   neighbors_getNeighborEui64(&parent, ADDR_64B, i);

   //the specific track for 6P Link Requests
   sixtopTrack = sixtop_get_trackcontrol();

   //how many cells for TRACK_PARENT_CONTROL?
   nbCells = schedule_getNbCellsWithTrack(sixtopTrack, &parent);

   //ask 6top to reserve one new cell if none exists
   if (nbCells == 0){
      sixtop_setHandler(SIX_HANDLER_SFLOC);
      sixtop_request(
            IANA_6TOP_CMD_ADD,
            &parent,
            1,
            sixtopTrack,           //SFLOC -> control track to exchange 6P transactions
            NULL                   // any cells
         );

#ifdef _DEBUG_SFLOC_
      char        str[150];
      sprintf(str, "reservation triggered (control cell with the parent ");
      openserial_ncat_uint8_t_hex(str, parent.addr_64b[6], 150);
      openserial_ncat_uint8_t_hex(str, parent.addr_64b[7], 150);
      strncat(str, ") ",150 );
      openserial_printf(COMPONENT_SFLOC, str, strlen(str));
#endif

      return(TRUE);
   }

   return(FALSE);
}






//======= SF-Theo (Scheduling Function)
// Convergence Condition: NBCells for one track >= Nb of packets in the queue for that track


//asks 6top to reserve a cell if we don't have enough for this packet
//returns the number of cells asked to 6top
uint8_t sfloc_reserve_agressive_for(OpenQueueEntry_t* msg){
   uint8_t nbCells_curr, nbCells_req, nbCells_toadd;


   //when 6top will have finished, sfloc will ask for bandwidth for this packet (if required)
   if (!sixtop_isIdle())
       return(0);

   // track 0 -> only periodical, no sixtop request
   if (msg->l2_track.instance == TRACK_BESTEFFORT)
      return(0);


   // requested and current allocations
   nbCells_curr   = schedule_getNbCellsWithTrack(msg->l2_track, &(msg->l2_nextORpreviousHop));
   nbCells_req    = openqueue_count_track(msg->l2_track);

   //track control: only one cell is required
   if (msg->l2_track.instance == TRACK_PARENT_CONTROL)
       nbCells_req = 1;

   //the current allocation is correct
   if (nbCells_curr >= nbCells_req)
      return(0);

#ifdef _DEBUG_SFLOC_
   char str[150];
   snprintf(str, 150, ", current=");
   openserial_ncat_uint32_t(str, (uint32_t)nbCells_curr, 150);
   strncat(str, ", required=", 150);
   openserial_ncat_uint32_t(str, (uint32_t)nbCells_req, 150);
   strncat(str, ", track instance=", 150);
   openserial_ncat_uint32_t(str, (uint32_t)msg->l2_track.instance, 150);
   strncat(str, ", track owner=", 150);
   openserial_ncat_uint8_t_hex(str, msg->l2_track.owner.addr_64b[6], 150);
   openserial_ncat_uint8_t_hex(str, msg->l2_track.owner.addr_64b[7], 150);
   openserial_printf(COMPONENT_SFLOC, str, strlen(str));
#endif

   if (msg->l2_track.instance == TRACK_PARENT_CONTROL){
      openserial_printError(
               COMPONENT_SFLOC,
               ERR_GENERIC,
               (errorparameter_t)23,
               (errorparameter_t)46
         );
      return(0);
   }

   //request to sixtop
   nbCells_toadd = nbCells_req - nbCells_curr;

    //upper bound the nb of cells (at most SIXTOP_NBCELLS_INREQ in the sixtop request)
   if (nbCells_toadd > SIXTOP_NBCELLS_INREQ)
      nbCells_toadd = SIXTOP_NBCELLS_INREQ;

   //debug
#ifdef _DEBUG_SFLOC_
   openserial_printError(
         COMPONENT_SFLOC,
         ERR_SFLOC_INSUFFICIENT,
         (errorparameter_t)(uint16_t)(msg->l2_track.instance),
         (errorparameter_t)nbCells_toadd
   );
#endif

   //ask 6top the required number of cells (any slotOffset/ChannelOffset)
   sixtop_setHandler(SIX_HANDLER_SFLOC);
   sixtop_request(IANA_6TOP_CMD_ADD, &(msg->l2_nextORpreviousHop), nbCells_toadd, msg->l2_track, NULL);
   return(nbCells_req - nbCells_curr);


}


//aggressive allocation: walks in openqueue and verifies enough cells are schedules to empty the queue during the slotframe
void sfloc_addCells_agressive(void){
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
          if(sfloc_reserve_agressive_for(msg) > 0)
             return;
  }
}


//======= TIMEOUTED (unused) Cells

//verifies that all the neighbors in CELL_TX are my parents
void sfloc_remove_obsolete_parents(void){
#ifndef SFLOC_REMOVE_OBSOLETE_PARENTS
   return;
#endif

   //TODO
   return;



   scheduleEntry_t  *cell;
   uint8_t          i;
   open_addr_t      parent;

   //parent address
   icmpv6rpl_getPreferredParentEui64(&parent);

   //no ongoing 6top transaction
   if (!sixtop_isIdle())
       return;

   uint16_t nbSlots = schedule_getMaxActiveSlots();

   //for each cell in the schedule
   for (i=0;i<nbSlots;i++){
      cell = schedule_getCell(i);

      //if this cell is in TX mode, it must be toward my parent
      if (cell->type == CELLTYPE_TX && !packetfunctions_sameAddress(&(cell->neighbor), &parent)) {

#ifdef _DEBUG_SFLOC_
          char str[150];
          sprintf(str, "SFLOC LinkRem(oldParent)=");
          openserial_ncat_uint8_t_hex(str, (uint8_t)(cell->neighbor.addr_64b[6]), 150);
          openserial_ncat_uint8_t_hex(str, (uint8_t)(cell->neighbor.addr_64b[7]), 150);
          strncat(str, ",slotOffset=", 150);
          openserial_ncat_uint32_t(str, (uint32_t)cell->slotOffset, 150);
          strncat(str, ",pos=", 150);
          openserial_ncat_uint32_t(str, (uint32_t)i, 150);
          openserial_printf(COMPONENT_SFLOC, str, strlen(str));
#endif

          //silently removed (we changed our parent, we cannot notify it probably)
          //these cells will be removed after a timeout from the receiver
          schedule_removeActiveSlot(
                  cell->slotOffset,
                  &(cell->neighbor)
          );

      }
   }
}

void sfloc_remove_unused_cells(void){

#ifndef SFLOC_REMOVE_UNUSED_CELLS
   return;
#endif

   //TODO
   return;

   //no ongoing 6top transaction
   if (!sixtop_isIdle())
      return;

   scheduleEntry_t  *cell;
   uint8_t          i;
   uint32_t         timeout;
   uint16_t         nbSlots = schedule_getMaxActiveSlots();

   //for each cell in the schedule
   for (i=0;i<nbSlots;i++){
      cell = schedule_getCell(i);

      //different timeouts depending on the cell type (RX > TX to avoid inconsistencies)
      switch (cell->type){
         case CELLTYPE_TX:
            timeout = SFLOC_CELL_TIMEOUT_TX;       // 3DAO
            break;
         case CELLTYPE_TXRX:
         case CELLTYPE_RX:
            timeout = SFLOC_CELL_TIMEOUT_RX;       // 2DAO
            break;
         default:
            break;

      }
      switch (cell->type){
         case CELLTYPE_TX:
         case CELLTYPE_RX:
         case CELLTYPE_TXRX:

            //ASN in nb of slots, timeout in ms, slotduration in us
            //TRACK_PARENT_CONTROL -> used for DAO. If nothing has been txed / rcvd -> Problem (Period DAO < Timeout)
             if (
                     ((uint32_t)ieee154e_asnDiff(&(cell->lastUsedAsn))*TSLOTDURATION_MS > timeout) &&
                     (cell->neighbor.type == ADDR_64B)
             ){


                 //silently removed (RX cell)
                 if (cell->type == CELLTYPE_RX){
#ifdef _DEBUG_SFLOC_
                     char str[150];
                     sprintf(str, "SFLOC LinkRem(silent)=");
                     openserial_ncat_uint8_t_hex(str, (uint8_t)cell->neighbor.addr_64b[6], 150);
                     openserial_ncat_uint8_t_hex(str, (uint8_t)cell->neighbor.addr_64b[7], 150);
                     strncat(str, ",slotOffset=", 150);
                     openserial_ncat_uint32_t(str, (uint32_t)cell->slotOffset, 150);
                     strncat(str, ", asnDiff=", 150);
                     openserial_ncat_uint32_t(str, (uint32_t)(ieee154e_asnDiff(&(cell->lastUsedAsn))), 150);
                     openserial_printf(COMPONENT_SFLOC, str, strlen(str));
#endif

                     schedule_removeActiveSlot(cell->slotOffset, &(cell->neighbor));
                 }
                 //sends a 6P request
                 else {
                    return;

#ifdef _DEBUG_SFLOC_
                     char str[150];
                     sprintf(str, "SFLOC LinkRem(unused)=");
                     openserial_ncat_uint8_t_hex(str, (uint8_t)cell->neighbor.addr_64b[6], 150);
                     openserial_ncat_uint8_t_hex(str, (uint8_t)cell->neighbor.addr_64b[7], 150);
                     strncat(str, ",slotOffset=", 150);
                     openserial_ncat_uint32_t(str, (uint32_t)cell->slotOffset, 150);
                     strncat(str, ", asnDiff=", 150);
                     openserial_ncat_uint32_t(str, (uint32_t)(ieee154e_asnDiff(&(cell->lastUsedAsn))), 150);
                     openserial_printf(COMPONENT_SFLOC, str, strlen(str));
#endif

                     sixtop_setHandler(SIX_HANDLER_SFLOC);
                     sixtop_request(
                             IANA_6TOP_CMD_DELETE,
                             &(cell->neighbor),
                             1,
                             sixtop_get_trackbesteffort(),           //SFLOC -> don't care of the track when a cell is REMOVED
                             cell
                     );

                     //at most one request at a time
                     return;
                 }
             }

             break;

         default:
            break;

      }
   }

}





//======= Periodic or Event-triggered Verifications in SFLOC


//can a linkReq be generated or should we wait some conditions?
bool sfloc_verifPossible(void){
   uint8_t      i;

   //I MUST be idle
   if (!sixtop_isIdle())
      return FALSE;


   //I am the DAGrooot -> nothing to prepare
   if (idmanager_getIsDAGroot()==TRUE)
      return (TRUE);

   // I must have a valid parent
   if (!icmpv6rpl_getPreferredParentIndex(&i))
       return FALSE;

   // to reserve one cell in the control_track for 6P toward my parent if none was allocated
#if (TRACK_MGMT == TRACK_MGMT_6P_ISOLATION)
   if (sfloc_reserveParentCells_controlTrack())
      return FALSE;
#endif


   return TRUE;
}


//updates the schedule
void sfloc_verifSchedule(void){

   //must some actions be triggered before reserving new cells?
   if (!sfloc_verifPossible())
      return;

#if (TRACK_MGMT > TRACK_MGMT_NO)
   sfloc_addCells_agressive();              // insert new cells in the schedule if we have too many packets in the queue for a given track
   sfloc_remove_obsolete_parents();         // do some cells exist toward an old parent?
   sfloc_remove_unused_cells();             // remove the cells not used for a while
#endif
}



//a packet is pushed to the MAC layer -> SFLOC notification
void sfloc_notif_pktTx(OpenQueueEntry_t* msg){

   //must some actions be triggered before reserving new cells?
   if (!sfloc_verifPossible())
      return;

   //remove inconsistencies in the schedule
   sfloc_verifSchedule();

#if (TRACK_MGMT > TRACK_MGMT_NO)
   sfloc_reserve_agressive_for(msg);
#endif
}






void sfloc_appPktPeriod(uint8_t numAppPacketsPerSlotFrame){
    sfloc_vars.numAppPacketsPerSlotFrame = numAppPacketsPerSlotFrame;
}


