/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <cstring>
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
#include "font.h"

// #include "tp_spi.h"
// #include "xpt2046.h"

#define TILE_BLANK 0
#define INVALID -1

#define JOYSTICK_A_BUTTON GPIO_NUM_16 // orange
#define JOYSTICK_B_BUTTON GPIO_NUM_17 // blue
#define JOYSTICK_C_BUTTON GPIO_NUM_21 // yellow
#define JOYSTICK_D_BUTTON GPIO_NUM_19 // gray

#define MASK_A 1
#define MASK_B 2
#define MASK_C 4
#define MASK_D 8

// https://github.com/anuprao/esp32_ili9488/blob/master/main/main.c

const color18_t BLACK = {0, 0, 0};
const color18_t WHITE = {255, 255, 255};
const color18_t RED = {0xff, 0, 0};
const color18_t GREEN = {0, 0xff, 0};
const color18_t BLUE = {0, 0, 0xff};
const color18_t DARK_BLUE = {0, 0, 0x20};
const color18_t MEDIUM_BLUE = {0, 0, 0x40};
const color18_t CYAN = {0, 0xff, 0xff};
const color18_t YELLOW = {0xff, 0xff, 0};
const color18_t PURPLE = {0xff, 0, 0xff};
const color18_t MEDIUM_GREY = {0x40, 0x40, 0x40};
const color18_t DARK_GREY = {0x20, 0x20, 0x20};
const color18_t LIGHT_GREY = {0x80, 0x80, 0x80};
const color18_t PINK = {255, 192, 203};

const uint8_t tileWidth = 16;
CTileSet tiles(tileWidth, tileWidth);
const uint16_t width = 320;
const uint16_t height = 480;
const uint16_t gridSize = 2 * tileWidth;

const uint8_t cols = width / gridSize;
const uint8_t rows = height / gridSize;
CGrid grid(cols, rows);
CFont font(CFont::shift8bytes);
static const char *TAG = "main";

const uint16_t blocksPerLevel = 50;
const uint16_t levelBonus = 25;
const uint16_t speedOffset = 2;

uint64_t ticks = 0;
uint16_t gameSpeed;
uint32_t score;
uint16_t level;
uint16_t blockCount;
uint16_t totalBlocks;
int blockRange;

// extern "C" static void IRAM_ATTR lv_tick_task(void);
static void IRAM_ATTR lv_tick_task(void)
{
	++ticks;
}

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
	// printf("Buttons: %.2x %.2x %.2x %.2x\n", a_button, b_button, c_button, d_button);
	return a_button |
		   (b_button << 1) |
		   (c_button << 2) |
		   (d_button << 3);
}

void drawString(int x, int y, const char *s, color18_t color = {255, 255, 255}, color18_t bkColor = {0, 0, 0})
{
	for (int i = 0; s[i]; ++i)
	{
		int j = s[i] - 32;
		ili9488_drawFont(x + i * 16, y, font[j], color, bkColor);
	}
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

void loadTiles()
{
	tiles.read("/spiffs/blocks.mcz");
}

void clear(color18_t color)
{
	ili9488_fill(0, 0, CONFIG_WIDTH, CONFIG_HEIGHT, color);
}

void test()
{
	clear(BLACK);
	int i = 0;
	while (1)
	{
		grid.random(tiles.size());
		drawGrid();
		ili9488_fill(0, 0, 64, 64, RED);
		vTaskDelay(150 / portTICK_PERIOD_MS);
		ili9488_fill(0, 0, 64, 64, GREEN);
		vTaskDelay(150 / portTICK_PERIOD_MS);
		ili9488_fill(0, 0, 64, 64, BLUE);
		vTaskDelay(150 / portTICK_PERIOD_MS);
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
		if (y < 1)
		{
			continue;
		}
		uint8_t tile = erase ? TILE_BLANK : shape.tile(i);
		drawTile(x, y, tile);
		grid.at(x, y) = tile;
	}
}

void eraseShape(CShape &shape)
{
	drawShape(shape, true);
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
	for (std::set<uint32_t>::iterator it = peers.begin(); it != peers.end(); ++it)
	{
		uint32_t key = *it;
		pos_t p = CGrid::toPos(key);
		grid.at(p.x, p.y) = TILE_BLANK;
		drawTile(p.x, p.y, TILE_BLANK);
	}
}

void collapseCols(std::set<int16_t> &chCols)
{
	for (std::set<int16_t>::iterator it = chCols.begin(); it != chCols.end(); ++it)
	{
		int16_t x = *it;
		collapseCol(x);
	}
}

void blocksFromShape(CShape &shape, std::vector<pos_t> &blocks)
{
	blocks.clear();
	uint8_t x = shape.x();
	for (int i = 0; i < shape.height(); ++i)
	{
		uint8_t y = i + shape.y();
		if (grid.isValidPos(x, y) && grid.at(x, y))
		{
			pos_t block = {x, y};
			blocks.push_back(block);
		}
	}
}

void blocksFromCols(std::set<int16_t> &chCols, std::vector<pos_t> &blocks)
{
	blocks.clear();
	for (std::set<int16_t>::iterator it = chCols.begin(); it != chCols.end(); ++it)
	{
		int16_t x = *it;
		for (int16_t y = 0; y < rows; ++y)
		{
			if (grid.at(x, y) != TILE_BLANK)
			{
				pos_t block = {x, y};
				blocks.push_back(block);
			}
		}
	}
}

uint16_t managePeers(CShape &shape)
{
	uint16_t removedBlocks = 0;
	std::vector<pos_t> blocks;
	blocksFromShape(shape, blocks);
	std::set<int16_t> chCols;
	peers_t allPeers;
	while (blocks.size() != 0)
	{
		for (auto p = blocks.begin(); p != blocks.end(); ++p)
		{
			peers_t peers;
			pos_t pos = *p;
			grid.findPeers(pos.x, pos.y, peers);
			if (peers.size() >= 3)
			{
				for (std::set<uint32_t>::iterator it = peers.begin(); it != peers.end(); ++it)
				{
					uint32_t key = *it;
					pos_t p = CGrid::toPos(key);
					if (chCols.count(p.x) == 0)
					{
						chCols.insert(p.x);
					}
					if (allPeers.count(key) == 0)
					{
						allPeers.insert(key);
					}
				}
			}
		}
		removePeers(allPeers);
		removedBlocks += allPeers.size();
		vTaskDelay(50 / portTICK_PERIOD_MS);
		collapseCols(chCols);
		vTaskDelay(50 / portTICK_PERIOD_MS);
		blocksFromCols(chCols, blocks);
		allPeers.clear();
		chCols.clear();
	}
	return removedBlocks;
}

void initGame()
{
	gameSpeed = 50;
	score = 0;
	level = 1;
	blockCount = 0;
	totalBlocks = 0;
	blockRange = CShape::DEFAULT_RANGE;
	clear(BLACK);
	grid.clear();
	ili9488_fill(0, 0, 320, 32, MEDIUM_BLUE);
	printf("(*) grid cleared\n");
}

void loadFont()
{
	if (!font.read("/spiffs/bitfont.bin"))
	{
		printf("failed to read font\n");
	}
}

void drawStatus()
{
	char t[16];
	// Score
	drawString(0, 0, "SCORE", CYAN, MEDIUM_BLUE);
	sprintf(t, "%.6ld", score);
	drawString(0, 16, t, CYAN, MEDIUM_BLUE);

	// Level
	drawString(7 * 16, 0, "LEVEL", WHITE, MEDIUM_BLUE);
	sprintf(t, "  %.2d ", level);
	drawString(7 * 16, 16, t, WHITE, MEDIUM_BLUE);

	// Blocks Left
	drawString(14 * 16, 0, "LEFT", PURPLE, MEDIUM_BLUE);
	sprintf(t, " %.2d ", blocksPerLevel - blockCount);
	drawString(14 * 16, 16, t, PURPLE, MEDIUM_BLUE);
}

extern "C" void app_main(void)
{
	const int8_t orgY = -2;
	disp_spi_init();
	ili9488_init();
	setupButtons();
	initSpiffs();
	loadTiles();
	loadFont();
	initGame();

	// tp_spi_init();
	// xpt2046_init();

	esp_register_freertos_tick_hook(lv_tick_task);

	CShape shape(std::rand() % cols, orgY);
	drawShape(shape);
	drawStatus();

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
			else if (buttons & MASK_B)
			{
				if (canMoveShape(shape, CShape::LEFT))
				{
					eraseShape(shape);
					shape.move(CShape::LEFT);
					drawShape(shape);
				}
			}
			else if (buttons & MASK_C)
			{
				if (canMoveShape(shape, CShape::RIGHT))
				{
					eraseShape(shape);
					shape.move(CShape::RIGHT);
					drawShape(shape);
				}
			}
			else if (buttons & MASK_D)
			{
				eraseShape(shape);
				while (canMoveShape(shape, CShape::DOWN))
				{
					shape.move(CShape::DOWN);
				}
				drawShape(shape);
			}
		}
		if ((cycles % gameSpeed) == 0)
		{
			// move shape down
			if (canMoveShape(shape, CShape::DOWN))
			{
				eraseShape(shape);
				shape.move(CShape::DOWN);
			}
			else
			{
				if (shape.y() <= 0)
				{
					initGame();
					drawStatus();
					cycles = 0;
				}
				else
				{
					uint16_t removedBlocks = managePeers(shape);
					score += removedBlocks;
					/*if (removedBlocks)
					{
						printf("blockCount %d + removedBlocks:%u = newTotal %d; score: %lu\n",
							   blockCount, removedBlocks, blockCount + removedBlocks, score);
					}*/
					blockCount += removedBlocks;
					totalBlocks += removedBlocks;
					bool levelChanged = false;
					while (blockCount >= blocksPerLevel)
					{
						blockCount -= blocksPerLevel;
						score += levelBonus;
						gameSpeed -= speedOffset;
						levelChanged = true;
					}
					if (levelChanged)
					{
						level++;
						// printf(">> level %d\n", level);
						if (level % 3 == 0)
						{
							blockRange = std::min(blockRange + 1, tiles.size() - 1);
						}
					}
					if (removedBlocks)
					{
						drawStatus();
					}
					vTaskDelay(levelChanged ? 100 : 50 / portTICK_PERIOD_MS);
				}
				shape.newShape(std::rand() % cols, orgY, blockRange);
			}
			drawShape(shape);
		}
		++cycles;
	}

	// All done, unmount partition and disable SPIFFS
	esp_vfs_spiffs_unregister(NULL);
}
