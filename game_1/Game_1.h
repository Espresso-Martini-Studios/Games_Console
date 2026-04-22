#ifndef GAME_1_H
#define GAME_1_H

#include "Menu.h"

// Game state enum for switch-case format before main loop

typedef enum {
    GAME_MENU,
    PLAYING,
    ENDED
} Game_State;

/**
 * @brief Game 1 - Student can implement their own game here
 * 
 * Placeholder for Student 1's game implementation.
 * This structure allows multiple students to work on separate games
 * while sharing common utilities from the shared/ folder.
 * 
 * The menu system calls this function when Game 1 is selected.
 * The function runs its own loop and returns when the game exits.
 * 
 * @return MenuState - Where to go next (typically MENU_STATE_HOME for menu)
 */

void Game1_Init(void); // sets up game when opened
void Game1_Update(void); // updates game states (score, position, etc.)
void Game1_Render(void); // renders updates onto LCD buffer then refreshes

MenuState Game1_Run(void);

#endif // GAME_1_H
