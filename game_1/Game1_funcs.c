/**
* @file Game1_funcs.c
* @brief Secondary functions for Game_1.c; will merge with files in the shared folder when appropriate.
**/

#include "Game1_funcs.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "LCD.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdlib.h> // for rand()

/* Grid organisation */

static const uint16_t row_height = SCREEN_HEIGHT / VISIBLE_ROWS;
static const uint16_t column_width = SCREEN_WIDTH / VISIBLE_COLUMNS;
static uint16_t row_topCoord[VISIBLE_ROWS]; // top coordinate of rows

static Grid grid; // midpoints on grid

/* Player */

Direction player_direction = CENTRE;

// maximum rows the player can travel back (-1 to view the row behind the player)
static const int rows_back_max = ((BLOCK_SIZE + 1) * NUM_BACKWARDS_BLOCKS) - 1;

/* Block generation */

static const int max_trees = VISIBLE_COLUMNS / 2; // limit tree numbers to half a row
static const int full_block_size = BLOCK_SIZE + 1; // block size *including* tree row

static Block block_stack[NUM_BLOCKS]; // number of blocks loaded

/*
Grid organisation (see h file)
*/

void grid_init(void) {
    for (int i = 0; i < VISIBLE_ROWS; i++) { // y coordinate of middle of rows
        grid.row[i] = (row_height / 2) + (i * row_height);
    }
    for (int i = 0; i < VISIBLE_COLUMNS; i++) { // x coordinate of middle of columns
        grid.column[i] = (column_width / 2) + (i * column_width);
    }
    // assign top coordinate of rows
    for (int i = 0; i < VISIBLE_ROWS; i++) {
        row_topCoord[i] = i * row_height;
    }
}

/*
Player
*/


// initialise all player settings according to the game's needs
void player_init(Player* player) {
    player->row = VISIBLE_ROWS - 2;
    player->column = VISIBLE_COLUMNS / 2;
    player->width = 0;
    player->height = 0;
    player->progress = 0;
    player->score = 0;
    player_coordinate(player); // set x, y and block
}

// adjust x and y coordinates and current block based on changes to row, column or progress
void player_coordinate (Player* player) {
    player->x = grid.column[player->column];
    player->y = grid.row[player->row];
    // blocks repeat every NUM_BLOCKS - see block generation section
    // player->block is furthest block traveled
    player->block = (player->score / full_block_size) % NUM_BLOCKS;
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
            // stop progress going negative or before BLOCK_SIZE*NUM_BACKWARDS_BLOCKS rows (note: slightly smaller than NUM_BACKWARDS_BLOCKS blocks due to BLOCK_SIZE not including tree row)
            if ((player->progress > 0) && (player->progress > (player->score - (BLOCK_SIZE * NUM_BACKWARDS_BLOCKS)))) {
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

/*
Block generation
From a user standpoint, the world generates infinitely in chunks of fixed length ("blocks") with a tree row then further rows with roads or greenspace etc.
The user should be able to travel backwards NUM_BACKWARDS_BLOCKS.

The systems is based on a "block_stack" array of Block structs with NUM_BLOCKS blocks constantly loaded.
The player's score position (the furthestpoint it reached) will always be on NUM_BACKWARDS_BLOCKS block to allow for moving backwards.
The player is stopped from moving backwards beyond this by player_update function.
full_block_size is created because BLOCK_SIZE doesn't include the tree row.
When score reaches a mutltiple of full_block_size,
*/

void blockGen_init(void) {
    // seed random library
    srand((unsigned int) HAL_GetTick());
    // generate all blocks
    for (int i = 0; i < NUM_BLOCKS; i++) {
        generate_block(&block_stack[i]);
    }
    // clear first tree row (starting point)
    for (int i; i < VISIBLE_COLUMNS; i++) {
        block_stack[0].tree_row[i] = 0;
    }
    // and clear row behind
    block_stack[NUM_BLOCKS].type[BLOCK_SIZE - 1] = GREENSPACE;
}

void generate_block(Block* block) {
    // randomise number of trees for tree row
    int num_trees = rand() % (max_trees + 1);
    for (int i = 0; i < num_trees; i++) {
        // assign trees randomly in tree row
        block->tree_row[rand() % VISIBLE_COLUMNS] = 1;
    }
    // now assign other rows
    for (int i = 0; i < BLOCK_SIZE; i++) {
        // 1/4 chance the row is greenspace (note: magic number)
        if ((rand() % 4) == 0) {
            block->type[i] = GREENSPACE;
            // everything else is 0 (doesn't matter)
            block->direction[i] = 0;
            block->speed[i] = 0.0;
            block->phase[i] = 0;
        }
        else {
            block->type[i] = ROAD;
            // randomise everything else
            block->direction[i] = rand() % 2; // enumeration from 0
            block->phase[i] = rand() % SCREEN_WIDTH;
            block->speed[i] = ((float) rand() / RAND_MAX) * CAR_MAX_SPEED;
        }
    }
}

void road_draw(Block* block, int row) {
    LCD_Draw_Rect(0, row_topCoord[row], SCREEN_WIDTH, row_height, 0, 1);
}

// only 50 (full_block_size * NUM_BLOCKS) rows loaded at once and NUM_BACKWARDS_BLOCKS is the block we started on
void blocks_draw(Player* player) {
    // player->block only means the furthest block traveled
    // calculate current block (index for block_stack)
    int current_block = (player->progress / full_block_size) % NUM_BLOCKS;
    int row_in_block = player->progress % full_block_size;
    // note: calulate blocks behind with (current_block + NUM_BLOCKS - 1) % NUM_BLOCKS
    // if we are in the first row of current_block, need to load the last row of the block before
    switch (row_in_block) {
        case 0: // last row of last block, tree row of this block, rest of this block
            // need to render last row of block behind
            if (block_stack[current_block - 1].type[BLOCK_SIZE - 1] == ROAD) {
                road_draw(&block_stack[current_block - 1], (VISIBLE_ROWS - 1));
            }
            // tree row of current_block - finish later
            // render rest of current_block
            for (int i = 0; i < BLOCK_SIZE; i++) {
                if (block_stack[current_block].type[i] == ROAD) {
                    // first row = VISIBLE_ROWS, -1 due to indexing, -1 for row behind, -1 for tree row
                    // then increments down (closer to top of screen) as i increases
                    road_draw(&block_stack[current_block], (VISIBLE_ROWS - 3) - i);
                }
            }
            // don't render top row (title only)
            break;
        case 1: // tree row of this block, rest of this block, tree row of next block
            // tree row of current_block - finish later
            // render rest of current_block
            for (int i = 0; i < BLOCK_SIZE; i++) {
                if (block_stack[current_block].type[i] == ROAD) {
                    // then as row_in_block increases we need to bring it further down so + row_in_block
                    road_draw(&block_stack[current_block], ((VISIBLE_ROWS - 3) - i) + row_in_block);
                }
            }
            // tree row of next block - finish later
            break;
        default: // rest of this block, tree row of next block, rest of next block
            // start i increasingly higher as we load less of the current block
            for (int i = row_in_block - 2; i < BLOCK_SIZE; i++) {
                if (block_stack[current_block].type[i] == ROAD) {
                    road_draw(&block_stack[current_block], ((VISIBLE_ROWS - 3) - i) + row_in_block);
                }
            }
            // tree row of next block - finish later
            // render rest of next block
            for (int i = 0; i < (row_in_block - 1); i++) {
                if (block_stack[current_block + 1].type[i] == ROAD) {
                    // ((VISIBLE_ROWS - 6) - i) starts on row 1, + (row_in_block - 2) increases start row as player moves forwards
                    // -i moves it up the screen
                    road_draw(&block_stack[current_block], ((VISIBLE_ROWS - 6) - i) + (row_in_block - 2));
                }
            }
            break;
    }
}