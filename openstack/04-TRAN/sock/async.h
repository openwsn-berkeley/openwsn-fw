#ifndef OPENWSN_ASYNC_SOCKET_H
#define OPENWSN_ASYNC_SOCKET_H

#include "sock.h"


void sock_udp_set_cb(sock_udp_t* sock, sock_udp_cb_t cb, void* cb_arg);

#endif /* OPENWSN_ASYNC_SOCKET_H */
