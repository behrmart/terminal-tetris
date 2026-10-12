#include <ncurses.h>

#include "input.h"

Action input_poll(void)
{
    const int key = getch();

    if (key == 'q' || key == 'Q') {
        return ACTION_QUIT;
    }
    return ACTION_NONE;
}
