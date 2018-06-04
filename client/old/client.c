#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#define BUFFER_MAX   200

int connect_to_host(char* host, char* port, int protocol);

int main() {
   int sock;
   char query[2048];
   char response[2048];
   int query_len;
   char hostname[] = "localhost";
   
   sock = connect_to_host(hostname, "8080", SOCK_STREAM);

   sprintf(query ,"GET / HTTP/1.1\r\nHost: %s\r\n", hostname);
   query_len = strlen(query);
   send(sock,query,query_len,0);
   recv(sock,response,2048,0);

   printf("Received:\n%s", response);

   close(sock);
   return 0;
}




int connect_to_host(char* host, char* service, int protocol) {
   int sock;
   int ret;
   struct addrinfo hints;
   struct addrinfo* addr_ptr;
   struct addrinfo* addr_list;

   memset(&hints, 0, sizeof(hints));
   hints.ai_socktype = protocol;
   hints.ai_family = AF_UNSPEC; // IP4 or IP6, we don't care
   ret = getaddrinfo(host, service, &hints, &addr_list);
   if (ret != 0) {
      fprintf(stderr, "Failed in getaddrinfo: %s\n", gai_strerror(ret));
      exit(EXIT_FAILURE);
   }

   for (addr_ptr = addr_list; addr_ptr != NULL; addr_ptr = addr_ptr->ai_next) {
      sock = socket(addr_ptr->ai_family, addr_ptr->ai_socktype, addr_ptr->ai_protocol);
      if (sock == -1) {
         perror("socket");
         continue;
      }
      if (connect(sock, addr_ptr->ai_addr, addr_ptr->ai_addrlen) == -1) {
         perror("connect");
         close(sock);
         continue;
      }
      break;
   }
   freeaddrinfo(addr_list);
   if (addr_ptr == NULL) {
      fprintf(stderr, "Failed to find a suitable address for connection\n");
      exit(EXIT_FAILURE);
   }
   return sock;
}