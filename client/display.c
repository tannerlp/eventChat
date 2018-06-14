#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
//http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/

void init();

void init() {
   initscr();                 /* Start curses mode              */
   noecho();                  // Dont echo characters as they are typed
   cbreak();                  // pass the intreupts to the program?
   scrollok(stdscr,TRUE);
   keypad(stdscr, TRUE);      // enable the arrow keys to be used
}

void test() {
   char mesg[]="test";           /* message to be appeared on the screen */
   int row_max,col_max;          /* to store the number of rows and      *
                                  * the number of colums of the screen   */
   WINDOW *new;
   int row = 0;
   int col = 0;

   getmaxyx(stdscr,row_max,col_max);     /* get the number of rows and columns */

   new = newwin(row_max-2, col_max, 0, 0);
   curs_set(0);

   scrollok(new,TRUE);
   // scrollok(stdscr,TRUE);

   
   mvprintw(row_max-2,0,"This screen has %d rows and %d columns\n",row_max,col_max);
   printw("Try resizing your window(if possible) and then run this program again");
   refresh();

   for (row = 0; row < row_max+15; ++row)
   { 

      wprintw(new,"%s %d\n",mesg,row); /* print the message on the screen */
      // mvprintw(row,0,"%s %d\n",mesg,row); /* print the message on the screen */

      usleep(100000);
      wrefresh(new);
   }
   
   //
   
   
}

void hello() {
   printw("Hello World !!!");      /* Print Hello World              */
   refresh();                      /* Print it on to the real screen */
   getch();                        /* Wait for user input */
}

void start_interface() {
   init();

   test();

   getch();
   endwin();                       /* End curses mode                */

   return 0;

}
