/**
* @file Game1_funcs.h
* @brief Secondary functions for Game_1; will merge with files in the shared folder when appropriate.
*
* Will use for functions called by Game1_Run, Game1_Init, Game1_Update, or Game1_Render. These are the primary functions.
**/

#ifndef GAME1_FUNCS_H
#define GAME1_FUNCS_H

#include <stdint.h>
#include "Utils.h"
#include "Joystick.h"

/* Movement section */
//Direction get_direction(void)

/* Player section */

typedef struct {
    // need to use int16_t for LCD functions to work properly
    int16_t row;
    int16_t column;
    int16_t x;
    int16_t y;
    // player dimensions
    int16_t width;
    int16_t height;
    // progress means how far along the sprite is (should equal score until moving backwards)
    int16_t progress;
    int16_t score;
} Player;

void player_init(Player* player, int16_t row, int16_t column, int16_t x, int16_t y, int16_t width, int16_t height, int16_t progress, int16_t score);

void player_update(Player* player, UserInput input);

void player_draw(Player* player);

#endif