/**
* @file Game1_funcs.c
* @brief Secondary functions for Game_1.c; will merge with files in the shared folder when appropriate.
**/

#include "Game1_funcs.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "LCD.h"
#include <stdint.h>

/*
Grid organisation (see h file)
*/

static const uint16_t row_height = SCREEN_HEIGHT / VISIBLE_ROWS;
static const uint16_t column_width = SCREEN_WIDTH / VISIBLE_COLUMNS;

Grid grid; // midpoints on grid

void grid_init(Grid* grid) {
    for (int i = 0; i < VISIBLE_ROWS; i++) { // y coordinate of middle of rows
        grid->row[i] = (row_height / 2) + (i * row_height);
    }
    for (int i = 0; i < VISIBLE_COLUMNS; i++) { // x coordinate of middle of columns
        grid->column[i] = (column_width / 2) + (i * column_width);
    }
}

/*
Player
*/

Direction player_direction = CENTRE;

// initialise all player settings according to the game's needs
void player_init(Player* player) {
    player->row = VISIBLE_ROWS - 2;
    player->column = VISIBLE_COLUMNS / 2;
    player_coordinate(player); // set x and y from row and column
    player->width = 0;
    player->height = 0;
    player->progress = 0;
    player->score = 0;
}

void player_coordinate (Player* player) {
    player->x = grid.column[player->column];
    player->y = grid.row[player->row];
}

void player_update(Player* player, Direction player_direction) {
    switch (player_direction) {
        case N: // forwards
            // check not moved backwards (prevent score increase with back and forth movement)
            if (player->progress == player->score) {
                player->score++;
            }
            player->progress++;
            break;
        case S: // backwards
            if (player->progress != 0) { // stop progress going negative
                player->progress--;
            }
            break;
        case E: // right
            if (player->column < (VISIBLE_COLUMNS - 1)) { // bounds check
                player->column++;
            }
            break;
        case W: // left
            if (player->column > 0) { // bounds check
                player->column--;
            }
            break;
        default:
            break;    
    }
    player_coordinate(player);
}

void player_draw(Player* player) {
    // just draw a circle for now - will make sprites later
    LCD_Draw_Circle(player->x, player->y, 4, 1, 1);
    return;
}