#include "config.h"

#if defined(OPENWSN_UDP_C)

#include "opendefs.h"
#include "sock.h"
#include "af.h"
#include "errno.h"
#include "packetfunctions.h"
#include "openqueue.h"
#include "udp.h"
#include "openrandom.h"
#include "idmanager.h"
#include "scheduler.h"
#include "openserial.h"

// ============================ defines ========================================

sock_udp_t* udp_socket_list;

// =========================== variables =======================================
// =========================== prototypes ======================================

static bool _sock_valid_af(uint8_t af);

static bool _sock_valid_addr(sock_udp_ep_t* ep);

static void _sock_get_local_addr(open_addr_t* local);

static void _sock_transmit_internal(void);

// ============================= public ========================================

void sock_udp_init(void) {
    udp_socket_list = NULL;
}

int sock_udp_create(sock_udp_t* sock, const sock_udp_ep_t* local, const sock_udp_ep_t* remote, uint16_t flags) {
    sock_udp_t* current;

    if (sock == NULL) {
        return -EINVAL;
    }

    memset(&sock->gen_sock.local, 0, sizeof(sock_udp_ep_t));

    if (local != NULL) {
        current = udp_socket_list;

        while (current != NULL) {
            if (current->gen_sock.local.port == local->port) {
                return -EADDRINUSE;
            }

            current = current->next;
        }

        memcpy(&sock->gen_sock.local, local, sizeof(sock_udp_ep_t));
    }

    memset(&sock->gen_sock.remote, 0, sizeof(sock_udp_ep_t));

    if (remote != NULL) {
        if (_sock_valid_af(remote->family) == FALSE) {
            return -EAFNOSUPPORT;
        }

        if (_sock_valid_addr((sock_udp_ep_t*)remote) == FALSE) {
            return -EINVAL;
        }

        memcpy(&sock->gen_sock.remote, remote, sizeof(sock_udp_ep_t));
    }

    sock->gen_sock.flags = flags;
    sock->async_cb = NULL;

    sock->next = udp_socket_list;
    udp_socket_list = sock;

    return 0;
}

int sock_udp_send(sock_udp_t* sock, const void* data, size_t len, const sock_udp_ep_t* remote) {
    OpenQueueEntry_t* pkt;

    if (sock == NULL && remote == NULL) {
        return -EINVAL;
    }

    if (data == NULL && len != 0) {
        return -EINVAL;
    }

    if ((pkt = openqueue_getFreePacketBuffer(COMPONENT_SOCK_TO_UDP)) == NULL) {
        return -ENOMEM;
    }

    if (remote != NULL) {
        if (remote->port == 0) {
            return -EINVAL;
        }

        if (_sock_valid_af(remote->family) == FALSE) {
            return -EAFNOSUPPORT;
        }

        if (_sock_valid_addr((sock_udp_ep_t*)remote) == FALSE) {
            return -EINVAL;
        }

        pkt->l3_destinationAdd.type = ADDR_128B;
        memcpy(&pkt->l3_destinationAdd.addr_128b, &remote->addr, LENGTH_ADDR128b);

        pkt->l4_destination_port = remote->port;

        if (sock != NULL) {
            pkt->l4_sourcePortORicmpv6Type = sock->gen_sock.local.port;
        } else {
            pkt->l4_sourcePortORicmpv6Type = openrandom_get16b();
        }
    } else if (sock != NULL) {
        pkt->l3_destinationAdd.type = ADDR_128B;
        memcpy(&pkt->l3_destinationAdd.addr_128b, &sock->gen_sock.remote.addr, LENGTH_ADDR128b);

        pkt->l4_sourcePortORicmpv6Type = sock->gen_sock.local.port;
        pkt->l4_destination_port = sock->gen_sock.remote.port;
    } else if (sock == NULL) {
        return -EINVAL;
    }

    open_addr_t local;
    _sock_get_local_addr(&local);
    memcpy(&pkt->l3_sourceAdd, &local, sizeof(open_addr_t));

    pkt->owner = COMPONENT_SOCK_TO_UDP;
    pkt->creator = COMPONENT_SOCK_TO_UDP;

    if (packetfunctions_reserveHeader(&pkt, len)) {
        openqueue_freePacketBuffer(pkt);

        return -ENOBUFS;
    }

    memcpy(pkt->payload, data, len);

    scheduler_push_task(_sock_transmit_internal, TASKPRIO_UDP);

    pkt->l4_payload = pkt->payload;
    pkt->l4_length = pkt->length;

    return len;
}

void sock_udp_close(sock_udp_t* sock) {
    sock_udp_t* temp = udp_socket_list;
    sock_udp_t* prev = udp_socket_list;

    /* check if head is the socket to be closed */
    if (temp != NULL && temp == sock) {
        udp_socket_list = temp->next;

        return;
    }

    /* search for the socket to be deleted, keep the previous one to
       be able to update next */
    while (temp != NULL && temp != sock) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        return;
    }

    /* remove socket from linked list */
    prev->next = temp->next;
}

int sock_udp_get_local(sock_udp_t* sock, sock_udp_ep_t* ep) {
    if (sock->gen_sock.local.family == AF_UNSPEC) {
        return -EADDRINUSE;
    }

    memcpy(ep, &sock->gen_sock.local, sizeof(sock_udp_ep_t));

    return 0;
}

int sock_udp_get_remote(sock_udp_t* sock, sock_udp_ep_t* ep) {
    if (sock->gen_sock.remote.family == AF_UNSPEC) {
        return -ENOTCONN;
    }

    memcpy(ep, &sock->gen_sock.remote, sizeof(sock_udp_ep_t));

    return 0;
}

int sock_udp_recv(sock_udp_t* sock, void* data, size_t max_len, uint32_t timeout, sock_udp_ep_t* remote) {
    uint16_t bytes_to_copy;
    sock_udp_ep_t ep;

    if (sock->txrx == NULL) {
        return -EINVAL;
    }

    if (max_len >= sock->txrx->l4_length) {
        bytes_to_copy = sock->txrx->l4_length;
    } else {
        bytes_to_copy = max_len;
    }

    if (remote != NULL) {
        ep.family = AF_INET6;
        ep.port = sock->txrx->l4_sourcePortORicmpv6Type;
        memcpy(&ep.addr, sock->txrx->l3_sourceAdd.addr_128b, LENGTH_ADDR128b);
        memcpy(remote, &ep, sizeof(sock_udp_ep_t));
    }

    memset(data, 0, max_len);
    memcpy(data, sock->txrx->l4_payload, bytes_to_copy);

    openqueue_freePacketBuffer(sock->txrx);

    return bytes_to_copy;
}

void sock_receive_internal(void) {
    OpenQueueEntry_t* pkt;
    sock_udp_t* current;

    pkt = openqueue_getPacketByComponent(COMPONENT_UDP_TO_SOCK);

    if (pkt == NULL) {
        openserial_printf("found nothing\n");

        return;
    }

    current = udp_socket_list;

    while (current != NULL) {
        if (current->gen_sock.local.port == pkt->l4_destination_port &&
            current->async_cb != NULL &&
            idmanager_isMyAddress(&pkt->l3_destinationAdd))
        {

            current->txrx = pkt;
            current->async_cb(current, SOCK_ASYNC_MSG_RECV, NULL);
            break;
        }

        current = current->next;
    }

    if (current == NULL) {
        openqueue_freePacketBuffer(pkt);
        openserial_printf("no associated socket found\n");
    }
}

void sock_senddone_internal(OpenQueueEntry_t* msg, owerror_t error) {
    OpenQueueEntry_t* pkt;
    sock_udp_t* current;

    pkt = openqueue_getPacketByComponent(COMPONENT_UDP);

    if (pkt == NULL) {
        openserial_printf("found nothing\n");

        return;
    }

    current = udp_socket_list;

    while (current != NULL) {
        if (current->gen_sock.local.port == pkt->l4_sourcePortORicmpv6Type &&
            current->async_cb != NULL)
        {
            current->txrx = pkt;
            current->async_cb(current, SOCK_ASYNC_MSG_SENT, &error);
            break;
        }

        current = current->next;
    }
}

void sock_udp_set_cb(sock_udp_t* sock, sock_udp_cb_t cb, void* cb_arg) {
    sock->async_cb = cb;
    sock->async_cb_arg = cb_arg;
}

// ============================= private =======================================

void _sock_transmit_internal(void) {
    OpenQueueEntry_t* pkt;

    pkt = openqueue_getPacketByComponent(COMPONENT_SOCK_TO_UDP);

    if (pkt == NULL) {
        openserial_printf("found nothing\n");

        return;
    }

    udp_transmit(pkt);
}

static bool _sock_valid_af(uint8_t af) {
    if (af == AF_INET6) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static void _sock_get_local_addr(open_addr_t* local) {
    local->type = ADDR_128B;

    open_addr_t* prefix_addr = idmanager_getMyID(ADDR_PREFIX);
    memcpy(local->addr_128b, prefix_addr->prefix, 8);

    open_addr_t* id64b_addr = idmanager_getMyID(ADDR_64B);
    memcpy(local->addr_128b + 8, id64b_addr->addr_64b, 8);
}

static bool _sock_valid_addr(sock_udp_ep_t* ep) {
    uint8_t zero_count;
    const uint8_t* p;

    p = (uint8_t* )&ep->addr;
    zero_count = 0;

    for (uint8_t i = 0; i < sizeof(ep->addr); i++) {
        if (p [i] == 0) {
            zero_count++;
        }
    }

    if (zero_count == 16) {
        return FALSE;
    }

    return TRUE;
}

#endif
