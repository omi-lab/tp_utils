#ifndef tp_utils_Globals_h
#define tp_utils_Globals_h

#include "lib_platform/Warnings.h"
#include "lib_platform/RandomDevice.h"

#include <functional>

#include <string>
#include <vector>
#include <list>
#include <random>
#include <algorithm>
//#include <variant>

#if defined(TP_UTILS_LIBRARY)
#  define TP_UTILS_SHARED_EXPORT TP_EXPORT
#else
#  define TP_UTILS_SHARED_EXPORT TP_IMPORT
#endif

//##################################################################################################
//TP_UNUSED
#define TP_UNUSED(var) (void)(var)

//##################################################################################################
//TP_NONCOPYABLE
#define TP_NONCOPYABLE(T) T(const T&)=delete; T& operator=(const T&)=delete; T(T&&)=delete; T& operator=(T&&)=delete

//##################################################################################################
//TP_NODISCARD
#ifdef TP_WIN32
#  define TP_NODISCARD
#else
#  define TP_NODISCARD __attribute__((warn_unused_result))
#endif

//##################################################################################################
//TP_DEPRECATED
#if TP_CPP_VERSION>=14
#define TP_DEPRECATED(typ, func) [[deprecated]] typ func
#else
#ifdef __GNUC__
#define TP_DEPRECATED(typ, func) typ func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define TP_DEPRECATED(typ, func) typ __declspec(deprecated) func
#else
#define TP_DEPRECATED(typ, func) typ func
#endif
#endif

//##################################################################################################
//TP_SIZEOF
template<int s> struct __TP_SIZEOF;
//! Use this to show the size of objects at compile time, will fail at the line it is used.
#define TP_SIZEOF(o)__TP_SIZEOF<sizeof(o)> __TP_sizeof;

//##################################################################################################
//TP_LINE_STRING
#define TP_STRINGIZE(x) TP_STRINGIZE2(x)
#define TP_STRINGIZE2(x) #x
#define TP_LINE_STRING TP_STRINGIZE(__LINE__)

//##################################################################################################
//TP_CONCAT
#define TP_CONCAT_(s1, s2) s1##s2
#define TP_CONCAT(s1, s2) TP_CONCAT_(s1, s2)

//##################################################################################################
//TP_CLEANUP
//! Create an object on the stack and call cleanup when it falls out of scope.
#define TP_CLEANUP(cleanup) TPCleanUp TP_CONCAT(tpCleanUp, __LINE__)(cleanup); TP_UNUSED(TP_CONCAT(tpCleanUp, __LINE__))

#define TP_DEFINE_FLAGS(C) \
  inline C operator|(C lhs, C rhs) \
{ \
  using T = std::underlying_type_t <C>; \
  return static_cast<C>(static_cast<T>(lhs) | static_cast<T>(rhs)); \
} \
  inline C& operator |= (C& lhs, C rhs) \
{ \
  lhs = lhs | rhs; \
  return lhs; \
} \
  inline bool operator&(C lhs, C rhs) \
{ \
  using T = std::underlying_type_t <C>; \
  return (static_cast<T>(lhs) & static_cast<T>(rhs))!=0; \
} \
struct TP_CONCAT(TP_DEFINE_FLAGS_, __LINE__)

//##################################################################################################
//! Return a const object
template<typename T>
const T& tpConst(const T& o){return o;}
template<typename T>
const T* tpConst(const T* o){return o;}
template<typename T>
const T* tpConst(T* o){return o;}
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
std::string TP_UTILS_SHARED_EXPORT tpFromHEX(const std::string& input);

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT tpToHex(const std::string& input);

//##################################################################################################
//! Returns true if input starts with the string in s
bool TP_UTILS_SHARED_EXPORT tpStartsWith(const std::string& input, const std::string& s);

//##################################################################################################
//! Returns true if input ends with the string in s
bool TP_UTILS_SHARED_EXPORT tpEndsWith(const std::string& input, const std::string& s);

//##################################################################################################
std::string tpToLower(const std::string& str);

//##################################################################################################
std::string tpToUpper(const std::string& str);

//##################################################################################################
//! Returns true if input contains the string in s
bool TP_UTILS_SHARED_EXPORT tpContains(const std::string& input, const std::string& s);

//##################################################################################################
template<class B, class E>
void tpRandomShuffle(B begin, E end)
{
  lib_platform::RandomDevice rd;
  std::mt19937 g(rd());
  std::shuffle(begin, end, g);
}

//##################################################################################################
void TP_UTILS_SHARED_EXPORT* tpVoidLiteral(size_t value);

namespace tp_utils
{

//##################################################################################################
enum class SplitBehavior
{
  KeepEmptyParts,
  SkipEmptyParts
};

//##################################################################################################
enum class CreateFullPath
{
  Yes,
  No
};
}

//##################################################################################################
//! Split a string on a delimiter
void TP_UTILS_SHARED_EXPORT tpSplit(std::vector<std::string>& result,
                                    const std::string& input,
                                    const std::string& del,
                                    tp_utils::SplitBehavior behavior=tp_utils::SplitBehavior::KeepEmptyParts);

//##################################################################################################
//! Split a string on a delimiter
void TP_UTILS_SHARED_EXPORT tpSplit(std::vector<std::string>& result,
                                    const std::string& input,
                                    char del,
                                    tp_utils::SplitBehavior behavior=tp_utils::SplitBehavior::KeepEmptyParts);

//##################################################################################################
//! Remove all instances of a character from a string.
void TP_UTILS_SHARED_EXPORT tpRemoveChar(std::string& s, char c);

//##################################################################################################
template<typename T>
void tpDeleteAll(const T& container)
{
  for(auto i : container)
    delete i;
}

//##################################################################################################
template<typename T>
typename std::list<T>::value_type tpTakeLast(std::list<T>& container)
{
  typename std::list<T>::value_type t;
  std::swap(t, container.back());
  container.pop_back();
  return t;
}

//##################################################################################################
template<typename T>
typename std::vector<T>::value_type tpTakeLast(std::vector<T>& container)
{
  typename std::vector<T>::value_type t;
  std::swap(t, container.back());
  container.pop_back();
  return t;
}

//##################################################################################################
template<typename T>
typename T::value_type tpTakeFirst(T& container)
{
  auto i = container.begin();
  typename T::value_type t = *i;
  container.erase(i);
  return t;
}

//##################################################################################################
template<typename T, typename I>
typename T::value_type tpTakeAt(T& container, I index)
{
  auto i = container.begin() + index;
  typename T::value_type t = *i;
  container.erase(i);
  return t;
}

//##################################################################################################
template<typename T>
bool tpContainsKey(const T& container, const typename T::key_type& key)
{
  return container.count(key)!=0;
}

//##################################################################################################
template<typename T>
bool tpContainsValue(const T& container, const typename T::mapped_type& value)
{
  return (std::find(container.begin(), container.end(), value) != container.end());
}

//##################################################################################################
template<typename T>
bool tpContains(const T& container, const typename T::value_type& value)
{
  return (std::find(container.begin(), container.end(), value) != container.end());
}

//##################################################################################################
template<typename T>
void tpRemoveOne(T& container, const typename T::value_type& value)
{
  auto i = std::find(container.begin(), container.end(), value);
  if(i != container.end())
    container.erase(i);
}

//##################################################################################################
template<typename T, typename I>
void tpRemoveAt(T& container, I index)
{
  container.erase(container.begin() + index);
}

//##################################################################################################
template<typename T>
size_t tpIndexOf(const T& container, const typename T::value_type& value)
{
  return size_t(std::find(container.begin(), container.end(), value) - container.begin());
}

//##################################################################################################
template<typename T, typename I>
void tpInsert(T& container, I index, const typename T::value_type& value)
{
  container.insert(container.begin() + int(index), value);
}

//##################################################################################################
template<typename T, typename I>
void tpReplace(T& container, I index, const typename T::value_type& value)
{
  container.at(index) = value;
}

//##################################################################################################
template<typename T>
typename T::mapped_type tpGetMapValue(const T& map,
                                      const typename T::key_type& key,
                                      const typename T::mapped_type& defaultValue=typename T::mapped_type())
{
  auto i = map.find(key);
  return (i != map.end())?(i->second):defaultValue;
}

//##################################################################################################
template<typename T>
typename T::key_type tpGetMapKey(const T& map,
                                 const typename T::mapped_type& value,
                                 const typename T::key_type& defaultKey=typename T::key_type())
{
  for(const auto& i : map)
    if(i.second == value)
      return i.first;
  return defaultKey;
}

//##################################################################################################
template<typename T>
typename std::vector<typename T::key_type> tpKeys(const T& map)
{
  std::vector<typename T::key_type> keys;
  for(const auto& i : map)
    keys.push_back(i.first);
  return keys;
}

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT tpToUTF8(const std::u16string& source);

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT tpToUTF8(const std::wstring& source);

//##################################################################################################
std::u16string TP_UTILS_SHARED_EXPORT tpFromUTF8(const std::string& source);

//##################################################################################################
std::wstring tpWStringFromUTF8(const std::string& source);

namespace detail
{
//##################################################################################################
template <typename T, std::size_t...Is>
std::array<T, sizeof...(Is)> tpMakeArray(const T& value, std::index_sequence<Is...>)
{
  return {{(static_cast<void>(Is), value)...}};
}
}

//##################################################################################################
template <typename T, std::size_t N>
std::array<T, N> tpMakeArray(const T& value)
{
  return detail::tpMakeArray(value, std::make_index_sequence<N>());
}

////##################################################################################################
//template<typename V, typename T>
//V tpGetVariantValue(const T& variant, const V& defaultValue=V())
//{
//  const V* v = std::get_if<V>(&variant);
//  return v?*v:defaultValue;
//}

//##################################################################################################
template<typename T>
class TPCleanUp
{
  TP_NONCOPYABLE(TPCleanUp);
  T m_cleanup;
public:
  TPCleanUp(const T& cleanup):m_cleanup(cleanup){}
  ~TPCleanUp(){m_cleanup();}
};

//##################################################################################################
//! Classes used throughout the rest of tp Toolkit.
/*!
This module provides a set of general purpose classes that form the core of tp Toolkit.
*/
namespace tp_utils
{

//##################################################################################################
void TP_UTILS_SHARED_EXPORT leftJustified(std::string& text, size_t maxLength, char padding=' ');

//##################################################################################################
void TP_UTILS_SHARED_EXPORT rightJustified(std::string& text, size_t maxLength, char padding=' ');

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT fixedWidthKeepRight(std::string data, size_t len, char pad);

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT fixedWidthKeepLeft(std::string data, size_t len, char pad);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT parseColor(const std::string& color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT parseColorF(const std::string& color, float& r, float& g, float& b, float& a);

}

#endif
