#include "InputHandler.h"
#include "Joystick.h"
#include "main.h"

#define BTN_DEBOUNCING_DELAY 200
#define JOYSTICK_DELAY 300

/* Button input */

// Global input state
InputState current_input = {0};

// Track button presses in interrupt
static volatile uint8_t btn2_raw_press = 0;
static volatile uint8_t btn3_raw_press = 0;

void Input_Init(void) {
    // GPIO and EXTI already initialized by MX_GPIO_Init() in main.c
    // Just reset the state
    current_input.btn2_pressed = 0;
    current_input.btn3_pressed = 0;
    btn2_raw_press = 0;
    btn3_raw_press = 0;
}

void Input_Read(void) {
    // Copy the button press flags from interrupt to current input state
    // This is read once per frame by the main loop
    current_input.btn2_pressed = btn2_raw_press;
    current_input.btn3_pressed = btn3_raw_press;
    
    // Reset the flags after reading so they only trigger once
    btn2_raw_press = 0;
    btn3_raw_press = 0;
}

// ===== INTERRUPT CALLBACK FOR BUTTONS =====
// Called by hardware when button is pressed
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    static uint32_t last_btn2_interrupt = 0;
    static uint32_t last_btn3_interrupt = 0;
    uint32_t current_time = HAL_GetTick();

    // Handle BT2
    if (GPIO_Pin == BTN2_Pin) {
        // Software debouncing (200ms)
        if ((current_time - last_btn2_interrupt) > BTN_DEBOUNCING_DELAY) {
            last_btn2_interrupt = current_time;
            
            // Toggle LED to indicate button press
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            
            // Set flag indicating button was pressed
            btn2_raw_press = 1;
        }
    }
    
    // Handle BT3 (joystick button)
    if (GPIO_Pin == BTN3_Pin) {
        // Software debouncing (200ms)
        if ((current_time - last_btn3_interrupt) > BTN_DEBOUNCING_DELAY) {
            last_btn3_interrupt = current_time;
            
            // Toggle LED to indicate button press
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            
            // Set flag indicating button was pressed
            btn3_raw_press = 1;
        }
    }
}

/* Joystick input with delay (burst movement) for Game_1 */

extern Joystick_cfg_t joystick_cfg;  // configuration
extern Joystick_t joystick_data;     // data
// need to compare current and last directions
Direction last_direction = CENTRE;
Direction current_direction = CENTRE;
// need to compare times the joystick has been pressed
static uint32_t last_joystick_interrupt = 0;
static uint32_t current_joystick_interrupt = 0;

// allow "holding down" the joystick in one direction and moves forward in bursts (defined by JOYSTICK_DELAY)
// also want to move faster than this if the joystick to returns to centre in between moves
// I will call this "burst movement"
Direction burstMove_getDirection(void) {
    current_joystick_interrupt = HAL_GetTick();
    // read current joystick input
    Joystick_Read(&joystick_cfg, &joystick_data);
    current_direction = joystick_data.direction;
    // always return current input if in centre
    if (current_direction == CENTRE) {
        last_direction = current_direction;
        last_joystick_interrupt = current_joystick_interrupt;
        return current_direction;
        // this means if we go to the centre in between moves, we can go as fast as we like
    }
    // always return current direction if it has changed
    else if ((current_direction != last_direction)) {
        last_direction = current_direction;
        last_joystick_interrupt = current_joystick_interrupt;
        return current_direction;
    }
    // if not in centre, only return input once the JOYSTICK_DELAY time has passed
    else if ((current_direction == last_direction) && ((current_joystick_interrupt - last_joystick_interrupt) > JOYSTICK_DELAY)) {
        last_direction = current_direction;
        last_joystick_interrupt = current_joystick_interrupt;
        return current_direction;
    }
    // time hasn't passed and input is the same, return CENTRE so the player doesn't move and nothing reset
    else {
        return CENTRE;
    }
}