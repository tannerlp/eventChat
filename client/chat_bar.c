#include <ncurses.h>
#include <event2/event.h>
#include <event2/bufferevent.h>

#include "display.h"
#include "chat_bar.h"



void read_loop(struct bufferevent *bev){
    char buf[BUF_SIZE];
    int len =0;

    // Think I need a mutex here to make sure that the buffer event has been set up 
    // before I am able to write to it...

    while(1)
    {
        read_chat_bar(buf,BUF_SIZE);
        len = strlen(buf);
        // does this need a mutex around it as well so
        // two people are not accessing the buffer event?
        bufferevent_write(bev,buf,len);
    }
}