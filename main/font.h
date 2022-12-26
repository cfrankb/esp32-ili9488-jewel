#ifndef __FONT_H__
#define __FONT_H__
#include <stdint.h>
class CFont
{
public:
    CFont(uint8_t bitShift = shift64bytes);
    ~CFont();

    uint8_t *operator[](int i);
    uint8_t *get(int i);
    bool read(const char *fname);
    void forget();
    enum
    {
        shift8bytes = 3, // 8 bytes
        shift64bytes = 6 // 64 bytes
    };

protected:
    uint8_t *m_font;
    uint8_t m_bitShift;
};

#endif