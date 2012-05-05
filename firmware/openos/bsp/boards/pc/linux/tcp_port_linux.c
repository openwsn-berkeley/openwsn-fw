/**
\brief Generic TCP client, Linux-style

\author Xavier Vilajosana <watteyne@eecs.berkeley.edu>, May 2012.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "tcp_port.h"



SOCKET tcp_port_connect(char* server_name, unsigned short portno) {
   int                  sockfd;
   struct   sockaddr_in serv_addr;
   struct   hostent*    server;
   unsigned int         addr;
   
   // prepare server port
   if (portno==0) {
      fprintf(stderr,"ERROR: can not connect to port 0\n");
      exit(1);
   }
   
   // prepare server name
   if (isalpha(server_name[0])) {
      server = gethostbyname(server_name);
   } else {
      addr = inet_addr(server_name);
      server = gethostbyaddr((char*)&addr,4,AF_INET);
   }
   if (server == NULL) {
      fprintf(stderr,"ERROR: cannot resolve \"%s\" \n", server_name);
      exit(1);
   }
   
   // copy information into the sockaddr_in structure
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
   serv_addr.sin_port   = htons(portno);
   
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0) {
      fprintf(stderr,"ERROR opening socket\n");
   }
   
   if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
      fprintf(stderr,"ERROR: could not connect to server_address\n");
      exit(1);
   }
   
   return sockfd;
}
