#include <event2/event.h>
#include <event2/bufferevent.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>

#include "interface.h"

#define BUF_SIZE 1024


static void readcb(struct bufferevent *bev, void *arg);
static void eventcb(struct bufferevent *bev, short events, void *ptr);
static void signal_cb(evutil_socket_t fd, short event, void* arg);

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
   printf("%s\n",buf);
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
      printf("connected!\n");
      fgets(buf,BUF_SIZE,stdin);
      len = strlen(buf);
      printf("got this%d:%s\n",len,buf);
      bufferevent_write(bev,buf,len);
      printf("sent\n");
   } else if (events & BEV_EVENT_ERROR) {
      perror("Error occured while connecting\n");
      event_base_loopbreak(ptr);
   /* An error occured while connecting. */
   }
}

int main(int argc, char **argv) {
   struct event_base *base;
   struct event* sev_int;
   struct bufferevent *bev;
   struct sockaddr_in sin;

   // start_interface();
   // exit(0);

   base = event_base_new();
   if (!base) {
      perror("Couldn't open event base");
      return 1;
   }

   sev_int = evsignal_new(base, SIGINT, signal_cb, base);
   if (sev_int == NULL) {
      perror("Couldn't create SIGINT handler event\n");
      return 1;
   }
   evsignal_add(sev_int, NULL);

   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = htonl(0); /* 127.0.0.1 */
   sin.sin_port = htons(8080); /* Port 8080 */

   bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

   bufferevent_setcb(bev, readcb, NULL, eventcb, base);
   bufferevent_enable(bev, EV_READ|EV_WRITE);

   if (bufferevent_socket_connect(bev, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      /* Error starting connection */
      perror("Unable to connect to host");
      bufferevent_free(bev);
      return -1;
   }

   event_base_dispatch(base);

   event_free(sev_int);
   event_base_free(base);

   return 0;
}

void signal_cb(evutil_socket_t fd, short event, void* arg) {
   int signum = fd;
   switch (signum) {
      case SIGINT:
         printf("Caught SIGINT\n");
         event_base_loopbreak(arg);
         break;
      default:
         break;
   }
   return;
}