#ifndef OPENWSN_SOCK_INTERNAL_H
#define OPENWSN_SOCK_INTERNAL_H

#include "opendefs.h"

void sock_receive_internal(void);

void sock_senddone_internal(OpenQueueEntry_t* msg, owerror_t error);

#endif /* OPENWSN_SOCK_INTERNAL_H */
