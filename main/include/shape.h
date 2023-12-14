#ifndef __SHAPE_H
#define __SHAPE_H

#include <stdint.h>

class CShape
{

public:
    CShape(int8_t x = 0, int8_t y = 0);
    ~CShape();

    void newShape(int8_t x = 0, int8_t y = 0, int range = -1);
    int8_t x();
    int8_t y();
    uint8_t tile(int i);
    void shift();
    static uint8_t height();
    void move(int aim);
    void moveTo(int8_t x, int8_t y);

    enum
    {
        UP = 0,
        DOWN = 1,
        LEFT = 2,
        RIGHT = 3,
        HEIGHT = 3,
    };

    enum defs : int
    {
        DEFAULT_RANGE = 0xa
    };

private:
    uint8_t
    randomTile(int range);
    int8_t m_x;
    int8_t m_y;
    uint8_t m_tiles[HEIGHT];
};

#endif