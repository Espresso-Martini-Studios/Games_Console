/**
* @file Game1_funcs.c
* @brief Secondary functions for Game_1.c; will merge with files in the shared folder when appropriate.
**/

#include "Game1_funcs.h"
#include "Joystick.h"
#include "LCD.h"
#include <stdint.h>

/*
Player
*/
void player_init(Player* player, int16_t row, int16_t column, int16_t x, int16_t y, int16_t width, int16_t height, int16_t progress, int16_t score) {
    player->row = row;
    player->column = column;
    player->x = x;
    player->y = y;
    player->width = width;
    player->height = height;
    player->progress = progress;
    player->score = score;
}

//void player_update(Player* player)

void player_draw(Player *player) {
    // just draw a circle for now - will make sprites later
    LCD_Draw_Circle(player->x, player->y, 4, 1, 1);
    return;
}