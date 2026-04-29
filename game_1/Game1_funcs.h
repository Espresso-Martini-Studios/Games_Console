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

// Frame rate for this game (in milliseconds)
#define GAME1_FRAME_TIME_MS 30  // ~33 FPS
// Colours
#define COLOUR_BACKGROUND 3 // green
#define COLOUR_WRITING 0 // black

/* Grid organisation - rows and columns are those visible to player (fixed) */
/*  about the LCD  coordinates:
(0,0) is the top right corner
(240,240) is the bottom left

grid will be:
        column 1    column 2 ...    column 10
row 1
row 2
...
row 10
*/
#define VISIBLE_ROWS 8
#define VISIBLE_COLUMNS 9 // odd number so can start in middle
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

// points on grid
typedef struct {
    uint16_t row[VISIBLE_ROWS];
    uint16_t column[VISIBLE_COLUMNS];
} Grid;

void grid_init(Grid* grid);

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

void player_init(Player* player);

void player_coordinate (Player* player);

void player_update(Player* player, Direction player_direction);

void player_draw(Player* player);

#endif