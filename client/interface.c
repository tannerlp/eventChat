#include <ncurses.h>
//http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/

void init();

void init() {
   initscr();                 /* Start curses mode              */
   noecho();                  // Dont echo characters as they are typed
   cbreak();                  // pass the intreupts to the program?
   keypad(stdscr, TRUE);      // enable the arrow keys to be used
}

void start_interface() {
   init();
   printw("Hello World !!!");      /* Print Hello World              */
   refresh();                      /* Print it on to the real screen */
   getch();                        /* Wait for user input */
   endwin();                       /* End curses mode                */

   return 0;

}
