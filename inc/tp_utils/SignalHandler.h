#ifndef tp_utils_SignalHandler_h
#define tp_utils_SignalHandler_h

#include "tp_utils/Globals.h"

namespace tp_utils
{

//##################################################################################################
//! Handles Posix signals.
class SignalHandler
{
public:
  //################################################################################################
  SignalHandler(bool exitOnInt=false);

  //################################################################################################
  ~SignalHandler();

  //################################################################################################
  void waitCtrlC();

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
