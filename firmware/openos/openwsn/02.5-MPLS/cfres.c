/**
 \brief Completely Fair Reservation impl

Bridge between L2.5 and L2 used to set the schedule in neighbours using the information provided by GMPLS.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
 */

#include "cfres.h"
#include "rsvp.h"
#include "schedule.h"


//given a label objects computes the schedule and modifies it.
void updateSchedule(rsvp_label_object_t* label,open_addr_t* neighbor){
   //the algorithm works as follow:
   //1-find all the cycles where the node participate. This depends on its order. if its order is higher than  num_participants it does not participate.
   slotOffset_t    slotOffset;
   cellType_t      type;
    bool            shared=FALSE;

    uint8_t j=0;
    uint8_t i=0;
   uint8_t delay=label->level;
   uint8_t ordering=label->ordering;
   uint8_t freq_offset=label->freq_offset;
   cycle_table_tuple_t* cycle_table=&(label->cycle_table.cycles[0]);
   uint8_t num_participants;
   uint8_t num_cycles;

   while (i<RSVP_MAX_SCHED){
      num_participants=(*(cycle_table+i)).num_participants;
      num_cycles=(*(cycle_table+i)).num_cycles;

      if (ordering<=num_participants){
         //something to schedule. the schedule is set at slots separated by number of participants*2 slots
         j=0;
         while (j<delay+2*(num_participants)*(num_cycles)){//insert as much slots as num cycles this node participates in.
            slotOffset=delay+1+2*(ordering-1)+j; //compute the ordering
            schedule_addActiveSlot(slotOffset,type,shared,freq_offset,  neighbor); //insert the schedule slot
            j+=2*(num_participants);//jump 2*num_participants slots.
         }
      }
      i++;
   }
}

