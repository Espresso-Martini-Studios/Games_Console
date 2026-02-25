# Multi-Game Menu System - Student Collaboration Framework

## Overview

This project demonstrates how **3 students can work together** on the same codebase with a shared menu system, allowing each student to implement their own game in a dedicated folder.

### Project Structure

```
MenuTest/
├── Core/              # STM32 auto-generated files
├── Drivers/           # STM32 HAL drivers
├── shared/            # Shared utilities (Menu, Input, LCD)
├── game_1/            # Student 1's game
├── game_2/            # Student 2's game
├── game_3/            # Student 3's game
├── Joystick/          # Joystick driver
├── PWM/               # PWM driver
├── Buzzer/            # Buzzer driver
└── CMakeLists.txt     # Build config
```

## How It Works

### The Main Game Loop

The system uses a **simple, centralized game loop** in `main.c`:

```c
while(1) {
    // Step 1: Read all input
    Input_Read();
    
    // Step 2: Update based on current state
    switch(current_state) {
        case MENU_SCREEN:     Menu_Update();    break;
        case GAME_1_SCREEN:   Game1_Update();   break;
        case GAME_2_SCREEN:   Game2_Update();   break;
        case GAME_3_SCREEN:   Game3_Update();   break;
    }
    
    // Step 3: Render based on current state  
    switch(current_state) {
        case MENU_SCREEN:     Menu_Render();    break;
        case GAME_1_SCREEN:   Game1_Render();   break;
        case GAME_2_SCREEN:   Game2_Render();   break;
        case GAME_3_SCREEN:   Game3_Render();   break;
    }
}
```

### Navigation Flow

```
Start
  ↓
Main Menu
  ├─ Select "Game 1" → Game 1 (press BT3 to return)
  ├─ Select "Game 2" → Game 2 (press BT3 to return)
  └─ Select "Game 3" → Game 3 (press BT3 to return)
```

## For Each Student

Each student works in their own game folder with three simple functions:

### File: `game_X/Game_X.c`

```c
void GameX_Init(void) {
    // Called once when your game is selected from the menu
    // Initialize your game state here
}

void GameX_Update(void) {
    // Called 30 times per second
    // Update your game logic, check input, etc.
}

void GameX_Render(void) {
    // Called 30 times per second (after Update)
    // Draw everything to the LCD
}
```

## Input Handling

Buttons (BT2 and BT3) use **interrupt-driven input** for fast response:

```c
void Game1_Update(void) {
    extern InputState current_input;
    extern MenuSystem menu;
    
    // Check if BT3 was pressed this frame
    if (current_input.btn3_pressed) {
        // Return to menu
        Menu_BackToHome(&menu);
    }
    
    // Check if BT2 was pressed this frame
    if (current_input.btn2_pressed) {
        // Your custom button action
    }
    
    // Your game logic
}
```

## Shared Resources

All games can use:

- **LCD Display**: Draw text and graphics
- **Joystick**: Read X/Y analog values (see Joystick.h)
- **Buzzer**: Play sounds (see Buzzer.h)
- **PWM/LED**: Control brightness (see PWM.h)
- **Timers**: TIM6 (100Hz) and TIM7 (1Hz) for game timing (see TIMER_USAGE_GUIDE.md)
- **Menu System**: Navigate back to menu
- **Input State**: Check if buttons were pressed

## Quick Start - Implement Your Game

1. **Edit `game_1/Game_1.c`** (or game_2/Game_2.c or game_3/Game_3.c)

2. **Implement the three functions**:

```c
#include "Game_1.h"
#include "LCD.h"
#include "InputHandler.h"
#include "Menu.h"

void Game1_Init(void) {
    // Initialize
}

void Game1_Update(void) {
    extern InputState current_input;
    extern MenuSystem menu;
    
    if (current_input.btn3_pressed) {
        Menu_BackToHome(&menu);
    }
    // Your game logic
}

void Game1_Render(void) {
    extern ST7789V2_cfg_t cfg0;
    
    LCD_Fill_Buffer(0);
    LCD_printString("Your Game!", 50, 50, 1, 2);
    LCD_Refresh(&cfg0);
}
```

3. **Build and run**:
```bash
cd MenuTest
mkdir -p build/Debug
cd build/Debug
cmake -GNinja ../.. -DCMAKE_BUILD_TYPE=Debug
ninja
```

## Key Global Variables You Can Access

In your game functions, you can use these external variables:

```c
// Input state
extern InputState current_input;        // Check if BT3 was pressed
extern MenuSystem menu;                 // Access menu state

// Display  
extern ST7789V2_cfg_t cfg0;            // LCD configuration for rendering

// Hardware (initialized in main.c)
extern Joystick_cfg_t joystick_cfg;    // Joystick configuration
extern Joystick_t joystick_data;       // Current joystick readings
```

## Frame Rate

The system runs at approximately **30 FPS** (FRAME_TIME_MS = 30ms).

You can use frame counting in your game:

```c
static uint32_t frame_count = 0;

void Game1_Update(void) {
    frame_count++;
    
    // Do something every 15 frames (~2 times per second)
    if (frame_count % 15 == 0) {
        // ...
    }
}
```

## Example: Simple Bouncing Game

```c
#include "Game_1.h"
#include "LCD.h"
#include "InputHandler.h"
#include "Menu.h"

static int x = 120, y = 160, vx = 2, vy = 2;

void Game1_Init(void) {
    x = 120;
    y = 160;
    vx = 2;
    vy = 2;
}

void Game1_Update(void) {
    extern InputState current_input;
    extern MenuSystem menu;
    
    if (current_input.btn3_pressed) {
        Menu_BackToHome(&menu);
        return;
    }
    
    x += vx;
    y += vy;
    
    if (x <= 0 || x >= 230) vx = -vx;
    if (y <= 0 || y >= 230) vy = -vy;
}

void Game1_Render(void) {
    extern ST7789V2_cfg_t cfg0;
    
    LCD_Fill_Buffer(0);
    LCD_printString("o", x, y, 1, 2);
    LCD_printString("Bounce Game", 40, 10, 1, 2);
    LCD_Refresh(&cfg0);
}
```

## Collaboration Notes

Since students work in separate folders:
- **No merge conflicts** on game code
- **Shared infrastructure** in `shared/` folder
- **Independent development** - each student controls their game
- **Common interface** - all games follow same Update/Render pattern

## Hardware

- **MCU**: STM32L476
- **LCD**: ST7789V2 display
- **Input**: Joystick + BT2 and BT3 buttons
- **Output**: PWM LED, Buzzer

## Troubleshooting

### Game doesn't render
- Check that `LCD_Refresh(&cfg0)` is called in your Render function
- Verify includes: `#include "LCD.h"`

### Button press not detected
- Make sure to check `current_input.btn3_pressed` in Update (not elsewhere)
- Remember to call `Menu_BackToHome(&menu)` to return

### Game won't compile
- Check that all forward declarations are correct in headers
- Verify `extern MenuSystem menu;` in your game functions

## Next Steps

1. Implement a simple game in your game_X folder
2. Test with the menu system
3. Extend to use Joystick input (see Joystick.h)
4. Add sounds with Buzzer (see Buzzer.h)
5. Add more features and polish

Good luck, and happy coding!
