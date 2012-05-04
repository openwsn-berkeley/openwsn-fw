/**
\brief Client program to the OpenSim server.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "supply.h"
#include "tcp_port.h"
#include "opensim_client.h"

//=========================== variables =======================================

typedef struct {
   char    txBuffer[OPENCLIENT_BUFSIZE];
   char    rxBuffer[OPENCLIENT_BUFSIZE];
   SOCKET  conn_socket;
} opensim_client_vars_t;

opensim_client_vars_t opensim_client_vars;

//=========================== prototypes ======================================

void printUsage(char* progname);

//=========================== main ============================================

int main(int argc, char **argv) {
   unsigned short       server_port;
   char*                server_name;
   int                  loopflag;
   int                  i;
   int                  loopcount;
   int                  numloops;
   int                  retval;
   
   server_name     =  DEFAULT_SERVER_NAME;
   server_port     =  DEFAULT_SERVER_PORT;
   numloops        =  5;

   // print banner
   printf("OpenSim client\r\n\r\n");
   
   // filter parameters passed
   if (argc >1) {
      for(i=1; i<argc; i++) {
         if ((argv[i][0] == '-') || (argv[i][0] == '/')) {
            switch(tolower(argv[i][1])) {
               case 'n':
                  server_name = argv[++i];
                  break;
               case 'p':
                  server_port = atoi(argv[++i]);
                  break;
               default:
                  printUsage(argv[0]);
                  break;
            }
         } else {
            printUsage(argv[0]);
         }
      }
   }
   
   // connect to the server
   opensim_client_vars.conn_socket = tcp_port_connect(server_name, server_port);
   
   supply_init();
   
   while(1) {
      supply_rootFunction();
   }
}

//=========================== public ==========================================

void opensim_client_send(int  txPacketType,
                         int* txPacketParamsBuf,
                         int  txPacketParamsLength) {
   int retval;
   
   // filter errors
   if (txPacketType>0xff || txPacketType<0) {
      fprintf(stderr,"[opensim_client] ERROR: invalid packet type %d\n",txPacketType);
      opensim_client_abort();
   }
   if (txPacketParamsLength+1>sizeof(opensim_client_vars.txBuffer)) {
      fprintf(stderr,"[opensim_client] ERROR: too many bytes to send: expected at most %d, got %d\n",
                             sizeof(opensim_client_vars.txBuffer),
                             txPacketParamsLength+1);
      opensim_client_abort();
   }
   
   // prepare txbuffer
   opensim_client_vars.txBuffer[0] = txPacketType;
   if (txPacketParamsLength>0) {
      memcpy(&opensim_client_vars.txBuffer[1],txPacketParamsBuf,txPacketParamsLength);
   }
   
   // send command to server
   retval = send(opensim_client_vars.conn_socket,
                 opensim_client_vars.txBuffer,
                 txPacketParamsLength+1,
                 0);
   if (retval == SOCKET_ERROR) {
      fprintf(stderr,"[opensim_client] ERROR: send() failed (error=%d)\n", WSAGetLastError());
      opensim_client_abort();
   }
   printf("[opensim_client] DEBUG: sent %d\r\n",txPacketType);
}

void opensim_client_waitForPacket(int* rxPacketType,
                                  int* rxPacketParamsBuf,
                                  int  rxPacketParamsMaxLength,
                                  int* rxPacketParamsLength) {
   int retval;
   
   // this blocks while waiting for a packet
   retval = recv(opensim_client_vars.conn_socket,
                 opensim_client_vars.rxBuffer,
                 sizeof(opensim_client_vars.rxBuffer),
                 0);
   
   // filter errors
   if (retval==SOCKET_ERROR) {
      fprintf(stderr,"[opensim_client] ERROR: received failed (error=%d)\n", WSAGetLastError());
      opensim_client_abort();
   }
   if (retval == 0) {
      printf("[opensim_client] WARNING: server closed connection.\n");
      opensim_client_abort();
   }
   if (retval>rxPacketParamsMaxLength+1) {
      fprintf(stderr,"[opensim_client] ERROR: expected at most %d bytes, received %d\n",
                                  rxPacketParamsMaxLength,
                                  retval-1);
      opensim_client_abort();
   }
   
   printf("[opensim_client] DEBUG: received %d\n",(int)opensim_client_vars.rxBuffer[0]);
   
   // copy packet type to rxPacketType
   *rxPacketType = opensim_client_vars.rxBuffer[0];
   
   // copy params type to rxPacketParamsBuf
   if (rxPacketParamsMaxLength>0) {
      memcpy(rxPacketParamsBuf,&opensim_client_vars.rxBuffer[1],retval-1);
   }
   
   // indicate number of bytes written
   *rxPacketParamsLength = retval-1;
}

void opensim_client_sendAndWaitForAck(int  txPacketType,
                                      int* txPacketParamsBuf,
                                      int  txPacketParamsLength,
                                      int* rxPacketParamsBuf,
                                      int  rxPacketParamsExpectedLength) {
   int rxPacketType;
   int rxPacketParamsLength;
   
   // send packet
   opensim_client_send(txPacketType,
                       txPacketParamsBuf,
                       txPacketParamsLength);
   
   // wait for ACK
   opensim_client_waitForPacket(&rxPacketType,
                                rxPacketParamsBuf,
                                rxPacketParamsExpectedLength,
                                &rxPacketParamsLength);
   
   // filter errors
   if (rxPacketType!=txPacketType) {
      fprintf(stderr,"[opensim_client] ERROR: wrong ACK's packet type: expected %d, got %d\n",
                              txPacketType,
                              rxPacketType);
      opensim_client_abort();
   }
   if (rxPacketParamsLength!=rxPacketParamsExpectedLength) {
      fprintf(stderr,"[opensim_client] ERROR: wrong ACK length: expected %d bytes, got %d\n",
                              rxPacketParamsExpectedLength,
                              rxPacketParamsLength);
      opensim_client_abort();
   }
}

void opensim_client_abort() {
   closesocket(opensim_client_vars.conn_socket);
   WSACleanup();
   exit(1);
}

//=========================== private =========================================

void printUsage(char* progname) {
   fprintf(stderr,"printUsage: %s -n [server_address name/IP address] -p [port_num] -l [iterations]\n", progname);
   fprintf(stderr,"Where:\n\tprotocol is one of TCP or UDP\n");
   fprintf(stderr,"\t- server_address is the IP address or name of server_address\n");
   fprintf(stderr,"\t- port_num is the port to listen on\n");
   opensim_client_abort();
}