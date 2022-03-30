#include "tp_utils/Resources.h"
#include "tp_utils/FileUtils.h"

namespace tp_utils
{

//##################################################################################################
ResourceStream::ResourceStream(const Resource& resource):
  std::istringstream(std::string(resource.data, resource.size))
{

}

//##################################################################################################
std::unordered_map<std::string, Resource>& resources()
{
  static std::unordered_map<std::string, Resource> resources;
  return resources;
}

//##################################################################################################
Resource resource(const std::string& name)
{
  return tpGetMapValue(resources(), name);
}

//##################################################################################################
std::string resourceString(const std::string& name)
{
  auto r = resource(name);
  return std::string(r.data, r.size);
}
//##################################################################################################
void addResource(const std::string& name, const char* data, size_t size)
{
  auto& r = resources()[name];
  r.data = data;
  r.size = size;
}

//##################################################################################################
void TP_UTILS_SHARED_EXPORT writeResource(const std::string& from, const std::string& to)
{
  auto r = tp_utils::resource(from);
  tp_utils::writeTextFile(to, std::string(r.data, r.size));
};

}
