#ifndef JOYSTICK_H___
#define JOYSTICK_H___
#include <cstdint>

#define JOYSTICK_A_BUTTON static_cast<gpio_num_t>(CONFIG_BUTTON_A_GPIO) // GPIO_NUM_16 // orange
#define JOYSTICK_B_BUTTON static_cast<gpio_num_t>(CONFIG_BUTTON_B_GPIO) // GPIO_NUM_17 // blue
#define JOYSTICK_C_BUTTON static_cast<gpio_num_t>(CONFIG_BUTTON_C_GPIO) // GPIO_NUM_21 // yellow
#define JOYSTICK_D_BUTTON static_cast<gpio_num_t>(CONFIG_BUTTON_D_GPIO) // GPIO_NUM_19 // gray

#define MASK_A 1
#define MASK_B 2
#define MASK_C 4
#define MASK_D 8

void initButtons();
uint8_t readButtons();

enum JoyDir
{
    JOY_UP = 0x01,    // A
    JOY_DOWN = 0x08,  // D
    JOY_LEFT = 0x02,  // B
    JOY_RIGHT = 0x04, // C
    JOY_NONE = 0
};

bool initJoystick();
uint8_t readJoystick();

#endif