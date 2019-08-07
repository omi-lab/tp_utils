#ifndef tp_utils_Test_h
#define tp_utils_Test_h

#include "tp_utils/Globals.h"

#include <iostream>
#include <functional>

namespace tp_utils
{

//##################################################################################################
class Test
{
public:

  //################################################################################################
  void runTest(const std::string& name, const std::function<bool()>& fn)
  {
    constexpr auto failed = "[\033[21;31mFAIL\033[0m] ";
    constexpr auto passed = "[ \033[21;32mOK\033[0m ] ";
    constexpr auto except = "[\033[21;41mEXCP\033[0m] ";

    try
    {
      if(!fn())
      {
        m_result = 1;
        std::cerr << failed << name << std::endl;
      }
      else
      {
        std::cout << passed << name << std::endl;
      }
    }
    catch(...)
    {
      m_result = 1;
      std::cerr << except << name << std::endl;
    }
  }

  //################################################################################################
  int result()
  {
    return m_result;
  }

private:
  int m_result{0};
};

}

#endif
