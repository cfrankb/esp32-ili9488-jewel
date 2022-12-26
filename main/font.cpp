#include "font.h"
#include <cstring>
#include <cstdio>

CFont::CFont(uint8_t bitShift)
{
    m_font = nullptr;
    m_bitShift = bitShift;
}

CFont::~CFont()
{
    forget();
}

bool CFont::read(const char *fname)
{
    FILE *sfile = fopen(fname, "rb");
    if (sfile)
    {
        forget();
        fseek(sfile, 0, SEEK_END);
        int size = ftell(sfile);
        fseek(sfile, 0, SEEK_SET);
        m_font = new uint8_t[size];
        fread(m_font, size, 1, sfile);
        fclose(sfile);
    }
    return sfile != nullptr;
}

void CFont::forget()
{
    if (m_font)
    {
        delete[] m_font;
    }
}

uint8_t *CFont::operator[](int i)
{
    return m_font + (i << m_bitShift);
}

uint8_t *CFont::get(int i)
{
    return m_font + (i << m_bitShift);
}