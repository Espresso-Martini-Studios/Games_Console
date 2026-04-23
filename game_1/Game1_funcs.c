/**
* @file Game1_funcs.c
* @brief Secondary functions for Game_1.c; will merge with files in the shared folder when appropriate.
**/

#include "Game1_funcs.h"
#include "Joystick.h"
#include "LCD.h"

void player_init(Player* player, int16_t init_x, int16_t init_y, int16_t init_width, int16_t init_height, int16_t init_progress, int16_t init_score) {
    player->x = init_x;
    player->y = init_y;
    player->width = init_width;
    player->height = init_height;
    player->progress = init_progress;
    player->score = init_score;
}

void player_draw(Player *player) {
    // just draw a circle for now - will make sprites later
    LCD_Draw_Circle(player->x, player->y, player->width, const uint8_t 1, const uint8_t fill)
}