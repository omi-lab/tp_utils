#ifndef tp_utils_Globals_h
#define tp_utils_Globals_h

#include <unordered_map>

#include <boost/variant.hpp>
#include <boost/core/ignore_unused.hpp>

#include <functional>

#include <string>
#include <vector>

#ifndef TP_UTILS_SHARED_EXPORT
#if defined(TP_UTILS_LIBRARY)
#  define TP_UTILS_SHARED_EXPORT
#else
#  define TP_UTILS_SHARED_EXPORT
#endif
#endif

#ifndef TP_CPP_VERSION
#define TP_CPP_VERSION 17
#endif

#define TP_UNUSED(var) boost::ignore_unused(var)

//##################################################################################################
//TDP_NODISCARD
#define TDP_NODISCARD __attribute__((warn_unused_result))

//##################################################################################################
//TDP_DEPRECATED
#if TDP_CPP_VERSION>=14
#define TDP_DEPRECATED(typ, func) [[deprecated]] typ func
#else
#ifdef __GNUC__
#define TDP_DEPRECATED(typ, func) typ func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define TDP_DEPRECATED(typ, func) typ __declspec(deprecated) func
#else
#define TDP_DEPRECATED(typ, func) typ func
#endif
#endif

//##################################################################################################
//TDP_FALLTHROUGH
#if TDP_CPP_VERSION>=17
#  define TDP_FALLTHROUGH [[fallthrough]]
#elif __GNUC__>=7
#  define TDP_FALLTHROUGH [[gnu::fallthrough]]
#elif defined __CLANG__
#  define TDP_FALLTHROUGH [[clang::fallthrough]]
#else
#  define TDP_FALLTHROUGH
#endif

//##################################################################################################
//TDP_SIZEOF
template<int s> struct __TDP_SIZEOF;
//! Use this to show the size of objects at compile time, will fail at the line it is used.
#define TDP_SIZEOF(o)__TDP_SIZEOF<sizeof(o)> __tdp_sizeof;


//##################################################################################################
//! Return a const object
template<typename T>
const T& tpConst(const T& o){return o;}
template<typename T>
const T& tpConst(const T&& o)=delete;

//##################################################################################################
template<typename T>
T tpBound(T l, T v, T h)
{
  return v<l?l:(v>h?h:v);
}

//##################################################################################################
template<typename T>
T tpMin(T a, T b)
{
  return a<b?a:b;
}

//##################################################################################################
template<typename T>
T tpMax(T a, T b)
{
  return a>b?a:b;
}

//##################################################################################################
//! Returns true if input starts with the string in s
bool tpStartsWith(const std::string& input, const std::string& s);

//##################################################################################################
//! Split a string on a delimiter
void tpSplit(std::vector<std::string>& result, const std::string& input, const std::string& del);

//##################################################################################################
//! Remove all instances of a character from a string.
void tpRemoveChar(std::string& s, char c);

//##################################################################################################
template<typename T>
void tpDeleteAll(T container)
{
  auto const* i = container.data();
  auto const* iMax = i+container.size();
  for(; i<iMax; i++)
    delete i;
}

//##################################################################################################
class TPCleanUp
{
  std::function<void()> m_cleanup;
public:
  TPCleanUp(std::function<void()> cleanup):m_cleanup(cleanup){}
  ~TPCleanUp(){m_cleanup();}
};

//##################################################################################################
//! Classes used throughout the rest of Tdp Toolkit.
/*!
This module provides a set of general purpose classes that form the core of Tdp Toolkit.
*/
namespace tp_utils
{

//##################################################################################################
template<typename V, typename T, typename K>
V getMapValue(const T& map, const K& key, const V& defaultValue=V())
{
  auto i = map.find(key);
  return (i != map.end())?(i->second):defaultValue;
}

//##################################################################################################
template<typename V, typename T>
V getVariantValue(const T& variant, const V& defaultValue=V())
{
  const V* v = boost::get<V>(&variant);
  return v?*v:defaultValue;
}

//##################################################################################################
template<typename V, typename T>
bool contains(const T& container, const V& value)
{
  return (std::find(container.begin(), container.end(), value) != container.end());
}

//##################################################################################################
template<typename V, typename T>
void removeOne(T& container, const V& value)
{
  auto i = std::find(container.begin(), container.end(), value);
  if(i != container.end())
    container.erase(i);
}

//##################################################################################################
template<typename V, typename T>
int indexOf(const T& container, const V& value)
{
  return (std::find(container.begin(), container.end(), value) - container.begin());
}

//##################################################################################################
template<typename V, typename T>
V takeLast(T& container)
{
  auto i = container.begin() + (container.size()-1);
  V t = *i;
  container.erase(i);
  return t;
}

//##################################################################################################
template<typename V, typename T>
V takeFirst(T& container)
{
  auto i = container.begin();
  V t = *i;
  container.erase(i);
  return t;
}

//##################################################################################################
template<typename V, typename T>
V takeAt(T& container, int index)
{
  auto i = container.begin() + index;
  V t = *i;
  container.erase(i);
  return t;
}

//##################################################################################################
void leftJustified(std::string& text, int maxLength, char padding=' ');

//##################################################################################################
void rightJustified(std::string& text, int maxLength, char padding=' ');

}

#endif
