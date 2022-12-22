#ifndef __GRID_H
#define __GRID_H

#include <stdint.h>
#include <set>

typedef struct
{
    int x;
    int y;
} pos_t;

typedef std::set<uint32_t> peers_t;

class CGrid
{
public:
    CGrid(uint16_t cols, uint16_t rows);
    ~CGrid();

    uint8_t &at(uint16_t x, uint16_t y);
    void clear();
    void random(uint8_t maxvals);
    void clearRow(uint16_t y);
    void removeRow(uint16_t row);
    bool isValidPos(int16_t x, int16_t y);
    void findPeers(int16_t x, int16_t y, peers_t &peers);
    static uint32_t toKey(int16_t x, int16_t y);
    static pos_t toPos(uint32_t key);

private:
    uint16_t m_cols;
    uint16_t m_rows;
    uint8_t *m_grid;
};

#endif