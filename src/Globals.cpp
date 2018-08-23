#include "tp_utils/Globals.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/utility/string_view.hpp>

//##################################################################################################
bool tpStartsWith(const std::string& input, const std::string& s)
{
  return input.size() >= s.size() && std::equal(s.begin(), s.end(), input.begin());
}

//##################################################################################################
void tpSplit(std::vector<std::string>& result, const std::string& input, const std::string& del)
{
  boost::iter_split(result, input, boost::algorithm::first_finder(del));
}

//##################################################################################################
void tpRemoveChar(std::string& s, char c)
{
  s.erase(std::remove_if(s.begin(), s.end(), [c](int a){return a==c;}), s.end());
}

namespace tp_utils
{

//##################################################################################################
void leftJustified(std::string& text, int maxLength, char padding)
{
  if(int(text.size()) >= maxLength)
    return;

  text+=std::string(maxLength-text.size(), padding);
}

//##################################################################################################
void rightJustified(std::string& text, int maxLength, char padding)
{
  if(int(text.size()) >= maxLength)
    return;

  text = std::string(maxLength-text.size(), padding)+text;
}


//##################################################################################################
bool parseColor(const std::string& color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a)
{
  r =   0;
  g =   0;
  b =   0;
  a = 255;

  if(color.size() != 7)
    return false;

  if(color.at(0)!='#')
    return false;

  uint32_t acc = 0;
  for(int i=1; i<7; i++)
  {
    char c = color.at(i);

    uint32_t v=0;
    if(c>='0' && c<='9')
      v = c-'0';

    if(c>='A' && c<='F')
      v = 10 + (c-'A');

    if(c>='a' && c<='f')
      v = 10 + (c-'a');

    acc<<=4;
    acc|=v;
  }

  b = acc; acc>>=8;
  g = acc; acc>>=8;
  r = acc;
  return true;
}

//##################################################################################################
bool parseColorF(const std::string& color, float& r, float& g, float& b, float& a)
{
  uint8_t r_;
  uint8_t g_;
  uint8_t b_;
  uint8_t a_;
  bool ret = parseColor(color, r_, g_, b_, a_);
  r = float(r_) / 255.0f;
  g = float(g_) / 255.0f;
  b = float(b_) / 255.0f;
  a = float(a_) / 255.0f;
  return ret;
}

}
