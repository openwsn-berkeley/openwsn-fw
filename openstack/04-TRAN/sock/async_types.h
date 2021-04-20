#ifndef OPENWSN_ASYNC_TYPES_H
#define OPENWSN_ASYNC_TYPES_H

/**
 * @brief   Type for a UDP sock object
 */
typedef struct sock_udp sock_udp_t;   /**< forward declare for async */

/**
 * @brief   Flag types to signify asynchronous sock events
 */
typedef enum {
    SOCK_ASYNC_CONN_RDY  = 0x0001, /**< Connection ready event */
    SOCK_ASYNC_CONN_FIN  = 0x0002, /**< Connection finished event */
    SOCK_ASYNC_CONN_RECV = 0x0004, /**< Listener received connection event */
    SOCK_ASYNC_MSG_RECV  = 0x0010, /**< Message received event */
    SOCK_ASYNC_MSG_SENT  = 0x0020, /**< Message sent event */
    SOCK_ASYNC_PATH_PROP = 0x0040, /**< Path property changed event */
} sock_async_flags_t;


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
typedef void (* sock_udp_cb_t)(sock_udp_t* sock, sock_async_flags_t type, void* arg);

#endif /* OPENWSN_ASYNC_TYPES_H */
