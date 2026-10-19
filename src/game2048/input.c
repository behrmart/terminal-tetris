#include "game2048/input.h"

#include <ncurses.h>

Game2048Input game2048_input_poll(void)
{
    Game2048Input input = {GAME2048_ACTION_NONE, 0U};
    switch (getch()) {
    case KEY_LEFT:  input.action = GAME2048_ACTION_LEFT; break;
    case KEY_RIGHT: input.action = GAME2048_ACTION_RIGHT; break;
    case KEY_UP:    input.action = GAME2048_ACTION_UP; break;
    case KEY_DOWN:  input.action = GAME2048_ACTION_DOWN; break;
    case 'r':
    case 'R': input.action = GAME2048_ACTION_RESTART; break;
    case 'c':
    case 'C': input.action = GAME2048_ACTION_CONTINUE; break;
    case 'q':
    case 'Q': input.action = GAME2048_ACTION_QUIT; break;
    case '4':
    case 10:
    case 13:
    case KEY_ENTER: input.selected_size = 4U; break;
    case '6': input.selected_size = 6U; break;
    default: break;
    }
    return input;
}
