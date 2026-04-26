#include "Game_1.h"
#include "Game1_funcs.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control

// Frame rate for this game (in milliseconds)
#define GAME1_FRAME_TIME_MS 30  // ~33 FPS
// Grid organisation - rows and columns are those visible to player (fixed)
#define VISIBLE_ROWS 10
#define VISIBLE_COLUMNS 9 // odd number so can start in middle
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240
// Colours
#define COLOUR_BACKGROUND 3 // green
#define COLOUR_WRITING 0 // black

// enums
static Game_State game_state = PLAYING;
// structs
static Player player;
// const
static const int row_height = SCREEN_HEIGHT / VISIBLE_ROWS;
static const int column_width = SCREEN_WIDTH / VISIBLE_COLUMNS;
// variables
static uint32_t frame_start = 0; // for HAL
// can't make row positions constant while using for loop to assign, though they won't change
static int row_midpoint[VISIBLE_ROWS];
static int column_midpoint[VISIBLE_COLUMNS];

MenuState Game1_Run(void) {
    Game1_Init();
    MenuState exit_state = MENU_STATE_HOME;  // Default: return to menu
    
    // startup sound
    buzzer_tone(&buzzer_cfg, 1000, 30);  // 1kHz at 30% volume
    HAL_Delay(50);  // Brief beep duration
    buzzer_off(&buzzer_cfg);  // Stop the buzzer
    
    // main game loop
    int game_loop = 1;
    while (game_loop) {
        switch (game_state) {
            case PLAYING:
                Game1_Update();
                Game1_Render();
                break;
            case ENDED:
                PWM_SetDuty(&pwm_cfg, 50);  // Reset LED to 50% when returning
                exit_state = MENU_STATE_HOME; // safety line
                game_loop = 0;
                break;  // Exit game loop
            default:
                break;
        }
    }

    return exit_state;  // Tell main where to go next
}

/* Game Initialisation */
void Game1_Init(void) {
    game_state = PLAYING;
    // assign position coordinates
    for (int i = 0; i < VISIBLE_ROWS; i++) {
        row_midpoint[i] = (row_height / 2) + (i * row_height);
    }
    for (int j = 0; j < VISIBLE_COLUMNS; j++) {
        column_midpoint[j] = (column_width / 2) + (j * column_width);
    }
    // player
    player_init(PLayer* player, int16_t 0, int16_t 0, int16_t 0, int16_t 0, int16_t 0, int16_t 0);
}

/* Game Update */
void Game1_Update(void) {
    frame_start = HAL_GetTick();
    Input_Read();
        
    // Check if button was pressed to return to menu 
    if (current_input.btn3_pressed) {
        game_state = ENDED;
    }
}

void Game1_Render(void) {
    // background
    LCD_Fill_Buffer(COLOUR_BACKGROUND);
        
    // title
    LCD_printString("CATTER", 60, 10, COLOUR_WRITING, 3);
        
    // instructions
    LCD_printString("Press BT3 to", 40, 220, COLOUR_WRITING, 1);
    LCD_printString("Return to Menu", 40, 235, COLOUR_WRITING, 1);
        
    LCD_Refresh(&cfg0);
    
    // main game
    player.draw(Player* player);

    // Frame timing - wait for remainder of frame time
    uint32_t frame_time = HAL_GetTick() - frame_start;
    if (frame_time < GAME1_FRAME_TIME_MS) {
        HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
    }
}