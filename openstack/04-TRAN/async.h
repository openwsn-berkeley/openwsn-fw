#ifndef OPENWSN_ASYNC_SOCKET_H
#define OPENWSN_ASYNC_SOCKET_H

#include "sock.h"

/**
 * @brief   Event callback for @ref sock_udp_t
 *
 * @pre `(sock != NULL)`
 *
 * @param[in] sock  The sock the event happened on
 * @param[in] flags The event flags. Expected values are
 *                  - @ref SOCK_ASYNC_MSG_RECV,
 *                  - @ref SOCK_ASYNC_MSG_SENT,
 *                  - @ref SOCK_ASYNC_PATH_PROP, or
 *                  - a combination of them.
 * @param[in] arg   Argument provided when setting the callback using
 *                  @ref sock_udp_set_cb(). May be NULL.
 */

void sock_udp_set_cb(sock_udp_t* sock, sock_udp_cb_t cb, void* cb_arg);

#endif /* OPENWSN_ASYNC_SOCKET_H */
