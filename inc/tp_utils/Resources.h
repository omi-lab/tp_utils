#ifndef tp_utils_Resources_h
#define tp_utils_Resources_h

#include "tp_utils/Globals.h"

#include <unordered_map>
#include <sstream>

namespace tp_utils
{

//##################################################################################################
struct Resource
{
  const char* data{nullptr};
  size_t size{0};
};

//##################################################################################################
struct ResourceStream :public std::istringstream
{
  //################################################################################################
  ResourceStream(const Resource& resource);
};

//##################################################################################################
std::unordered_map<std::string, Resource>& resources();

//##################################################################################################
Resource resource(const std::string& name);

//##################################################################################################
void addResource(const std::string& name,  const char* data, size_t size);

}

#endif
