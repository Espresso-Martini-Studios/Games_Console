#include "Game_1.h"
#include "Game1_funcs.h"
#include "Game1_sprites.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/_intsup.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control

// enums
static Game_State game_state;
static Direction player_direction = CENTRE;
// structs
static Player player;
// variables
static int animation_counter = 0;
static uint32_t frame_start = 0; // for HAL

MenuState Game1_Run(void) {
    // set colour palette
    LCD_Set_Palette(PALETTE_VINTAGE);
    Game1_Init();
    MenuState exit_state = MENU_STATE_HOME;  // Default: return to menu
    
    // startup sound
    buzzer_tone(&buzzer_cfg, 1000, 30);  // 1kHz at 30% volume
    HAL_Delay(50);  // Brief beep duration
    buzzer_off(&buzzer_cfg);  // Stop the buzzer
    
    // main game loop
    game_state = PLAYING;
    int game_loop = 1;
    while (game_loop) {
        switch (game_state) {
            case PLAYING:
                Game1_Update();
                Game1_Render();
                break;
            case HIT:
                hit_menu();
                break;
            case ENDED:
                PWM_SetDuty(&pwm_cfg, 50);  // Reset LED to 50% when returning
                exit_state = MENU_STATE_HOME; // safety line
                game_loop = 0; // will break while statement
                break;
            default:
                break;
        }
    }
    LCD_Set_Palette(PALETTE_DEFAULT);
    return exit_state;  // Tell main where to go next
}

/* Game Initialisation */
void Game1_Init(void) {
    animation_counter = 0;
    game_state = PLAYING;
    grid_init();
    // player
    player_init(&player);
    // block generation
    blockGen_init();
    sprites_init();
    update_blocks(&player);
    update_objects(animation_counter++);
}

/* Game Update */
void Game1_Update(void) {
    frame_start = HAL_GetTick();
    Input_Read();
    player_direction = burstMove_getDirection();
    player_update(&player, player_direction);
    update_blocks(&player);
    update_objects(animation_counter++);
    if (check_hit(&player)) {
        game_state = HIT; // uses current_block and row_in_block calculated by update_blocks
        return;
    }
    // Check if button was pressed to return to menu 
    if (current_input.btn3_pressed) {
        game_state = ENDED;
    }
}

void Game1_Render(void) {
    // background
    LCD_Fill_Buffer(COLOUR_BACKGROUND);
        
    // title
    LCD_printString("CATTER", 10, 10, COLOUR_WRITING, 3);
        
    // score
    char score_text[20];
    snprintf(score_text, sizeof(score_text), "Score: %u", player.score);
    LCD_printString(score_text, 10, 35, COLOUR_WRITING, 2);
    
    // main game
    blocks_draw(&player);
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

void hit_menu(void) {
    // render the menu
    LCD_Fill_Buffer(0);
    LCD_printString("CATTER", 10, 10, 2, 3);
    LCD_printString("Cat has been hit!", 20, 50, 2, 2);
    LCD_printString("Press Btn 3 to", 20, 100, 2, 2);
    LCD_printString("return to main menu.", 20, 120, 2, 2);
    LCD_Refresh(&cfg0);
    int been_hit = 1;
    while (been_hit) {
        Input_Read();
        if (current_input.btn3_pressed) {
            Input_Read();
            game_state = ENDED;
            been_hit = 0;
        }
    }

}