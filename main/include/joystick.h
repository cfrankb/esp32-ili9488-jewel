#ifndef JOYSTICK_H___
#define JOYSTICK_H___
#include <cstdint>

#define JOYSTICK_A_BUTTON GPIO_NUM_16 // orange
#define JOYSTICK_B_BUTTON GPIO_NUM_17 // blue
#define JOYSTICK_C_BUTTON GPIO_NUM_21 // yellow
#define JOYSTICK_D_BUTTON GPIO_NUM_19 // gray

#define MASK_A 1
#define MASK_B 2
#define MASK_C 4
#define MASK_D 8

void initButtons();
uint8_t readButtons();

enum JoyDir
{
    JOY_UP = 0x01,
    JOY_DOWN = 0x02,
    JOY_LEFT = 0x04,
    JOY_RIGHT = 0x08,
    JOY_BUTTON = 0x10,
    JOY_A_BUTTON = 0x20,
    JOY_NONE = 0
};

bool initJoystick();
uint16_t readJoystick();

#endif