#ifndef SERVER_H
#define SERVER_H

#include "dlist.h"

#define BUF_SIZE 1024
#define NAME_MAX 32
#define DEAULT_NAME "USER%d"
#define TRUE 1
#define FALSE 0

typedef struct srv_ctx {
   struct event_base* ev_base;
   dl_entry_t* ctx_list;
   int user_count;
} srv_ctx_t;

typedef struct cli_ctx {
   srv_ctx_t* base_ctx;
   struct bufferevent *bev;
   char name[NAME_MAX];
} cli_ctx_t;


#endif /*SERVER_H*/