#ifndef tp_utils_DebugUtils_h
#define tp_utils_DebugUtils_h

#include "tp_utils/Globals.h"

namespace tp_utils
{

//##################################################################################################
class ExtendArgs
{
public:
  //################################################################################################
  ExtendArgs(int argc, char* argv[])
  {
    for(int i=0; i<argc; i++)
      m_strings.push_back(argv[i]);
  }

  //################################################################################################
  void addArg(const std::string& arg)
  {
    m_strings.push_back(arg);
  }

  //################################################################################################
  int& argc()
  {
    m_argc = int(m_argv.size());
    return m_argc;
  }

  //################################################################################################
  char** argv()
  {
    m_argv.resize(m_strings.size());
    for(size_t i=0; i<m_strings.size(); i++)
      m_argv[i] = &m_strings.at(i)[0];
    return m_argv.data();
  }

private:
  int m_argc{0};
  std::vector<std::string> m_strings;
  std::vector<char*> m_argv;
};

}

#endif
