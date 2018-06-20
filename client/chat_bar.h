#ifndef CHAT_BAR_H
#define CHAT_BAR_H

#include <event2/event.h>
#include <event2/bufferevent.h>

#define BUF_SIZE 1024

void read_loop(struct bufferevent *bev);

#endif /*CHAT_BAR_H*/