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

#define TEST_MSG "This is a test message\n"
#define BUF_SIZE 1024

typedef struct srv_ctx {
   struct event_base* ev_base;
   dl_entry_t* ctx_list;
} srv_ctx_t;


// TODO: MAKE A STRUCT FOR EACH CLIENT TO USE FOR NAME AND ALL THAT
typedef struct cli_ctx {
   srv_ctx_t* ctx;
} srv_ctx_t;

static void echo_read_cb(struct bufferevent *bev, void *arg);
static void echo_event_cb(struct bufferevent *bev, short events, void *arg);
static void accept_conn_cb(struct evconnlistener *listener,
   evutil_socket_t fd, struct sockaddr *address, int socklen,
   void *arg);
static void accept_error_cb(struct evconnlistener *listener, void *arg);
static void signal_cb(evutil_socket_t fd, short event, void* arg);
static void free_ctx(srv_ctx_t* ctx);

static void broadcast_msg(dl_entry_t* ctx_list, char* msg, int len);

static void test_server();



int main(int argc, char **argv) {
   struct event_base *base;
   struct evconnlistener *listener;
   struct event* sev_int;
   struct sockaddr_in sin;
   srv_ctx_t* ctx = NULL;

   // test_list();
   // return 0;

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

   ctx = malloc(sizeof(srv_ctx_t));
   if (ctx == NULL) {
      printf("Unable to malloc ctx\n");
   }
   ctx->ev_base = base; 
   ctx->ctx_list = NULL;
   printf("ctx:%p\n", ctx);

   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = htonl(0);
   sin.sin_port = htons(port);

   listener = evconnlistener_new_bind(base, accept_conn_cb, ctx,
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

   free_ctx(ctx);
   free(ctx);

   evconnlistener_free(listener);

   return 0;
}

void free_ctx(srv_ctx_t* ctx) {
   dl_entry_t* con_entry = ctx->ctx_list;
   while(con_entry != NULL) {
      con_entry = deque(con_entry,con_entry);
   }
}

void broadcast_msg(dl_entry_t* ctx_list, char* msg, int len) {
   dl_entry_t* con = ctx_list;
   struct bufferevent *bev = NULL;

   while(con != NULL){
      bev = (struct bufferevent *) con->data;
      bufferevent_write(bev,msg,len);
      con = con->next;
   }
}


void echo_read_cb(struct bufferevent *bev, void *arg) {
   /* This callback is invoked when there is data to read on bev. */
   srv_ctx_t* ctx = (srv_ctx_t*)arg;
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
   broadcast_msg(ctx->ctx_list, buf, written);
   // bufferevent_write(bev,buf,written);
   // evbuffer_remove(input,buf,BUF_SIZE);


   /* Copy all the data from the input buffer to the output buffer. */
   // evbuffer_add(output, input);
}

static void echo_event_cb(struct bufferevent *bev, short events, void *arg) {
   srv_ctx_t* ctx = (srv_ctx_t*)arg;
   if (events & BEV_EVENT_ERROR) {
      perror("Error from bufferevent");
   }
   if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
      printf("Freeing connection bev%p\n",bev);
      ctx->ctx_list = deque_val(ctx->ctx_list,bev);
      print_list(ctx->ctx_list);
      bufferevent_free(bev);
   }
}

static void accept_conn_cb(struct evconnlistener *listener,
   evutil_socket_t fd, struct sockaddr *address, int socklen,
   void *arg) {
   /* We got a new connection! Set up a bufferevent for it. */
   struct event_base *base = evconnlistener_get_base(listener);
   struct bufferevent *bev = bufferevent_socket_new(
          base, fd, BEV_OPT_CLOSE_ON_FREE);

   srv_ctx_t* ctx = (srv_ctx_t*)arg;
   dl_entry_t* dl_entry;
   // Not sure if bufferevents are unique?
   dl_entry = insque(ctx->ctx_list,bev);
   if (ctx->ctx_list == NULL) {
      ctx->ctx_list = dl_entry;
   }

   printf("Accepted New connection bev:%p\n",bev);
   print_list(ctx->ctx_list);

   bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, ctx);

   bufferevent_enable(bev, EV_READ|EV_WRITE);
}

static void accept_error_cb(struct evconnlistener *listener, void *arg) {
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

