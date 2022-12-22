/**
 * @file lv_templ.h
 *
 */

#ifndef ILI9488_H
#define ILI9488_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*********************
     *      INCLUDES
     *********************/
    // #include "../lvgl/lvgl.h"

    /*********************
     *      DEFINES
     *********************/

#define ILI9488_DC GPIO_NUM_27   // 19
#define ILI9488_RST GPIO_NUM_33  // 18
#define ILI9488_BCKL GPIO_NUM_32 // 23

    /**********************
     *      TYPEDEFS
     **********************/

    /**********************
     * GLOBAL PROTOTYPES
     **********************/

    typedef struct
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } color18_t;

    void ili9488_init(void);
    void ili9488_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color18_t color);
    void ili9488_drawTile(int32_t x1, int32_t y1, void *tile);

    // void ili9488_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_map);

    /**********************
     *      MACROS
     **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*ILI9488_H*/
