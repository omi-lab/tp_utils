#include "tp_utils/RefCount.h"

#ifdef TDP_REF_COUNT

#include "tp_utils/StackTrace.h"
#include "tp_utils/DebugUtils.h"

#include "lib_json/Android.hpp"

#include <mutex>

namespace tp_utils
{

namespace
{

//##################################################################################################
class StaticDetails_lt
{
public:
  std::mutex mutex;
  std::unordered_map<tp_utils::StringID, InstanceDetails> instances;

  //################################################################################################
  std::vector<std::string> serialize()
  {
    std::vector<std::string> results;

    mutex.lock();

    std::string title = "Class";
    int maxLength = title.size();
    int maxDigits = 1;

    for(auto i : instances)
    {
      maxLength = tpMax(maxLength, int(i.first.keyString().size()));
      maxDigits = tpMax(maxDigits, int(std::to_string(i.second.total).size()));
    }

    {
      std::string count = std::string("#");
      std::string total = std::string("##");

      leftJustified(title, maxLength);
      rightJustified(count, maxDigits);
      rightJustified(total, maxDigits);

      const std::string line = title + ' ' + count + ' ' + total;
      results.push_back(line);
    }

    for(auto i : instances)
    {
      title = i.first.keyString();
      std::string count = std::to_string(i.second.count);
      std::string total = std::to_string(i.second.total);

      leftJustified(title, maxLength);
      rightJustified(count, maxDigits);
      rightJustified(total, maxDigits);

      const std::string line = title + ' ' + count + ' ' + total;
      results.push_back(line);
    }

    mutex.unlock();

    return results;
  }
};

//##################################################################################################
StaticDetails_lt& staticDetails()
{
  static StaticDetails_lt staticDetails;
  return staticDetails;
}

}

//##################################################################################################
void RefCount::ref(const tp_utils::StringID& type)
{
  StaticDetails_lt& sd(staticDetails());
  sd.mutex.lock();
  InstanceDetails& instanceDetails(sd.instances[type]);
  instanceDetails.count++;
  instanceDetails.total++;
  sd.mutex.unlock();
}

//##################################################################################################
void RefCount::unref(const tp_utils::StringID& type)
{
  StaticDetails_lt& sd(staticDetails());
  sd.mutex.lock();
  InstanceDetails& instanceDetails(sd.instances[type]);
  instanceDetails.count--;
  if(instanceDetails.count<0)
  {
    tpWarning() << "RefCount::unref, error! Type: " << type.keyString().data();
    printStackTrace();
    abort();
  }
  sd.mutex.unlock();
}

//##################################################################################################
void RefCount::lock()
{
  staticDetails().mutex.lock();
}

//##################################################################################################
void RefCount::unlock()
{
  staticDetails().mutex.unlock();
}

//##################################################################################################
const std::unordered_map<tp_utils::StringID, InstanceDetails>& RefCount::instances()
{
  if(staticDetails().mutex.try_lock())
  {
    staticDetails().mutex.unlock();
    tpWarning() << "You must call RefCount::lock() before RefCount::instances()!";
  }

  return staticDetails().instances;
}

//##################################################################################################
std::vector<std::string> RefCount::serialize()
{
  return staticDetails().serialize();
}

}
#endif
