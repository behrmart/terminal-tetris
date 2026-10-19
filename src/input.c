#include "input.h"

#include <ncurses.h>

Action input_poll_navigation(bool *main_menu)
{
    *main_menu = false;
    switch (getch()) {
    case 'm':
    case 'M':       *main_menu = true; return ACTION_NONE;
    case KEY_LEFT:  return ACTION_LEFT;
    case KEY_RIGHT: return ACTION_RIGHT;
    case KEY_DOWN:  return ACTION_SOFT_DROP;
    case KEY_UP:
    case 'x':
    case 'X':       return ACTION_ROTATE_CW;
    case 'z':
    case 'Z':       return ACTION_ROTATE_CCW;
    case ' ':       return ACTION_HARD_DROP;
    case 'p':
    case 'P':       return ACTION_PAUSE;
    case 'r':
    case 'R':       return ACTION_RESTART;
    case 'q':
    case 'Q':       return ACTION_QUIT;
    default:       return ACTION_NONE; /* Includes timeout and KEY_RESIZE. */
    }
}

Action input_poll(void)
{
    bool main_menu = false;
    return input_poll_navigation(&main_menu);
}
