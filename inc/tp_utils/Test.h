#ifndef tp_utils_Test_h
#define tp_utils_Test_h

#include "tp_utils/Globals.h"

#include <iostream>
#include <functional>

namespace tp_utils
{

//##################################################################################################
class TP_UTILS_EXPORT Test
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
        m_failedTests.push_back(name);
        std::cout << failed << name << std::endl;
      }
      else
      {
        std::cout << passed << name << std::endl;
      }
    }
    catch(...)
    {
      m_result = 1;
      m_failedTests.push_back(name);
      std::cout << except << name << std::endl;
    }
  }  

  //################################################################################################
  void printMessage(const std::string& message)
  {
    std::cout << "\033[21;32m" << message << "\033[0m" << std::endl;
  }

  //################################################################################################
  void printWarning(const std::string& message)
  {
    std::cout << "\033[21;33m" << message << "\033[0m" << std::endl;
  }

  //################################################################################################
  void printError(const std::string& message)
  {
    std::cout << "\033[21;31m" << message << "\033[0m" << std::endl;
  }

  //################################################################################################
  int result()
  {
    return m_result;
  }

  //################################################################################################
  const std::vector<std::string>& failedTests()
  {
    return m_failedTests;
  }

private:
  int m_result{0};
  std::vector<std::string> m_failedTests;
};

//##################################################################################################
class TestSection
{
  TP_NONCOPYABLE(TestSection);
  Test& m_test;
  size_t m_failCount;
public:
  //################################################################################################
  TestSection(Test& test, const std::string& name):
    m_test(test),
    m_failCount(test.failedTests().size())
  {
    m_test.printMessage("===================================================================");
    m_test.printMessage("Testing " + name + "...");
  }

  //################################################################################################
  ~TestSection()
  {
    if(m_failCount != m_test.failedTests().size())
      m_test.printError("Failed!");

    m_test.printMessage("===================================================================");
  }
};

}

#endif
