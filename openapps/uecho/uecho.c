#include "opendefs.h"
#include "uecho.h"
#include "sock.h"
#include "async.h"
#include "openserial.h"

//=========================== variables =======================================

sock_udp_t uecho_sock;

//=========================== prototypes ======================================

void uecho_handler(sock_udp_t *sock, sock_async_flags_t type, void *arg);

//=========================== public ==========================================

void uecho_init(void) {
    // clear local variables
    memset(&uecho_sock, 0, sizeof(sock_udp_t));

    sock_udp_ep_t local;

    local.port = WKP_UDP_ECHO;

    if (sock_udp_create(&uecho_sock, &local, NULL, 0) < 0) {
        openserial_printf("Could not create socket\n");
        return;
    }

    openserial_printf("Created a UDP socket\n");

    sock_udp_set_cb(&uecho_sock, uecho_handler, NULL);
}

void uecho_handler(sock_udp_t *sock, sock_async_flags_t type, void *arg) {
    (void) arg;

    char buf[50];

    if (type & SOCK_ASYNC_MSG_RECV) {
        sock_udp_ep_t remote;
        int16_t res;

        if ((res = sock_udp_recv(sock, buf, sizeof(buf), 0, &remote)) >= 0) {
            openserial_printf("Received %d bytes from remote endpoint:\n", res);
            openserial_printf(" - port: %d", remote.port);
            openserial_printf(" - addr: ", remote.port);
            for(int i=0; i < 16; i ++)
                openserial_printf("%x ", remote.addr.ipv6[i]);

            openserial_printf("\n\n");
            openserial_printf("Msg received: %s\n\n", buf);

            if (sock_udp_send(sock, buf, res, &remote) < 0) {
                openserial_printf("Error sending reply\n");
            }
        }
    }
}
//=========================== private =========================================

