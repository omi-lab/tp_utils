#ifndef tp_utils_TPPixel_h
#define tp_utils_TPPixel_h

#include "tp_utils/Globals.h"

union TPPixel
{
  TPPixel():
    r(0),
    g(0),
    b(0),
    a(255)
  {

  }

  TPPixel(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_=255):
    r(r_),
    g(g_),
    b(b_),
    a(a_)
  {

  }

  explicit TPPixel(uint32_t value):
    i(value)
  {

  }

  explicit TPPixel(const std::string& color)
  {
    r =   0;
    g =   0;
    b =   0;
    a = 255;

    if(color.size() != 7)
      return;

    if(color.at(0)!='#')
      return;

    uint32_t acc = 0;
    for(size_t i=1; i<7; i++)
    {
      char c = color.at(i);

      uint32_t v=0;
      if(c>='0' && c<='9')
        v = uint32_t(c-'0');

      if(c>='A' && c<='F')
        v = uint32_t(10 + (c-'A'));

      if(c>='a' && c<='f')
        v = uint32_t(10 + (c-'a'));

      acc<<=4;
      acc|=v;
    }

    b = uint8_t(acc); acc>>=8;
    g = uint8_t(acc); acc>>=8;
    r = uint8_t(acc);
  }

  std::string toString()const
  {
    uint32_t acc = 0;
    acc|=r; acc<<=8;
    acc|=g; acc<<=8;
    acc|=b;

    std::string result = "#000000";

    for(size_t i=6; i>0; i--)
    {
      int v = acc&0xF; acc>>=4;
      result[i] = char(v+(v>9?('A'-10):'0'));
    }

    return result;
  }

  uint32_t i;
#ifndef tp_qt_WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
  struct
  {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
  };
#ifndef tp_qt_WIN32
#pragma GCC diagnostic pop
#endif
};

//##################################################################################################
inline bool operator== (const TPPixel& a, const TPPixel& b)
{
  return a.i == b.i;
}

//##################################################################################################
inline bool operator!= (const TPPixel& a, const TPPixel& b)
{
  return a.i != b.i;
}

#endif
