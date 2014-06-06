/**
 \brief GMPLS L2.5 manager. Is in charge of L2.5 activity. Controls RSVP messages and its timing and sends to cfres RSVP objects
 containing the information to create an schedule. It can talk with uRes as well.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
 */

#include "gmpls.h"
#include "stdint.h"
#include "openwsn.h"
#include "opentimers.h"

typedef struct {
   uint16_t        refreshperiod;
   BOOL            busySending;          // TRUE when busy sending a rsvp message
   opentimer_id_t  timerId;
} mpls_vars_t;

mpls_vars_t mpls_vars;

void    mpls_timer_cb();

void    mpls_init(){
   mpls_vars.refreshperiod = 60000; // fires every 10 s on average
   mpls_vars.busySending       = FALSE;
   mpls_vars.timerId = opentimers_start(mpls_vars.refreshperiod,TIMER_PERIODIC,TIME_MS, mpls_timer_cb);
}

// from upper layer or from the timer. Triggers a reservation request. Genereates a rsv msg or in contrast forwards path message.
error_t mpls_send(OpenQueueEntry_t *msg){
return 0;
}


/*
 *  from lower layer.
 *  get the message. Determine type.
 *  if path message, trigger cfres to modify the scheduling.
 *  send path messsage down to the next neighbour.
 *
 *  if resv message, aggregate demand from neighbours. and go up.
 */
void  mpls_receive(OpenQueueEntry_t* msg){

}

void mpls_timer_cb(){
   //TODO
}
