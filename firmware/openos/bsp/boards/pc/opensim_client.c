/**
\brief Client program to the OpenSim server.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
   
   // wait for command from the server
   while(1) {
      
      // receive command from server
      retval = recv(opensim_client_vars.conn_socket,
                    opensim_client_vars.rxBuffer,
                    sizeof(opensim_client_vars.rxBuffer),
                    0);
      if (retval==SOCKET_ERROR) {
         fprintf(stderr,"ERROR: recv() failed (error=%d)\n", WSAGetLastError());
         closesocket(opensim_client_vars.conn_socket);
         WSACleanup();
         return -1;
      }
      if (retval == 0) {
         printf("WARNING: server closed connection.\n");
         closesocket(opensim_client_vars.conn_socket);
         WSACleanup();
         return -1;
      }
      printf("INFO: Received %d bytes, data \"%s\"\n",
                   retval,
                   opensim_client_vars.rxBuffer);
                   
      mote_main();
   }
   
   closesocket(opensim_client_vars.conn_socket);
   WSACleanup();
   return 0;
}

//=========================== public ==========================================

int opensim_client_send(int* pTxData,
                        int  txDataLength,
                        int* pRxBuffer,
                        int  maxRxBufferLength) {
   int retval;
   
   // send command to server
   retval = send(opensim_client_vars.conn_socket,
                 pTxData,
                 txDataLength,
                 0);
   if (retval == SOCKET_ERROR) {
      fprintf(stderr,"ERROR: send() failed (error=%d)\n", WSAGetLastError());
      WSACleanup();
      exit(1);
   }
   printf("command sent\r\n");
   
   // wait for reply from server
   retval = recv(opensim_client_vars.conn_socket,
                 opensim_client_vars.rxBuffer,
                 sizeof(opensim_client_vars.rxBuffer),
                 0);
   if (retval==SOCKET_ERROR) {
      fprintf(stderr,"FATAL: recv failed (error=%d)\n", WSAGetLastError());
      closesocket(opensim_client_vars.conn_socket);
      WSACleanup();
      exit(1);
   }
   if (retval==0) {
      fprintf(stderr,"FATAL: server closed connection.\n");
      closesocket(opensim_client_vars.conn_socket);
      WSACleanup();
      exit(1);
   }
   if (retval>maxRxBufferLength) {
      fprintf(stderr,"FATAL: expected at most %d bytes, got %d.\n",maxRxBufferLength,retval);
      closesocket(opensim_client_vars.conn_socket);
      WSACleanup();
      exit(1);
   }
   
   // copy reply in pRxBuffer
   memcpy(pRxBuffer,opensim_client_vars.rxBuffer,retval);
   
   printf("INFO: received %d bytes\n",retval);
   printf("ack received\r\n");
   return 0;
}

//=========================== private =========================================

void printUsage(char* progname) {
   fprintf(stderr,"printUsage: %s -n [server_address name/IP address] -p [port_num] -l [iterations]\n", progname);
   fprintf(stderr,"Where:\n\tprotocol is one of TCP or UDP\n");
   fprintf(stderr,"\t- server_address is the IP address or name of server_address\n");
   fprintf(stderr,"\t- port_num is the port to listen on\n");
   WSACleanup();
   exit(1);
}