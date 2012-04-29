/**
\brief Client program to the OpenSim engine

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tcp_port.h"

#define DEFAULT_SERVER_NAME "localhost"
#define DEFAULT_SERVER_PORT 14159

void Usage(char* progname) {
   fprintf(stderr,"Usage: %s -n [server_address name/IP address] -p [port_num] -l [iterations]\n", progname);
   fprintf(stderr,"Where:\n\tprotocol is one of TCP or UDP\n");
   fprintf(stderr,"\t- server_address is the IP address or name of server_address\n");
   fprintf(stderr,"\t- port_num is the port to listen on\n");
   WSACleanup();
   exit(1);
}

int main(int argc, char **argv) {
   char                 Buffer[128];
   unsigned short       server_port;
   char*                server_name;
   int                  loopflag;
   int                  i;
   int                  loopcount;
   int                  numloops;
   SOCKET               conn_socket;
   int                  retval;
   
   server_name     =  DEFAULT_SERVER_NAME;
   server_port     =  DEFAULT_SERVER_PORT;
   numloops        =  5;

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
                  Usage(argv[0]);
                  break;
            }
         } else {
            Usage(argv[0]);
         }
      }
   }
   
   // connect to the server
   conn_socket = tcp_port_connect(server_name, server_port);
   
   // send some data
   while(numloops>0) {
      
      // send to server
      wsprintf(Buffer,"This is a test message");
      retval = send(conn_socket, Buffer, sizeof(Buffer), 0);
      if (retval == SOCKET_ERROR) {
         fprintf(stderr,"ERROR: send() failed (error=%d)\n", WSAGetLastError());
         WSACleanup();
         return -1;
      }

      // receive from server
      retval = recv(conn_socket, Buffer, sizeof(Buffer), 0);
      if (retval==SOCKET_ERROR) {
         fprintf(stderr,"ERROR: recv() failed (error=%d)\n", WSAGetLastError());
         closesocket(conn_socket);
         WSACleanup();
         return -1;
      }
      if (retval == 0) {
         printf("WARNING: server closed connection.\n");
         closesocket(conn_socket);
         WSACleanup();
         return -1;
      }
      printf("INFO: Received %d bytes, data \"%s\" from server_address.\n", retval, Buffer);
      
      // decrement number of loops
      numloops--;
   }
   
   closesocket(conn_socket);
   WSACleanup();
   return 0;
}