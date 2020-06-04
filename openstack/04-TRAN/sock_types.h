#ifndef OPENWSN_SOCK_TYPES_H
#define OPENWSN_SOCK_TYPES_H

#include "sock.h"

struct _socket {
    sock_udp_ep_t local;         /**< local end-point */
    sock_udp_ep_t remote;        /**< remote end-point */
    uint16_t flags;              /**< option flags */
};

typedef struct _socket socket_t;

struct sock_udp {
    socket_t gen_sock;                /**< Generic socket */
    sock_udp_cb_t async_cb;           /**< asynchronous callback */
    OpenQueueEntry_t* txrx;
    void* async_cb_arg;
    struct sock_udp *next;
};

#endif /* OPENWSN_SOCK_TYPES_H */
