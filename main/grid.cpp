#include <string>
#include <cstring>
#include "grid.h"

CGrid::CGrid(uint16_t cols, uint16_t rows)
{
    m_rows = rows;
    m_cols = cols;
    m_grid = new uint8_t[cols * rows];
}

uint8_t &CGrid::at(uint16_t x, uint16_t y)
{
    return m_grid[x + y * m_cols];
}

void CGrid::clear()
{
    memset(m_grid, 0, m_cols * m_rows);
}

void CGrid::random(uint8_t maxvals)
{
    for (int i = 0; i < m_cols * m_rows; ++i)
    {
        m_grid[i] = ::random() % maxvals;
    }
}

void CGrid::clearRow(uint16_t y)
{
    for (int x = 0; x < m_cols; ++x)
    {
        at(x, y) = 0;
    }
}

void CGrid::removeRow(uint16_t row)
{
    for (int y = row; y > 0; --y)
    {
        // copy row on above current
        memcpy(m_grid + y * m_cols,
               m_grid + (y - 1) * m_cols,
               m_cols);
    }
    clearRow(0);
}

CGrid::~CGrid()
{
    if (m_grid)
    {
        delete[] m_grid;
    }
}

bool CGrid::isValidPos(int16_t x, int16_t y)
{
    return (x >= 0) && (x < m_cols) && (y >= 0) && (y < m_rows);
}

uint32_t CGrid::toKey(int16_t x, int16_t y)
{
    return x + (y << 16);
}

pos_t CGrid::toPos(uint32_t key)
{
    pos_t p = {
        static_cast<int>(key & 0xffff),
        static_cast<int>(key >> 16),
    };
    return p;
}

void CGrid::findPeers(int16_t x, int16_t y, peers_t &peers)
{
    if (!isValidPos(x, y))
    {
        return;
    }

    uint8_t needle = at(x, y);
    if (!needle) // if blank
    {
        return;
    }

    uint32_t key = toKey(x, y);
    if (peers.count(key) != 0) // already in list
    {
        return;
    }
    // insert peer into set
    peers.insert(key);

    pos_t pos[] = {
        {x - 1, y},
        {x + 1, y},
        {x, y - 1},
        {x, y + 1},
    };

    for (uint16_t i = 0; i < 4; ++i)
    {
        pos_t &p = pos[i];
        if (isValidPos(p.x, p.y))
        {
            // check if tile matches the needle
            if (at(p.x, p.y) != needle)
            {
                continue;
            }
            uint32_t key = toKey(p.x, p.y);
            // only check peers not already in set
            if (peers.count(key) == 0)
            {
                findPeers(p.x, p.y, peers);
            }
        }
    }
}
