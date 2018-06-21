#include <event2/event.h>
#include <event2/bufferevent.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <ncurses.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "display.h"
#include "chat_bar.h"

#define DEFAULT_IP "0" // localhost
#define DEFAULT_PORT 8080

static void readcb(struct bufferevent *bev, void *arg);
static void eventcb(struct bufferevent *bev, short events, void *ptr);
static void signal_cb(evutil_socket_t fd, short event, void* arg);

void perror_quit(char *error) {
   quit_display();
   perror(error);
}

void readcb(struct bufferevent *bev, void *arg) {
   struct evbuffer *input = bufferevent_get_input(bev);
   struct evbuffer *output = bufferevent_get_output(bev);
   char buf[BUF_SIZE];
   int written = 0;


   // bufferevent_write(bev, TEST_MSG, sizeof(TEST_MSG));

//    int evbuffer_remove  (  struct evbuffer *    buf,
// void *   data,
// size_t   datlen 
// )
   written = bufferevent_read(bev,buf,BUF_SIZE);

   buf[written] = '\0';
   write_chat_window(buf);
   // bufferevent_write(bev,buf,written);
   // evbuffer_remove(input,buf,BUF_SIZE);


   /* Copy all the data from the input buffer to the output buffer. */
   // evbuffer_add(output, input);
}

void eventcb(struct bufferevent *bev, short events, void *ptr) {
   char buf[BUF_SIZE];
   int len =0;
   if (events & BEV_EVENT_CONNECTED) {
   /* We're connected to 127.0.0.1:8080.   Ordinarily we'd do
   something here, like start reading or writing. */
      write_chat_window("[*]Connected to server!\n\n");
   // } else if (events & BEV_EVENT_ERROR) {
   //    //perror("Error occured while connecting\n");
   //    event_base_loopbreak(ptr);
   // /* An error occured while connecting. */
   // }
   } else if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) {
      struct event_base *base = ptr;
      if (events & BEV_EVENT_ERROR) {
         //write_chat_window("Error occured while connecting");
         perror_quit("Error occured while connecting");
      }
      else {
         write_chat_window("Connection ended\n");
      }
      bufferevent_free(bev);
      event_base_loopbreak(ptr);
   }
}

int main(int argc, char **argv) {
   struct event_base *base;
   struct event* sev_int;
   struct bufferevent *bev;
   struct sockaddr_in sin;
   pthread_t chat_bar;
   char* ip_addr;
   int port;

   start_display();

   base = event_base_new();
   if (!base) {
      //write_chat_window("Couldn't open event base");
      perror_quit("Couldn't open event base");
      return 1;
   }

   sev_int = evsignal_new(base, SIGINT, signal_cb, base);
   if (sev_int == NULL) {
      perror_quit("Couldn't create SIGINT handler event");
      return 1;
   }
   evsignal_add(sev_int, NULL);

   if (argc >= 2 ) {
      ip_addr = argv[1];
   } 
   else {
      ip_addr = DEFAULT_IP;
   }
   if (argc == 3) {
      port = atoi(argv[2]);
   } 
   else {
      port = DEFAULT_PORT;
   }

   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   if (!inet_aton(ip_addr,&sin.sin_addr.s_addr)) {
      perror_quit("IP address could not be parsed.");
   }
   sin.sin_port = htons(port); /* Port 8080 */

   bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

   bufferevent_setcb(bev, readcb, NULL, eventcb, base);
   bufferevent_enable(bev, EV_READ|EV_WRITE);

   if (bufferevent_socket_connect(bev, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      /* Error starting connection */
      //write_chat_window("Unable to connect to host");
      perror_quit("Unable to connect to host");
      bufferevent_free(bev);
      return -1;
   }

   // start a mutex here?
   pthread_create(&chat_bar, NULL, read_loop, (void*)bev);

   event_base_dispatch(base);

   event_free(sev_int);
   event_base_free(base);
   quit_display();
   return 0;
}

void signal_cb(evutil_socket_t fd, short event, void* arg) {
   int signum = fd;
   switch (signum) {
      case SIGINT:
         write_chat_window("Caught SIGINT\n");
         event_base_loopbreak(arg);
         break;
      default:
         break;
   }
   return;
}
