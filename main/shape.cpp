#include <string>
#include <cstring>
#include "shape.h"

CShape::CShape(int8_t x, int8_t y)
{
    newShape(x, y);
}

CShape::~CShape()
{
}

void CShape::newShape(int8_t x, int8_t y, int range)
{
    range = std::max(range, static_cast<int>(DEFAULT_RANGE));
    m_x = x;
    m_y = y;
    m_tiles[0] = randomTile(range);
    m_tiles[1] = randomTile(range);
    m_tiles[2] = m_tiles[0] != m_tiles[1] ? m_tiles[0] : randomTile(range);
    if ((random() & 3) == 0)
    {
        shift();
    }
}

uint8_t CShape::randomTile(int range)
{
    uint8_t v = ::random() % range;
    return v ? v : 1;
}

int8_t CShape::x()
{
    return m_x;
}

int8_t CShape::y()
{
    return m_y;
}

uint8_t CShape::tile(int i)
{
    return m_tiles[i];
}

void CShape::shift()
{
    uint8_t t = m_tiles[0];
    m_tiles[0] = m_tiles[1];
    m_tiles[1] = m_tiles[2];
    m_tiles[2] = t;
}

uint8_t CShape::height()
{
    return HEIGHT;
}

void CShape::move(int aim)
{
    switch (aim)
    {
    case DOWN:
        ++m_y;
        break;
    case LEFT:
        --m_x;
        break;
    case RIGHT:
        ++m_x;
    }
}

void CShape::moveTo(int8_t x, int8_t y)
{
    m_x = x;
    m_y = x;
}
