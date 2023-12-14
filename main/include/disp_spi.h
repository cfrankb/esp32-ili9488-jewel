/**
 * @file disp_spi.h
 *
 */

#ifndef DISP_SPI_H
#define DISP_SPI_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>

    /*********************
     *      DEFINES
     *********************/

#define DISP_SPI_MOSI static_cast<gpio_num_t>(CONFIG_MOSI_GPIO) //  GPIO_NUM_23 // 13
#define DISP_SPI_CLK static_cast<gpio_num_t>(CONFIG_SCLK_GPIO)  // GPIO_NUM_18  // 14
#define DISP_SPI_CS static_cast<gpio_num_t>(CONFIG_CS_GPIO)     // GPIO_NUM_14   // 5

    /**********************
     *      TYPEDEFS
     **********************/

    /**********************
     * GLOBAL PROTOTYPES
     **********************/
    void disp_spi_init(void);
    void disp_spi_send_data(uint8_t *data, uint16_t length);
    void disp_spi_send_colors(uint8_t *data, uint16_t length);

    /**********************
     *      MACROS
     **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DISP_SPI_H*/
