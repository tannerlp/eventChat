#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>


#include "commands.h"
#include "server.h"

#define TEST_MSG "This is a test message\n"

static void echo_read_cb(struct bufferevent *bev, void *arg);
static void echo_event_cb(struct bufferevent *bev, short events, void *arg);
static void accept_conn_cb(struct evconnlistener *listener,
   evutil_socket_t fd, struct sockaddr *address, int socklen,
   void *arg);
static void accept_error_cb(struct evconnlistener *listener, void *arg);
static void signal_cb(evutil_socket_t fd, short event, void* arg);
static void free_ctx(srv_ctx_t* ctx);


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
      perror("Couldn't open event base");
      return 1;
   }

   sev_int = evsignal_new(base, SIGINT, signal_cb, base);
   if (sev_int == NULL) {
      perror("Couldn't create SIGINT handler event\n");
      return 1;
   }
   evsignal_add(sev_int, NULL);

   ctx = malloc(sizeof(srv_ctx_t));
   if (ctx == NULL) {
      perror("Unable to malloc ctx\n");
      return -1;
   }
   memset(ctx,0,sizeof(srv_ctx_t));
   ctx->ev_base = base; 

   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = htonl(0);
   sin.sin_port = htons(port);

   listener = evconnlistener_new_bind(base, accept_conn_cb, ctx,
      LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE|LEV_OPT_THREADSAFE, -1,
      (struct sockaddr*)&sin, sizeof(sin));
   if (!listener) {
      perror("Couldn't create listener");
      return 1;
   }
   evconnlistener_set_error_cb(listener, accept_error_cb);

   event_base_dispatch(base);

   printf("Libevent Shutdown\n");

   evconnlistener_free(listener);
   event_free(sev_int);
   event_base_free(base);
   free_ctx(ctx);
   free(ctx);

   #if LIBEVENT_VERSION_NUMBER >= 0x02010000
   libevent_global_shutdown();
   #endif

   return 0;
}

void free_ctx(srv_ctx_t* ctx) {
   dl_entry_t* con_entry = ctx->ctx_list;
   while(con_entry != NULL) {
      con_entry = deque(con_entry,con_entry);
   }
}

void echo_read_cb(struct bufferevent *bev, void *arg) {
   /* This callback is invoked when there is data to read on bev. */
   cli_ctx_t* ctx = (cli_ctx_t*)arg;
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
   handle_command(buf, written, ctx);
   // bufferevent_write(bev,buf,written);
   // evbuffer_remove(input,buf,BUF_SIZE);


   /* Copy all the data from the input buffer to the output buffer. */
   // evbuffer_add(output, input);
}

static void echo_event_cb(struct bufferevent *bev, short events, void *arg) {
   cli_ctx_t* ctx = (cli_ctx_t*)arg;
   srv_ctx_t* base_ctx = ctx->base_ctx;
   if (events & BEV_EVENT_ERROR) {
      perror("Error from bufferevent");
   }
   if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
      printf("Freeing connection bev%p\n",bev);
      base_ctx->ctx_list = deque_val(base_ctx->ctx_list,bev);
      // Send some message that they are leaveing the chat
      //broadcast_msg(ctx, buf, len, TRUE);
      free(ctx);
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

   dl_entry_t* dl_entry;
   srv_ctx_t* base_ctx = (srv_ctx_t*)arg;
   cli_ctx_t* ctx = (cli_ctx_t*)malloc(sizeof(cli_ctx_t));
   memset(ctx,0,sizeof(cli_ctx_t));
   
   base_ctx->user_count++;
   ctx->base_ctx = base_ctx;
   ctx->bev = bev;
   snprintf(ctx->name,NAME_MAX,DEAULT_NAME,base_ctx->user_count);
   
   // Not sure if bufferevents are unique?
   dl_entry = insque(base_ctx->ctx_list,bev);
   if (base_ctx->ctx_list == NULL) {
      base_ctx->ctx_list = dl_entry;
   }

   printf("Accepted New connection %s\n",ctx->name);

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

