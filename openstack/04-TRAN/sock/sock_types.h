#ifndef OPENWSN_SOCK_TYPES_H
#define OPENWSN_SOCK_TYPES_H

#include "sock.h"
#include "async_types.h"

typedef struct _socket socket_t;

/**
 * @brief   A Common IP-based transport layer endpoint
 */
struct _sock_tl_ep {
    int family;
    union {
        uint8_t ipv6[16];
    } addr;

    uint16_t netif;
    uint16_t port;
};

/**
 *  @brief  Type for a UDP endpoint
 */
typedef struct _sock_tl_ep sock_udp_ep_t;

struct _socket {
    sock_udp_ep_t local;         /**< local end-point */
    sock_udp_ep_t remote;        /**< remote end-point */
    uint16_t flags;              /**< option flags */
};

struct sock_udp {
    socket_t gen_sock;                /**< Generic socket */
    sock_udp_cb_t async_cb;           /**< asynchronous callback */
    OpenQueueEntry_t* txrx;
    void* async_cb_arg;
    struct sock_udp *next;
};

#endif /* OPENWSN_SOCK_TYPES_H */
