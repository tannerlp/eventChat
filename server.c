#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "dlist.h"

#define TEST_MSG "This is a test message"

static void echo_read_cb(struct bufferevent *bev, void *ctx);
static void echo_event_cb(struct bufferevent *bev, short events, void *ctx);
static void accept_conn_cb(struct evconnlistener *listener,
   evutil_socket_t fd, struct sockaddr *address, int socklen,
   void *ctx);
static void accept_error_cb(struct evconnlistener *listener, void *ctx);
void signal_cb(evutil_socket_t fd, short event, void* arg);

static void test_server();

typedef struct srv_ctx {
   struct event_base* ev_base;
   int test;
} srv_ctx;

int main(int argc, char **argv) {
   struct event_base *base;
   struct evconnlistener *listener;
   struct event* sev_int;
   struct sockaddr_in sin;

   test_list();

   return 0;

   int port = 8080;

   if (argc > 1) {
      port = atoi(argv[1]);
   }
   if (port<=0 || port>65535) {
      printf("Invalid port");
      return 1;
   }

   base = event_base_new();
   if (!base) {
      printf("Couldn't open event base");
      return 1;
   }

   sev_int = evsignal_new(base, SIGINT, signal_cb, base);
   if (sev_int == NULL) {
      printf("Couldn't create SIGINT handler event\n");
      return 1;
   }
   evsignal_add(sev_int, NULL);

   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = htonl(0);
   sin.sin_port = htons(port);

   listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
      LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
      (struct sockaddr*)&sin, sizeof(sin));
   if (!listener) {
      perror("Couldn't create listener");
      return 1;
   }
   evconnlistener_set_error_cb(listener, accept_error_cb);

   event_base_dispatch(base);

   printf("Libevent Shutdown\n");

   event_free(sev_int);

   evconnlistener_free(listener);

   return 0;
}

void echo_read_cb(struct bufferevent *bev, void *ctx) {
   /* This callback is invoked when there is data to read on bev. */
   struct evbuffer *input = bufferevent_get_input(bev);
   struct evbuffer *output = bufferevent_get_output(bev);

   bufferevent_write(bev, TEST_MSG, sizeof(TEST_MSG));

   /* Copy all the data from the input buffer to the output buffer. */
   evbuffer_add_buffer(output, input);
}

static void echo_event_cb(struct bufferevent *bev, short events, void *ctx) {
   if (events & BEV_EVENT_ERROR) {
      perror("Error from bufferevent");
   }
   if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
      printf("Freeing connection\n");
      bufferevent_free(bev);
   }
}

static void accept_conn_cb(struct evconnlistener *listener,
   evutil_socket_t fd, struct sockaddr *address, int socklen,
   void *ctx) {
   /* We got a new connection! Set up a bufferevent for it. */
   struct event_base *base = evconnlistener_get_base(listener);
   struct bufferevent *bev = bufferevent_socket_new(
          base, fd, BEV_OPT_CLOSE_ON_FREE);

   printf("Accepted New connection\n");

   bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);

   bufferevent_enable(bev, EV_READ|EV_WRITE);
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx) {
   struct event_base *base = evconnlistener_get_base(listener);
   int err = EVUTIL_SOCKET_ERROR();
   fprintf(stderr, "Got an error %d (%s) on the listener. "
      "Shutting down.\n", err, evutil_socket_error_to_string(err));

   event_base_loopexit(base, NULL);
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

