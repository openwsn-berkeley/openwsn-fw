/**
\brief Generic TCP client, Windows-style

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#ifndef __TCP_PORT_H
#define __TCP_PORT_H

//=========================== define ==========================================

#ifdef LINUX //the socket object does not exist in linux. it is an int pointing to the socket fd
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define closesocket(s) close(s);
#endif
//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

SOCKET tcp_port_connect(char* server_name, unsigned short server_port);

#endif