#include "Game_1.h"
#include "Game1_funcs.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control

// enums
static Game_State game_state;
static Direction player_direction = CENTRE;
// structs
static Player player;
extern Grid grid; // from Game1_funcs.c so don't have to keep passing over
// variables
static uint32_t frame_start = 0; // for HAL

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
    grid_init(&grid);
    // player
    player_init(&player);
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
    
    // main game
    player_draw(&player);
    // grid test ******************************
/*
    for (int i = 0; i < sizeof(column_midpoint)/sizeof(column_midpoint[0]); i++) {
        for (int j = 0; j < sizeof(row_midpoint)/sizeof(row_midpoint[0]); j++) {
            LCD_Draw_Circle(column_midpoint[i], row_midpoint[j], 4, 1, 1);
            printf("Circle coordinate: %u, %u", column_midpoint[i], row_midpoint[j]);
        }
    }
*/
    LCD_Refresh(&cfg0);

    // Frame timing - wait for remainder of frame time
    uint32_t frame_time = HAL_GetTick() - frame_start;
    if (frame_time < GAME1_FRAME_TIME_MS) {
        HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
    }
}