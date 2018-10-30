#ifndef tp_utils_StringIDManager_h
#define tp_utils_StringIDManager_h

#include "tp_utils/Globals.h"

namespace tp_utils
{

class TP_UTILS_SHARED_EXPORT StringIDManager
{
public:
  //################################################################################################
  StringIDManager(const std::string& state=std::string());

  //################################################################################################
  virtual ~StringIDManager();

  //################################################################################################
  //! Get the key for keyString
  /*!
  This should return the key for a given keyString, if it can't then it should return 0. Keys can be
  any number greater than 0. You can re-implement this to store keys in your data own store.

  \warning This must be thread safe!

  \param keyString - The string to produce the key for
  \return The key
  */
  virtual int64_t key(const std::string& keyString);

  //################################################################################################
  //! Get the keyString for key
  /*!
  This should return the keyString for a given key, if it can't then it should return an empty
  string. Keys can be any number greater than 0, and key strings can be any string at least one
  character long and containing alphanumeric characters, hyphens, and under scores. You can
  re-implement this to store keys in your own data store.

  \warning This must be thread safe!

  \param  key - The string to produce the key for
  \return The key string for key
  */
  virtual std::string keyString(int64_t key);

  //################################################################################################
  //! Save the state of the manager
  /*!
  If you are saving StringID's to file this can be used to create a dictionary that can be saved
  with the file. This will only work with this class and NOT with derrived classes.

  \warning - This will save the state of the base class only it is not intended to be used in
  derrived classes.

  \return - The state of this manager
  */
  std::string saveState()const;

  //################################################################################################
  std::vector<std::pair<std::string, int64_t>> takeNewSIDs();

  //################################################################################################
  //! Returns the size of the largest sid, for use with squashSIDs
  /*!
  Returns one of the following:
   - idSize=0 1byte used
   - idSize=1 2byte used
   - idSize=2 4byte used
   - idSize=3 8byte used
  */
  int sidSize();

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
