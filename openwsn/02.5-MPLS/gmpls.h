/**
 \brief GMPLS L2.5 manager. Is in charge of L2.5 activity. Controls RSVP messages and its timming and sends to cfres RSVP objects
 containing the information to create an schedule. It can talk with uRes as well.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
 */

#ifndef __GMPLS_H
#define __GMPLS_H

#include "openwsn.h"
#include "openqueue.h"


void    mpls_init(void);
// from upper layer
error_t mpls_send(OpenQueueEntry_t *msg);
// from lower layer
void    mpls_receive(OpenQueueEntry_t* msg);

#endif
