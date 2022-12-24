/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_freertos_hooks.h"

#include "disp_spi.h"
#include "ili9488.h"
#include "esphelpers.h"
#include "tileset.h"
#include "grid.h"
#include "shape.h"

#include <vector>

// #include "tp_spi.h"
//  #include "xpt2046.h"

#define TILE_BLANK 0
#define INVALID -1

#define JOYSTICK_A_BUTTON GPIO_NUM_16
#define JOYSTICK_B_BUTTON GPIO_NUM_17
#define JOYSTICK_C_BUTTON GPIO_NUM_21
#define JOYSTICK_D_BUTTON GPIO_NUM_19 // gray

#define MASK_A 1
#define MASK_B 2
#define MASK_C 4
#define MASK_D 8

static void IRAM_ATTR lv_tick_task(void);

// https://github.com/anuprao/esp32_ili9488/blob/master/main/main.c

const color18_t BLACK = {0, 0, 0};
const color18_t RED = {0xff, 0, 0};
const color18_t GREEN = {0, 0xff, 0};
const color18_t BLUE = {0, 0, 0xff};
const color18_t CYAN = {0, 0xff, 0xff};
const color18_t YELLOW = {0xff, 0xff, 0};
const color18_t PURPLE = {0xff, 0, 0xff};
const color18_t GREY = {0x80, 0x80, 0x80};
const color18_t PINK = {255, 192, 203};

const uint8_t tileWidth = 16;
CTileSet tiles(tileWidth, tileWidth);
const uint16_t width = 320;
const uint16_t height = 480;
const uint16_t gridSize = 2 * tileWidth;

const uint8_t cols = width / gridSize;
const uint8_t rows = height / gridSize;
CGrid grid(cols, rows);
static const char *TAG = "main";

uint64_t ticks = 0;

void setupButtons()
{
	esp_err_t ret;
	ret = gpio_set_direction(JOYSTICK_A_BUTTON, GPIO_MODE_INPUT);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "[A] gpio_set_direction Failed (%s)", esp_err_to_name(ret));
	}

	ret = gpio_set_direction(JOYSTICK_B_BUTTON, GPIO_MODE_INPUT);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "[B] gpio_set_direction Failed (%s)", esp_err_to_name(ret));
	}

	ret = gpio_set_direction(JOYSTICK_C_BUTTON, GPIO_MODE_INPUT);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "[C] gpio_set_direction Failed (%s)", esp_err_to_name(ret));
	}

	ret = gpio_set_direction(JOYSTICK_D_BUTTON, GPIO_MODE_INPUT);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "[C] gpio_set_direction Failed (%s)", esp_err_to_name(ret));
	}

	gpio_set_pull_mode(JOYSTICK_A_BUTTON, GPIO_PULLUP_PULLDOWN);
	gpio_set_pull_mode(JOYSTICK_B_BUTTON, GPIO_PULLUP_PULLDOWN);
	gpio_set_pull_mode(JOYSTICK_C_BUTTON, GPIO_PULLUP_PULLDOWN);
	gpio_set_pull_mode(JOYSTICK_D_BUTTON, GPIO_PULLUP_PULLDOWN);
}

uint8_t readButtons()
{
	int a_button = gpio_get_level(JOYSTICK_A_BUTTON);
	int b_button = gpio_get_level(JOYSTICK_B_BUTTON);
	int c_button = gpio_get_level(JOYSTICK_C_BUTTON);
	int d_button = gpio_get_level(JOYSTICK_D_BUTTON);

	printf("Buttons: %.2x %.2x %.2x %.2x\n", a_button, b_button, c_button, d_button);
	return a_button |
		   (b_button << 1) |
		   (c_button << 2) |
		   (d_button << 3);
}

void drawTile(int x, int y, uint8_t tile)
{
	ili9488_drawTile(x * gridSize, y * gridSize, tiles[tile]);
}

void drawGrid()
{
	for (int y = 0; y < rows; ++y)
	{
		for (int x = 0; x < cols; ++x)
		{
			drawTile(x, y, grid.at(x, y));
		}
	}
}

void loadTitles()
{
	tiles.read("/spiffs/blocks.mcz");
}

void clear(color18_t color)
{
	ili9488_fill(0, 0, 320, 480, color);
}

void test()
{
	clear(BLACK);
	int i = 0;
	while (1)
	{
		grid.random(tiles.size());
		drawGrid();
		//	clear(BLACK);
		///		ili9488_drawTile(128, 128, tiles[i]);
		ili9488_fill(0, 0, 64, 64, RED);
		vTaskDelay(150 / portTICK_PERIOD_MS);
		ili9488_fill(0, 0, 64, 64, GREEN);
		vTaskDelay(150 / portTICK_PERIOD_MS);
		ili9488_fill(0, 0, 64, 64, BLUE);
		vTaskDelay(150 / portTICK_PERIOD_MS);

		//	lv_task_handler();
		i++;
		if (i >= tiles.size())
		{
			i = 0;
		}
	}
}

void drawShape(CShape &shape, bool erase = false)
{
	uint8_t x = shape.x();
	for (int8_t i = 0; i < CShape::height(); ++i)
	{
		int8_t y = shape.y() + i;
		if (y < 0)
		{
			continue;
		}
		uint8_t tile = erase ? TILE_BLANK : shape.tile(i);
		drawTile(x, y, tile);
		grid.at(x, y) = tile;
	}
}

bool canMoveShape(CShape &shape, int aim)
{
	int x = shape.x();
	int y = shape.y();
	int i;

	switch (aim)
	{
	case CShape::DOWN:
		y += shape.height();
		return y < rows && grid.at(x, y) == TILE_BLANK;
	case CShape::LEFT:
		--x;
		for (i = 0; i < shape.height(); ++i)
		{
			if (x < 0)
			{
				return false;
			}
			if (i + y < 0)
			{
				continue;
			}
			if (grid.at(x, i + y) != TILE_BLANK)
			{
				return false;
			};
		}
		return true;
	case CShape::RIGHT:
		++x;
		for (i = 0; i < shape.height(); ++i)
		{
			if (x >= cols)
			{
				return false;
			}
			if (i + y < 0)
			{
				continue;
			}
			if (grid.at(x, i + y) != TILE_BLANK)
			{
				return false;
			};
		}
		return true;
	}
	return false;
}

void collapseCol(int16_t x)
{
	int16_t dy = INVALID;
	for (int16_t y = rows - 1; y >= 0; --y)
	{
		if ((dy == INVALID) && grid.at(x, y) == TILE_BLANK)
		{
			dy = y;
			continue;
		}

		if (grid.at(x, y) != TILE_BLANK && dy != INVALID)
		{
			grid.at(x, dy) = grid.at(x, y);
			drawTile(x, dy, grid.at(x, y));
			--dy;
			grid.at(x, y) = TILE_BLANK;
			drawTile(x, y, TILE_BLANK);
		}
	}
}

void removePeers(peers_t &peers)
{
	std::set<int16_t> chCols;
	for (std::set<uint32_t>::iterator it = peers.begin(); it != peers.end(); ++it)
	{
		uint32_t key = *it;
		pos_t p = CGrid::toPos(key);
		grid.at(p.x, p.y) = TILE_BLANK;
		drawTile(p.x, p.y, TILE_BLANK);
		if (chCols.count(p.x) == 0)
		{
			chCols.insert(p.x);
		}
	}

	for (std::set<int16_t>::iterator it = chCols.begin(); it != chCols.end(); ++it)
	{
		int16_t x = *it;
		collapseCol(x);
	}
}

void findPeers(CShape &shape)
{
	uint8_t x = shape.x();
	std::vector<pos_t> blocks;
	for (int i = 0; i < shape.height(); ++i)
	{
		uint8_t y = i + shape.y();
		if (grid.isValidPos(x, y) && grid.at(x, y))
		{
			pos_t block = {x, y};
			blocks.push_back(block);
		}
	}

	for (auto p = blocks.begin(); p != blocks.end(); ++p)
	{
		peers_t peers;
		pos_t pos = *p;
		grid.findPeers(pos.x, pos.y, peers);
		//	printf(">>> found:%d\n", peers.size());
		if (peers.size() >= 3)
		{
			removePeers(peers);
		}
	}
}

extern "C" void app_main(void)
{
	const int8_t orgY = -2;

	disp_spi_init();
	ili9488_init();
	setupButtons();
	initSpiffs();
	loadTitles();

	// tp_spi_init();
	// xpt2046_init();

	esp_register_freertos_tick_hook(lv_tick_task);

	clear(BLACK);
	CShape shape(random() % cols, orgY);
	grid.clear();
	drawShape(shape);

	uint32_t cycles = 0;
	while (1)
	{
		vTaskDelay(10 / portTICK_PERIOD_MS);

		if ((cycles & 15) == 0)
		{
			uint8_t buttons = readButtons();

			if (buttons & MASK_A)
			{
				shape.shift();
				drawShape(shape);
			}

			if (buttons & MASK_B)
			{
				if (canMoveShape(shape, CShape::LEFT))
				{
					drawShape(shape, true);
					shape.move(CShape::LEFT);
					drawShape(shape);
				}
			}

			else if (buttons & MASK_C)
			{
				if (canMoveShape(shape, CShape::RIGHT))
				{
					drawShape(shape, true);
					shape.move(CShape::RIGHT);
					drawShape(shape);
				}
			}
		}
		if ((cycles & 31) == 0)
		{

			if (canMoveShape(shape, CShape::DOWN))
			{
				drawShape(shape, true);
				shape.move(CShape::DOWN);
			}
			else
			{
				if (shape.y() <= 0)
				{
					grid.clear();
					clear(BLACK);
				}
				else
				{
					findPeers(shape);
				}
				shape.newShape(random() % cols, orgY);
			}
			drawShape(shape);
		}
		++cycles;
	}

	// All done, unmount partition and disable SPIFFS
	esp_vfs_spiffs_unregister(NULL);
}

static void IRAM_ATTR lv_tick_task(void)
{
	++ticks;
}
