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
#define COLOUR_BACKGROUND 10 // green
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
#define ROW_HEIGHT (SCREEN_HEIGHT / VISIBLE_ROWS)
#define COLUMN_WIDTH (SCREEN_WIDTH / VISIBLE_COLUMNS)

// points on grid
typedef struct {
    uint16_t row[VISIBLE_ROWS];
    uint16_t column[VISIBLE_COLUMNS];
} Grid;

void grid_init(void);

/* Player section */

#define PLAYER_WIDTH 20
#define PLAYER_HEIGHT 24

typedef struct {
    // need to use int16_t for LCD functions to work properly
    int16_t row;
    int16_t column;
    int16_t x;
    int16_t y;
    // progress means how far along the sprite is (should equal score until moving backwards)
    int16_t progress;
    // the farthest progress the player has made
    int16_t score;
    // sprite image (added to struct in case we want animations)
    uint8_t* sprite; // make pointer for memory purposes
} Player;

void player_init(Player* player);

void player_coordinate (Player* player);

void player_update(Player* player, Direction player_direction);

int check_hit(Player* player); // check if the player is hit

void player_draw(Player* player);

/* Block generation */
/*
I don't want the game to freeze from loading new frames while on a road (as this would affect player experience more).
Therefore, I will generate the world in blocks with the first row being a tree row and the others random
*/


#define BLOCK_SIZE 4 // number of rows after the tree row
#define NUM_BLOCKS 10 // number of blocks loaded
#define NUM_BACKWARDS_BLOCKS 2 // the blocks the player can travel backwards

#define CAR_MAX_VELOCITY 5.0f // max pixels car travels per iteration
#define CAR_MIN_VELOCITY 1.0f
#define CAR_HEIGHT ROW_HEIGHT
#define CAR_WIDTH (COLUMN_WIDTH * 2)

typedef enum {
    GREENSPACE,
    ROAD // add river eventually
} RowType;

typedef struct {
    // first row will be the tree row
    int tree_row[VISIBLE_COLUMNS]; // logical array for where trees are
    // other rows will be roads, rivers, or greenspace
    RowType type[BLOCK_SIZE];
    float velocity[BLOCK_SIZE]; // number of pixels per frame
    int phase[BLOCK_SIZE]; // how far along the road compared to other cards
    uint16_t object_position[BLOCK_SIZE]; // where the car is (phase * velocity)
} Block;

void blockGen_init(void); // initialise block generation variables

void generate_block(Block* block);

void update_blocks(Player* player);

void update_objects(int animation_counter);

void road_draw(uint16_t object_position, float velocity, uint16_t row); // draw road with cars

void treeRow_draw(Block* block, uint16_t row);

void blocks_draw(Player* player); // includes object

#endif