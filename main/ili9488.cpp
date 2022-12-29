/**
 * @file ili9488.cpp
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "ili9488.h"
#include "disp_spi.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tileset.h"

#include <cstring>
#include <algorithm>

/*********************
 *      DEFINES
 *********************/

#define LV_HOR_RES 320
/**********************
 *      TYPEDEFS
 **********************/

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct
{
	uint8_t cmd;
	uint8_t data[16];
	uint8_t databytes; // No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ili9488_send_cmd(uint8_t cmd);
static void ili9488_send_data(void *data, uint16_t length);
static void ili9488_send_color(void *data, uint16_t length);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

const uint16_t dmaMax = 2048;

typedef struct
{
	int32_t x1;
	int32_t x2;
	int32_t y1;
	int32_t y2;
} rect_t;

void ili9488_init(void)
{
	lcd_init_cmd_t ili_init_cmds[] = {

		// CABC Control 9
		//{0xCF, {0x00, 0x83, 0X30}, 3},

		//{0xED, {0x64, 0x03, 0X12, 0X81}, 4},

		//{0xE8, {0x85, 0x01, 0x79}, 3},

		// CABC Control 5
		//{0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},

		// Adjust Control 3
		//{0xF7, {0x20}, 1},

		//{0xEA, {0x00, 0x00}, 2},

		// Positive Gamma Control
		//{0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
		{0xE0, {0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F}, 15},

		// Negative Gamma Control
		//{0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
		{0XE1, {0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45, 0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F}, 15},

		// Power Control 1
		{0xC0, {0x17, 0x15}, 2},

		// Power Control 2
		{0xC1, {0x41}, 1},

		// VCOM Control
		{0xC5, {0x00, 0x12, 0x80}, 3},

		// VCOM Control
		//{0xC7, {0xBE}, 1},

		// Memory Access Control
		//{0x36, {0x28}, 1},
		//{0x36, {0x48}, 1},
		{0x36, {0x88}, 1},

		// Pixel Interface Format
		// Original code for ILI9341
		//{0x3A, {0x55}, 1},
		{0x3A, {0x66}, 1},

		// Interface Mode Control
		{0xB0, {0x00}, 1},

		// Frame Rate Control
		{0xB1, {0xA0}, 1},

		//??
		//{0xF2, {0x08}, 1},

		//??
		//{0x26, {0x01}, 1},

		// Display Inversion Control
		{0xB4, {0x02}, 1},

		// Display Function Control
		// Original code for ILI9341
		//{0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
		{0xB6, {0x02, 0x02, 0x3B}, 3},

		// Entry Mode Set
		{0xB7, {0xC6}, 1},

		// Adjust Control 3
		{0xF7, {0xA9, 0x51, 0x2C, 0x82}, 4},

		// Set_column_address 4 parameters
		//{0x2A, {0x00, 0x00, 0x00, 0xEF}, 4}, // 0xEF = 240

		// Set_page_address 4 parameters
		//{0x2B, {0x00, 0x00, 0x01, 0x3f}, 4}, // 0x013F = 320

		// Write_memory_start
		// ??
		//{0x2C, {0}, 0},

		// Exit Sleep
		{0x11, {0}, 0x80},

		// Display on
		{0x29, {0}, 0x80},

		{0, {0}, 0xff},
	};

	// Initialize non-SPI GPIOs
	gpio_set_direction(ILI9488_DC, GPIO_MODE_OUTPUT);
	gpio_set_direction(ILI9488_RST, GPIO_MODE_OUTPUT);
	gpio_set_direction(ILI9488_BCKL, GPIO_MODE_OUTPUT);

	// Reset the display
	gpio_set_level(ILI9488_RST, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS); // portTICK_RATE_MS);
	gpio_set_level(ILI9488_RST, 1);
	vTaskDelay(100 / portTICK_PERIOD_MS); // portTICK_RATE_MS);

	printf("ILI9488 initialization.\n");

	// Send all the commands
	uint16_t cmd = 0;
	while (ili_init_cmds[cmd].databytes != 0xff)
	{
		ili9488_send_cmd(ili_init_cmds[cmd].cmd);
		ili9488_send_data(ili_init_cmds[cmd].data, ili_init_cmds[cmd].databytes & 0x1F);
		if (ili_init_cmds[cmd].databytes & 0x80)
		{
			vTaskDelay(100 / portTICK_PERIOD_MS); // portTICK_RATE_MS);
		}
		cmd++;
	}

	/// Enable backlight
	printf("Enable backlight.\n");
	gpio_set_level(ILI9488_BCKL, 1);
}

void initWriteWindow(const rect_t &rect)
{
	uint8_t data[4];

	/*Column addresses*/
	ili9488_send_cmd(0x2A);
	data[0] = (rect.x1 >> 8) & 0xFF;
	data[1] = rect.x1 & 0xFF;
	data[2] = (rect.x2 >> 8) & 0xFF;
	data[3] = rect.x2 & 0xFF;
	ili9488_send_data(data, 4);

	/*Page addresses*/
	ili9488_send_cmd(0x2B);
	data[0] = (rect.y1 >> 8) & 0xFF;
	data[1] = rect.y1 & 0xFF;
	data[2] = (rect.y2 >> 8) & 0xFF;
	data[3] = rect.y2 & 0xFF;
	ili9488_send_data(data, 4);

	/*Memory write*/
	ili9488_send_cmd(0x2C);
}

void commitWrite(uint8_t *buf, uint16_t bytesLeft)
{
	uint8_t *ptr = buf;
	while (bytesLeft)
	{
		uint16_t size = std::min(bytesLeft, dmaMax);
		ili9488_send_color(ptr, size);
		ptr += size;
		bytesLeft -= size;
	}

	/*Send the remaining data*/
	ili9488_send_color(buf, 1);
}

void ili9488_drawTile(int32_t x1, int32_t y1, void *tile)
{
	const uint8_t h = 16;
	const uint8_t w = 16;

	int32_t x2 = x1 + w * 2 - 1;
	int32_t y2 = y1 + h * 2;

	const uint16_t bufSize = w * h * sizeof(color18_t) * 4;
	uint16_t bytesLeft = bufSize;
	static uint8_t buf[bufSize];

	color18_t *s = reinterpret_cast<color18_t *>(tile);
	color18_t *d = reinterpret_cast<color18_t *>(buf);

	for (int y = 0; y < h; ++y)
	{
		for (int i = 0; i < 2; ++i)
		{
			for (int x = 0; x < w; ++x)
			{
				d[x * 2] = s[x];
				d[x * 2 + 1] = s[x];
			}
			d += w * 2;
		}
		s += w;
	}

	rect_t rect = {x1, x2, y1, y2};
	initWriteWindow(rect);
	commitWrite(buf, bytesLeft);
}

// Used in unbuffered mode
void ili9488_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color18_t color)
{
	rect_t rect = {x1, x2, y1, y2};
	initWriteWindow(rect);

	uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 0);
	static color18_t buf[LV_HOR_RES];

	uint32_t i;
	int fillSize = size > LV_HOR_RES ? LV_HOR_RES : size;
	for (i = 0; i < fillSize; i++)
	{
		buf[i] = color;
	}

	while (size > LV_HOR_RES)
	{
		ili9488_send_color(buf, LV_HOR_RES * sizeof(color18_t));
		size -= LV_HOR_RES;
	}

	/*Send the remaining data*/
	ili9488_send_color(buf, size * sizeof(color18_t));
}

void ili9488_drawFont(
	int32_t x1,
	int32_t y1,
	const uint8_t *fontBits,
	const color18_t color,
	const color18_t bkcolor)
{
	const uint8_t h = 8;
	const uint8_t w = 8;

	int32_t x2 = x1 + w * 2 - 1;
	int32_t y2 = y1 + h * 2;

	const uint16_t bufSize = w * h * sizeof(color18_t) * 4;
	uint16_t bytesLeft = bufSize;
	static uint8_t buf[bufSize];
	color18_t *d = reinterpret_cast<color18_t *>(buf);

	for (int y = 0; y < h; ++y)
	{
		for (int i = 0; i < 2; ++i)
		{
			uint8_t bits = fontBits[y];
			for (int x = 0; x < w; ++x)
			{
				d[x * 2] = (bits & 1) ? color : bkcolor;
				d[x * 2 + 1] = (bits & 1) ? color : bkcolor;
				bits = bits >> 1;
			}
			d += w * 2;
		}
	}

	rect_t rect = {x1, x2, y1, y2};
	initWriteWindow(rect);
	commitWrite(buf, bytesLeft);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void ili9488_send_cmd(uint8_t cmd)
{
	gpio_set_level(ILI9488_DC, 0); /*Command mode*/
	disp_spi_send_data(&cmd, 1);
}

static void ili9488_send_data(void *data, uint16_t length)
{
	gpio_set_level(ILI9488_DC, 1); /*Data mode*/
	disp_spi_send_data(reinterpret_cast<uint8_t *>(data), length);
}

static void ili9488_send_color(void *data, uint16_t length)
{
	gpio_set_level(ILI9488_DC, 1); /*Data mode*/
	disp_spi_send_colors(reinterpret_cast<uint8_t *>(data), length);
}
