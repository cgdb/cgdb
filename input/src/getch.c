#include <stdio.h>
#include <curses.h>

static void print_key(int key) {
       char temp[24];
       sprintf(temp, "(%c:%d)\n", key, key);
       printw(temp);
}

int main(int argc, char **argv){
    initscr();                       /* Start curses mode */
    noecho();                        /* Do not echo characters typed by user */
    keypad(stdscr, TRUE);
    refresh();                       /* Refresh the initial window once */

    while ( 1 ) {
        int key;
        key = getch();

        switch ( key ) {
            case KEY_UP:        printw("KEY_UP\n");        break;
            case KEY_DOWN:      printw("KEY_DOWN\n");      break;
            case KEY_LEFT:      printw("KEY_LEFT\n");      break;
            case KEY_RIGHT:     printw("KEY_RIGHT\n");     break;
            case KEY_HOME:      printw("KEY_HOME\n");      break;
            case KEY_END:       printw("KEY_END\n");       break;
            case KEY_PPAGE:     printw("KEY_PPAGE\n");     break;
            case KEY_NPAGE:     printw("KEY_NPAGE\n");     break;
            case KEY_DC:        printw("KEY_DC\n");        break;
            case KEY_IC:        printw("KEY_IC\n");        break;
            case KEY_F(1):      printw("KEY_F1\n");        break;
            case KEY_F(2):      printw("KEY_F2\n");        break;
            case KEY_F(3):      printw("KEY_F3\n");        break;
            case KEY_F(4):      printw("KEY_F4\n");        break;
            case KEY_F(5):      printw("KEY_F5\n");        break;
            case KEY_F(6):      printw("KEY_F6\n");        break;
            case KEY_F(7):      printw("KEY_F7\n");        break;
            case KEY_F(8):      printw("KEY_F8\n");        break;
            case KEY_F(9):      printw("KEY_F9\n");        break;
            case KEY_F(10):     printw("KEY_F10\n");       break;
            case KEY_F(11):     printw("KEY_F11\n");       break;
            case KEY_F(12):     printw("KEY_F12\n");       break;
            case 'q':           goto main_done; break;
            default:            print_key(key); break;
        }
    }

main_done:
      
    keypad(stdscr, FALSE);
    echo();
    endwin();

    return 0;
}
