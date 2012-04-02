/**
 \brief Completely Fair Reservation headers

Bridge between L2.5 and L2 used to set the schedule in neighbours using the information provided by GMPLS.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
 */

#ifndef __CFRES_H
#define __CFRES_H

#include "rsvp.h"
#include "neighbors.h"

void updateSchedule(rsvp_label_object_t* label,open_addr_t* neighbor);

#endif
