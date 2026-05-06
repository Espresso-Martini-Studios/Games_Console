/**
* @file Game1_funcs.c
* @brief Secondary functions for Game_1.c; will merge with files in the shared folder when appropriate.
**/

#include "Game1_funcs.h"
#include "Game1_sprites.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "LCD.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdlib.h> // for rand()

// VARIABLES, CONSTS AND STRUCTS

/* Grid organisation */
// note: ROW_HEIGHT and COLUMN_WIDTH declared in Game1_sprites.c

static Grid grid; // top left points of grid
static Grid player_grid; // grid

/* Player */

Direction player_direction = CENTRE;
static const int player_space = COLUMN_WIDTH - PLAYER_WIDTH; // how much smaller the sprite is to the ROW_HEIGHT or COLUMN_WIDTH

/* Block generation */

static const int max_trees = (2 * VISIBLE_COLUMNS) / 3; // limit tree numbers to a third of a row
static const int full_block_size = BLOCK_SIZE + 1; // block size *including* tree row
static const int rows_back_max = ((BLOCK_SIZE + 1) * NUM_BACKWARDS_BLOCKS) - 1; // maximum rows the player can travel back (-1 to view the row behind the player)

static const int car_loop_width = SCREEN_WIDTH + CAR_WIDTH;

static Block block_stack[NUM_BLOCKS]; // number of blocks loaded
static int farthest_block = 0;
static int current_block = 0;
static int row_in_block = 0;
static int next_block = 1;
static int prev_block = NUM_BLOCKS;


//FUNCTIONS

/*
Sprites (create flipped versions - scaling doesn't go negative)
*/
void sprites_init(void) {
    for (int row = 0; row < CAR_HEIGHT; row++) {
        for (int col = 0; col < CAR_WIDTH; col++) {
            CAR_SPRITE_FLIPPED[row][col] = CAR_SPRITE[row][(CAR_WIDTH - 1) - col];
        }
    }
}

/*
Grid organisation (see h file)
*/
// top left coordinate of all player points
void grid_init(void) {
    // account for player_space goes either side
    int playerGrid_addition = player_space / 2;
    for (int i = 0; i < VISIBLE_ROWS; i++) { // y coordinate of top of rows
        grid.row[i] = i * ROW_HEIGHT;
        player_grid.row[i] = grid.row[i] + playerGrid_addition;
    }
    for (int i = 0; i < VISIBLE_COLUMNS; i++) { // x coordinate of left of columns
        grid.column[i] = i * COLUMN_WIDTH;
        player_grid.column[i] = grid.column[i] + playerGrid_addition;
    }
}

/*
Player
*/


// initialise all player settings according to the game's needs
void player_init(Player* player) {
    player->row = VISIBLE_ROWS - 2;
    player->column = VISIBLE_COLUMNS / 2;
    player->progress = 0;
    player->score = 0;
    player->sprite = (uint8_t*) PLAYER_SPRITE; // need to convert to a "flat" uint8_t
    player_coordinate(player); // set x, y and block
}

// adjust x and y coordinates and current block based on changes to row, column or progress
void player_coordinate (Player* player) {
    player->x = player_grid.column[player->column];
    player->y = player_grid.row[player->row];
}

void player_update(Player* player, Direction player_direction) {
    switch (player_direction) {
        case N: // forwards
            // check for tree ahead
            if ((row_in_block == BLOCK_SIZE) && (block_stack[next_block].tree_row[player->column] == 1)) {
                break;
            }
            // check not moved backwards (prevent score increase with back and forth movement)
            if (player->progress == player->score) {
                player->score++;
            }
            player->progress++;
            break;
        case S: // backwards
            // check for tree behind
            if ((row_in_block == 1) && (block_stack[current_block].tree_row[player->column] == 1)) {
                break;
            }
            // stop progress going negative or furthest than the backwards limit
            if ((player->progress > 0) && (player->progress > (player->score - rows_back_max))) {
                player->progress--;
            }
            break;
        case E: // right
            // check for tree to the right
            if ((row_in_block == 0) && (block_stack[current_block].tree_row[player->column + 1] == 1)) {
                break;
            }
            if (player->column < (VISIBLE_COLUMNS - 1)) { // bounds check
                player->column++;
            }
            break;
        case W: // left
            // check for tree to the left
            if ((row_in_block == 0) && (block_stack[current_block].tree_row[player->column - 1] == 1)) {
                break;
            }
            if (player->column > 0) { // bounds check
                player->column--;
            }
            break;
        default: // centre
            break;    
    }
    player_coordinate(player);
}

int check_hit(Player* player) {
    // check x coordinates of player and object (account for sprite dimensions)
    if ((block_stack[current_block].type[row_in_block - 1] == ROAD) && (block_stack[current_block].object_position[row_in_block - 1] < (player->x + PLAYER_WIDTH)) && ((block_stack[current_block].object_position[row_in_block - 1] + CAR_WIDTH) > player->x)) {
        return 1;
    }
    else return 0;
}

void player_draw(Player* player) {
    LCD_Draw_Sprite(player->x, player->y, PLAYER_HEIGHT, PLAYER_WIDTH, player->sprite);
    return;
}

/*
Block generation
From a user standpoint, the world generates infinitely in chunks of fixed length ("blocks") 
with a tree row then further rows with roads or greenspace etc.
The user should be able to travel backwards NUM_BACKWARDS_BLOCKS.

The systems is based on a "block_stack" array of Block structs with NUM_BLOCKS blocks constantly loaded.
The player's score position (the furthestpoint it reached) will always be on NUM_BACKWARDS_BLOCKS block 
to allow for moving backwards.
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
    // clear starting point of a tree
    block_stack[0].tree_row[VISIBLE_COLUMNS / 2] = 0;
    // and clear row behind
    block_stack[NUM_BLOCKS - 1].type[BLOCK_SIZE] = GREENSPACE;
}

void generate_block(Block* block) {
    // reset tree row
    for (int i = 0; i < VISIBLE_COLUMNS; i++) {
        block->tree_row[i] = 0;
    }
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
            block->velocity[i] = 0.0;
            block->phase[i] = 0;
        }
        else {
            block->type[i] = ROAD;
            // randomise everything else
            block->phase[i] = rand() % SCREEN_WIDTH;
            // allow for left direction with negative velocity
            block->velocity[i] = (CAR_MIN_VELOCITY + ((float) rand()  / (float) RAND_MAX)) * (CAR_MAX_VELOCITY / (CAR_MIN_VELOCITY + 1)) * (1.0f - 2.0f * (rand() % 2));
        }
    }
}

void update_blocks(Player* player) {
    int prev_farthest_block = farthest_block;
    farthest_block = (player->score / full_block_size) % NUM_BLOCKS;
    current_block = (player->progress / full_block_size) % NUM_BLOCKS;
    row_in_block = player->progress % full_block_size;
    next_block = (current_block + 1) % NUM_BLOCKS;
    prev_block = (current_block + 9) % NUM_BLOCKS;
    // if we reach a new block, randomise block on other size of cyclic array
    if (prev_farthest_block != farthest_block) {
        generate_block(&block_stack[(farthest_block + (NUM_BLOCKS / 2)) % NUM_BLOCKS]);
        // we now have infinite world generation
    }
}

 // move the cars along
void update_objects(int animation_counter) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            // some maths:
            // object_position=(animation_counter+phase)*velocity
            // % car_loop_width to make it cyclical; car_loop_width = SCREEN_WIDTH + CAR_WIDTH due to -CAR_WIDTH later, starts loop as 0 to SCREEN_WIDTH+CAR_WIDTH
            // + car_loop_width) % car_loop_width to account for negatives
            // - CAR_WIDTH so it will cycle from -CAR_WIDTH to SCREEN_WIDTH or reverse if negative velocity
            block_stack[i].object_position[j] = (((((int) ((animation_counter + block_stack[i].phase[j]) * block_stack[i].velocity[j])) % car_loop_width) + car_loop_width) % car_loop_width) - CAR_WIDTH;
        }
    }
}

// only 50 (full_block_size * NUM_BLOCKS) rows loaded at once and NUM_BACKWARDS_BLOCKS is the block we started on
void blocks_draw(Player* player) {
    // a few different cases to consider depending on how far along we are
    switch (row_in_block) {
        case 0: // last row of last block, tree row of this block, rest of this block
            // need to render last row of block behind
            if (block_stack[prev_block].type[BLOCK_SIZE - 1] == ROAD) {
                road_draw(block_stack[prev_block].object_position[BLOCK_SIZE - 1], block_stack[prev_block].velocity[BLOCK_SIZE - 1], (VISIBLE_ROWS - 1));
            }
            // tree row of current_block
            treeRow_draw(&block_stack[current_block], (VISIBLE_ROWS - 2) + row_in_block);
            // render rest of current_block
            for (int i = 0; i < BLOCK_SIZE; i++) {
                if (block_stack[current_block].type[i] == ROAD) {
                    // first row = VISIBLE_ROWS, -1 due to indexing, -1 for row behind, -1 for tree row
                    // then increments down (closer to top of screen) as i increases
                    road_draw(block_stack[current_block].object_position[i], block_stack[current_block].velocity[i], (VISIBLE_ROWS - 3) - i);
                }
            }
            // don't render top 2 rows (title only)
            break;
        case 1: // tree row of this block, rest of this block, tree row of next block
            // tree row of current_block
            treeRow_draw(&block_stack[current_block], (VISIBLE_ROWS - 2) + row_in_block);
            // render rest of current_block
            for (int i = 0; i < BLOCK_SIZE; i++) {
                if (block_stack[current_block].type[i] == ROAD) {
                    // then as row_in_block increases we need to bring it further down so + row_in_block
                    road_draw(block_stack[current_block].object_position[i], block_stack[current_block].velocity[i], ((VISIBLE_ROWS - 3) - i) + row_in_block);
                }
            }
            // tree row of next block
            treeRow_draw(&block_stack[next_block], row_in_block + 1);
            break;
        default: // rest of this block, tree row of next block, rest of next block
            // start i increasingly higher as we load less of the current block
            for (int i = row_in_block - 2; i < BLOCK_SIZE; i++) {
                if (block_stack[current_block].type[i] == ROAD) {
                    road_draw(block_stack[current_block].object_position[i], block_stack[current_block].velocity[i], ((VISIBLE_ROWS - 3) - i) + row_in_block);
                }
            }
            // tree row of next block
            treeRow_draw(&block_stack[next_block], row_in_block + 1);
            // render rest of next block
            for (int i = 0; i < (row_in_block - 1); i++) {
                if (block_stack[next_block].type[i] == ROAD) {
                    // ((VISIBLE_ROWS - 6) - i) starts on row 1, + (row_in_block - 2) increases start row as player moves forwards
                    // -i moves it up the screen
                    road_draw(block_stack[next_block].object_position[i], block_stack[next_block].velocity[i], ((VISIBLE_ROWS - 6) - i) + (row_in_block - 2));
                }
            }
            break;
    }
}

void road_draw(uint16_t object_position, float velocity, uint16_t row) {
    LCD_Draw_Rect(0, grid.row[row], SCREEN_WIDTH, ROW_HEIGHT, 0, 1);
    if (velocity > 0) {
        LCD_Draw_Sprite(object_position, grid.row[row], CAR_HEIGHT, CAR_WIDTH, (uint8_t*) CAR_SPRITE);
    }
    else { // opposite way so flip sprite (can't scale negative)
        LCD_Draw_Sprite(object_position, grid.row[row], CAR_HEIGHT, CAR_WIDTH, (uint8_t*) CAR_SPRITE_FLIPPED);
    }
}

void treeRow_draw(Block* block, uint16_t row) {
    for (int col = 0; col < VISIBLE_COLUMNS; col++) {
        if (block->tree_row[col] == 1) {
            LCD_Draw_Sprite(grid.column[col], grid.row[row], ROW_HEIGHT, COLUMN_WIDTH, (uint8_t*) TREE_SPRITE);
        }
    }
}
