#include <stdio.h>
#include <string.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "server.h"

#define STR_MATCH(s, n) strcmp(s, n) == 0


static void broadcast_msg(cli_ctx_t* ctx, char* msg, int len, int show_name);
static int parse_command(char* buf, int len, cli_ctx_t* ctx);
static void copy_name(char* buf, cli_ctx_t* ctx);
static void send_error(char* buf, struct bufferevent* bev);

void send_msg(char* msg, struct bufferevent* bev) {
   bufferevent_write(bev,msg,strlen(msg));
}

void handle_command(char* buf, int len, cli_ctx_t* ctx) {

   // There is probably a better way to do this...
   if (buf[0] == '/') {
      parse_command(buf,len,ctx);
   }
   else {
      broadcast_msg(ctx,buf,len,TRUE);
   }
   
}

int parse_command(char* buf, int len, cli_ctx_t* ctx) {
   char* data, holder;

   // get ride of trailing new lines
   buf[strcspn(buf, "\n")] = 0;

   holder,data = strdup(buf+1);
   char* command = strsep(&data," ");

   if (STR_MATCH(command,"NAME") && data != NULL) {
      copy_name(data,ctx);
   }
   else {
      send_msg("[*] No matching command found\n",ctx->bev);
   }

   if (holder) {
      free(holder);
   }
   return 0;
}

void copy_name(char* buf, cli_ctx_t* ctx) {
   // Should be plenty big enough...
   char tmp_buf[BUF_SIZE];
   int len;

   if ( NAME_MAX <= strlen(buf)+1 ) {
      send_msg("[*] Name is too long\n",ctx->bev);
      return;
   }

   len = snprintf(tmp_buf,BUF_SIZE,"[*] %s changed names to %s\n",ctx->name,buf);

   // Check for dup names on server?

   strncpy(ctx->name,buf,NAME_MAX);
   ctx->name[NAME_MAX-1] = '\0';

   broadcast_msg(ctx, tmp_buf, len, FALSE);
}

void broadcast_msg(cli_ctx_t* ctx, char* buf, int len, int show_name) {
   char msg[BUF_SIZE+NAME_MAX+10]; 
   struct bufferevent *bev = NULL;
   int size = 0;

   srv_ctx_t* base_ctx = ctx->base_ctx;
   dl_entry_t* con = base_ctx->ctx_list;
   
   if (show_name) {
      size = sprintf(msg,"<%s> %s",ctx->name,buf);
   }
   else {
      strncpy(msg,buf,BUF_SIZE);
      msg[BUF_SIZE+1]='\0';
      size = len;
   }
   

   while(con != NULL){
      bev = (struct bufferevent *) con->data;
      bufferevent_write(bev,msg,size);
      con = con->next;
   }
}