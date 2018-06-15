#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
//http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/

#define BUF_SIZE 100

void init();

void init() {
   initscr();                 /* Start curses mode              */
   //noecho();                  // Dont echo characters as they are typed
   cbreak();                  // pass the intreupts to the program?
   keypad(stdscr, TRUE);      // enable the arrow keys to be used
}

void test() {
   char prompt[]="> ";           /* message to be appeared on the screen */
   int row_max,col_max;          /* to store the number of rows and      *
                                  * the number of colums of the screen   */
   char buf[BUF_SIZE];
   WINDOW *chat_display;
   WINDOW *chat_bar;
   int row = 0;
   int col = 0;

   getmaxyx(stdscr,row_max,col_max);     /* get the number of rows and columns */

   chat_display = newwin(row_max-1, col_max, 0, 0);
   chat_bar = newwin(1, col_max, row_max-1, 0);

   scrollok(chat_display,TRUE);

   
   mvprintw(row_max-2,0,"This screen has %d rows and %d columns\n",row_max,col_max);
   refresh();

   for (row = 0; row < row_max+15; ++row)
   { 
      mvwprintw(chat_bar,0,0,"> ");
      wclrtoeol(chat_bar);
      wrefresh(chat_bar);
      wgetnstr(chat_bar,buf,BUF_SIZE);

      wprintw(chat_display,"%s\n",buf); /* print the message on the screen */
      wrefresh(chat_display);
   }
   
   //
   
   
}

void start_interface() {
   init();

   test();

   getch();
   endwin();                       /* End curses mode                */

   return 0;

}
