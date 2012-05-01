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

//=========================== prototypes ======================================

int opensim_client_send(void* pTxData,
                        int   txDataLength,
                        void* pRxBuffer,
                        int   maxRxBufferLength);

#endif