/**
\brief Client program to the OpenSim server.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#ifndef __OPENSIM_CLIENT_H
#define __OPENSIM_CLIENT_H

//=========================== define ==========================================

#define DEFAULT_SERVER_NAME  "localhost"
#define DEFAULT_SERVER_PORT  14159
#define OPENCLIENT_BUFSIZE   150

//=========================== typedef =========================================
// include headers based on OS

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef int SOCKET;
#define INVALID_SOCKET -1         // WinSock invalid socket
#define SOCKET_ERROR   -1         // basic WinSock error
#define closesocket(s) close(s);  // Unix uses file descriptors, WinSock doesn't...
#define WSAGetLastError()  -1     //nothing
#define WSACleanup()              //nothing

//=========================== prototypes ======================================

void opensim_client_send(
   int   txPacketType,
   int*  txPacketParamsBuf,
   int   txPacketParamsLength
);

void opensim_client_waitForPacket(
   int*  rxPacketType,
   char* rxPacketParamsBuf,
   int   rxPacketParamsMaxLength,
   int*  rxPacketParamsLength
);

void opensim_client_sendAndWaitForAck(
   int   txPacketType,
   int*  txPacketParamsBuf,
   int   txPacketParamsLength,
   int*  rxPacketParamsBuf,
   int   rxPacketParamsExpectedLength
);

void opensim_client_abort();

#endif
