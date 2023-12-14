/**
 * @file ili9488.h
 *
 */

#ifndef ILI9488_H
#define ILI9488_H

#include <stdint.h>

#define ILI9488_DC static_cast<gpio_num_t>(CONFIG_DC_GPIO)     // GPIO_NUM_27 // 19
#define ILI9488_RST static_cast<gpio_num_t>(CONFIG_RESET_GPIO) // GPIO_NUM_33 // 18
#define ILI9488_BCKL static_cast<gpio_num_t>(CONFIG_BL_GPIO)   // GPIO_NUM_32 // 23

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} color18_t;

void ili9488_init(void);
void ili9488_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color18_t color);
void ili9488_drawTile(int32_t x1, int32_t y1, void *tile);
void ili9488_drawFont(int32_t x1, int32_t y1, const uint8_t *fontBits, const color18_t color, const color18_t);
#endif