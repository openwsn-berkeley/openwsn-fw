/**
\brief Generic TCP client, Windows-style

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#ifndef __TCP_PORT_H
#define __TCP_PORT_H

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

SOCKET tcp_port_connect(char* server_name, unsigned short server_port);

#endif