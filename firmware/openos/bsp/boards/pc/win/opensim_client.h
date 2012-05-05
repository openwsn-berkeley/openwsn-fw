/**
\brief Client program to the OpenSim server.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#ifndef __OPENSIM_CLIENT_H
#define __OPENSIM_CLIENT_H

#include <winsock2.h>
//=========================== define ==========================================

#define DEFAULT_SERVER_NAME  "localhost"
#define DEFAULT_SERVER_PORT  14159
#define OPENCLIENT_BUFSIZE   150

//=========================== typedef =========================================

//=========================== prototypes ======================================

void opensim_client_send(int  txPacketType,
                         int* txPacketParamsBuf,
                         int  txPacketParamsLength);

void opensim_client_waitForPacket(int* rxPacketType,
                                  int* rxPacketParamsBuf,
                                  int  rxPacketParamsMaxLength,
                                  int* rxPacketParamsLength);

void opensim_client_sendAndWaitForAck(int  txPacketType,
                                      int* txPacketParamsBuf,
                                      int  txPacketParamsLength,
                                      int* rxPacketParamsBuf,
                                      int  rxPacketParamsExpectedLength);

void opensim_client_abort();

#endif
