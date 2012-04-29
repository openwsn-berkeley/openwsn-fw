/**
\brief Generic TCP client, Windows-style

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tcp_port.h"

SOCKET tcp_port_connect(char* server_name, unsigned short server_port) {
   int                  retval;
   struct hostent*      hp;
   unsigned int         addr;
   struct sockaddr_in   server_address;
   WSADATA              wsaData;
   SOCKET               conn_socket;
   
   // initialize Windows' socket module
   if ((retval = WSAStartup(0x202, &wsaData)) != 0) {
      fprintf(stderr,"ERROR: WSAStartup() failed (error=%d)\n", retval);
      WSACleanup();
      exit(1);
   }
   
   // prepare server port
   if (server_port==0) {
      fprintf(stderr,"ERROR: can not connect to port 0");
      WSACleanup();
      exit(1);
   }
   
   // prepare server name
   if (isalpha(server_name[0])) {
      hp = gethostbyname(server_name);
   } else {
      addr = inet_addr(server_name);
      hp = gethostbyaddr((char*)&addr,4,AF_INET);
   }
   if (hp == NULL) {
      fprintf(stderr,"ERROR: cannot resolve \"%s\" (error=%d)\n", server_name, WSAGetLastError());
      WSACleanup();
      exit(1);
   }
   
   // copy information into the sockaddr_in structure
   memset(&server_address, 0, sizeof(server_address));
   memcpy(&(server_address.sin_addr), hp->h_addr, hp->h_length);
   server_address.sin_family = hp->h_addrtype;
   server_address.sin_port   = htons(server_port);
   
   // open the socket
   conn_socket = socket(AF_INET,SOCK_STREAM,0);
   if (conn_socket<0) {
      fprintf(stderr,"ERROR: could not open socket (error=%d)\n", WSAGetLastError());
      WSACleanup();
      exit(1);
   }
   
   // connect the socket
   if (connect(conn_socket,(struct sockaddr*)&server_address,sizeof(server_address))==SOCKET_ERROR) {
      fprintf(stderr,"ERROR: could not connect to server_address (error=%d)\n", WSAGetLastError());
      WSACleanup();
      exit(1);
   }
   
   return conn_socket;
}