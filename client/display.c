#include <ncurses.h>
#include <stdlib.h>
//http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/

#define PROMPT "> "

WINDOW *chat_display;
WINDOW *chat_bar;
int row_max;
int col_max;


static void init_config();


void init_config() {
   initscr();                 /* Start curses mode              */
   //noecho();                  // Dont echo characters as they are typed
   cbreak();                  // pass the intreupts to the program?
   keypad(stdscr, TRUE);      // enable the arrow keys to be used
}

void start_display() {
   init_config();
   char prompt[]="> ";           /* message to be appeared on the screen */

   getmaxyx(stdscr,row_max,col_max);     /* get the number of rows and columns */

   chat_display = newwin(row_max-1, col_max, 0, 0);
   chat_bar = newwin(1, col_max, row_max-1, 0);

   scrollok(chat_display,TRUE);

   // mvprintw(row_max-2,0,"This screen has %d rows and %d columns\n",row_max,col_max);
   refresh();
}

void read_chat_bar(char* buf, int size){
   if (chat_bar) {
      mvwprintw(chat_bar,0,0,PROMPT);
      wclrtoeol(chat_bar);
      wrefresh(chat_bar);
      wgetnstr(chat_bar,buf,size);
   }
}

void write_chat_window(char * buf){
   if (chat_display) {
      wprintw(chat_display,"%s",buf); /* print the message on the screen */
      wrefresh(chat_display);
      // //recenter cursor
      // wmove(chat_bar,0,0);
   }
}

void quit_display() {
   endwin();
}

void test_display() {
   init_config();

   start_display();

   getch();

   endwin();                       /* End curses mode                */

   exit(0);
}
