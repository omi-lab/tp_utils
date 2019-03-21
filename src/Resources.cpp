#include "tp_utils/Resources.h"

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
void addResource(const std::string& name, const char* data, size_t size)
{
  auto& r = resources()[name];
  r.data = data;
  r.size = size;
}

}
