#include "tp_utils/StringIDManager.h"
#include "tp_utils/StringID.h"

#include "json.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/endian/arithmetic.hpp>

#include <map>
#include <mutex>

namespace tp_utils
{
//##################################################################################################
struct StringIDManager::Private
{
  std::mutex mutex;

  std::map<std::string, int64_t> keys;
  std::map<int64_t, std::string> stringKeys;

  std::vector<std::pair<std::string, int64_t>> newSIDs;
  bool rememberNew;

  Private(bool rememberNew_):
    rememberNew(rememberNew_)
  {

  }
};

//##################################################################################################
StringIDManager::StringIDManager(const boost::string_ref& state, bool rememberNew):
  d(new Private(rememberNew))
{
  if(!state.empty())
  {
    nlohmann::json j = nlohmann::json::parse(state, nullptr, false);
    for(auto& keyPair : j["pairs"])
    {
      std::string key = keyPair["key"];
      int64_t     id  = keyPair["id"];

      if(key.empty())
        return;

      if(id<1)
        return;

      if(tpConst(d->stringKeys).find(id) == d->stringKeys.cend())
        return;

      d->keys[key]      = id;
      d->stringKeys[id] = key;

      if(d->rememberNew)
        d->newSIDs.emplace_back(key, id);
    }
  }
}

//##################################################################################################
StringIDManager::~StringIDManager()
{
  StringID::managerDestroyed(this);
  delete d;
}

//##################################################################################################
int64_t StringIDManager::key(const std::string& keyString)
{
  if(keyString.empty())
    return 0;

  d->mutex.lock();
  int64_t key = tpGetMapValue(d->keys, keyString, 0ll);

  if(!key)
  {
    key = int64_t(d->keys.size())+1;
    d->keys[keyString] = key;
    d->stringKeys[key] = keyString;
    if(d->rememberNew)
      d->newSIDs.emplace_back(keyString, key);
  }

  d->mutex.unlock();

  return key;
}

//##################################################################################################
std::string StringIDManager::keyString(int64_t key)
{
  d->mutex.lock();
  std::string keyString = tpGetMapValue(d->stringKeys, key, std::string());
  d->mutex.unlock();

  return keyString;
}

//##################################################################################################
std::string StringIDManager::saveState()const
{
  nlohmann::json j;
  auto a = j["pairs"] = nlohmann::json::array();
  for(const auto& keyPair : d->keys)
  {
    a.push_back({
                  {"key", keyPair.first},
                  {"id",  keyPair.second}
                });
  }
  return j;
}

//##################################################################################################
std::vector<std::pair<std::string, int64_t>> StringIDManager::takeNewSIDs()
{
  d->mutex.lock();
  std::vector<std::pair<std::string, int64_t>> newSIDs = std::move(d->newSIDs);
  d->newSIDs.clear();
  d->mutex.unlock();

  return newSIDs;
}

//##################################################################################################
int StringIDManager::sidSize()
{
  size_t c = d->keys.size();
  return (c<0xFF)?0:((c<0xFFFF)?1:2);
}

//##################################################################################################
std::string StringIDManager::squashStrings(const std::vector<boost::string_ref>& strings)
{
  static uint8_t lookup[] = "\x0\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x2\x3\x4\x5\x6\x7\x8\x9\xa\xb\x1\x1\x1\x1\x1\x1\x1\xc\xd\xe\xf\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x1\x1\x1\x1\x1\x1\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1";

  if(strings.empty())
    return std::string();

  //-- Count the bytes -----------------------------------------------------------------------------
  size_t count = strings.size();
  for(const boost::string_ref& string : strings)
    count += string.size();

  //-- Convert the characters and add null separators ----------------------------------------------
  std::string stage1;
  {
    stage1.reserve(count);
    for(const boost::string_ref& string : strings)
    {
      stage1.append(std::string(string));
      stage1.push_back(char(0));
    }

    auto c = reinterpret_cast<uint8_t*>(&stage1[0]);
    auto cMax = c + stage1.size();
    for(; c<cMax; c++)
      (*c) = lookup[(*c)];
  }

  //-- Squash the 6 bit characters -----------------------------------------------------------------
  std::string stage2;
  {
    size_t sz = stage1.size() * 3;
    sz = sz>>2;
    stage2.resize(sz);

    //The index into the source data in bits
    int i=0;
    auto s = reinterpret_cast<const uint8_t*>(stage1.c_str());

    auto c = reinterpret_cast<uint8_t*>(&stage2[0]);
    auto cMax = c + stage2.size();
    for(; c<cMax; c++)
    {
      int idx = i/6;

      uint8_t s1 = s[idx];
      uint8_t s2 = s[idx+1];

      int rem = i%6;
      (*c)  = s1 >> (rem);
      (*c) |= s2 << (6-rem);

      i+=8;
    }
  }

  return stage2;
}

//##################################################################################################
std::vector<std::string> StringIDManager::expandStrings(const boost::string_ref& data, int max, int* offset)
{
  static uint8_t lookup[] = ",\x20\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a                                                                                                                                                                                                ";

  //-- Expand the characters -----------------------------------------------------------------------
  //+2 1 for the null, 1 to round up
  auto c = size_t(float(data.size()+2)*1.3f);
  std::string result;
  result.resize(c);
  {
    uint8_t lastInsert=0;
    int count=0;
    int i=0;

    auto r = reinterpret_cast<uint8_t*>(&result[0]);
    auto rMax = r + result.size();

    auto s = reinterpret_cast<const uint8_t*>(data.data());
    s+=(*offset);
    const uint8_t* sMax = s + data.size();
    while(count<max)
    {
      int idx = i>>3;

      const uint8_t* ss = s+idx;

      //Make sure that we have not run out of data
      if(ss>=sMax)
        break;

      int s1 = *ss;
      int s2 = *(ss+1); //We take advantage of the null on the end of the byte array

      int shift = i&7;

      int v;
      v  = s1>>shift;
      v |= s2<<(8-shift);
      v &= 63;

      //Double null or null first character is a terminator
      if(v==0)
      {
        if(lastInsert==0)
          break;
        count++;
      }
      //result.append(lookup[v]);
      (*r) = lookup[v];
      lastInsert=uint8_t(v);

      r++;
      i+=6;
    }

    for(; r<rMax; r++)
      (*r)=0;

    {
      int idx = i>>3;
      (*offset) = int((reinterpret_cast<const char*>(s)+idx) - data.data());
    }
  }

  //-- Split the results ---------------------------------------------------------------------------
  std::vector<std::string> strings;
  boost::split(strings, result, [](char a){return a==',';}, boost::token_compress_on);
  return strings;
}

//##################################################################################################
std::string StringIDManager::squashSIDs(const std::vector<std::pair<boost::string_ref, int64_t>>& sids, int idSize)
{
  //0,1,2,3
  //1,2,4,8 Bytes
  idSize = tpBound(0, idSize, 3);
  size_t idBytes = (idSize==0)?1:(idSize==1)?2:(idSize==2)?4:8;
  size_t size = sids.size();
  size_t sizeSize = (size<0x10)?0:((size<0x1000)?1:((size<0x100000)?2:3));

  std::string data;

  //-- Add the header ------------------------------------------------------------------------------
  {
    uint32_t header = 0;
    header |= size_t(idSize);
    header |= sizeSize<<2;
    header |= size<<4;
    boost::endian::native_to_big_inplace(header);

    data.append(reinterpret_cast<const char*>(&header), sizeSize+1);
  }

  //-- Add the IDs ---------------------------------------------------------------------------------
  std::vector<boost::string_ref> keyStrings;
  {
    size_t existingSize = data.size();
    data.resize(existingSize+(size*idBytes));

    char* r = (&data[0]) + existingSize;
    for(size_t i=0; i<size; i++)
    {
      const std::pair<boost::string_ref, int64_t>& sid = sids[i];
      keyStrings.push_back(sid.first);
      int64_t key = boost::endian::native_to_little(sid.second);

      memcpy(r, &key, idBytes);
      r+=idBytes;
    }
  }

  //-- Add the strings -----------------------------------------------------------------------------
  data.append(squashStrings(keyStrings));

  return data;
}

//##################################################################################################
std::vector<std::pair<std::string, int64_t>> StringIDManager::expandSIDs(const boost::string_ref& data, int* headerSize)
{
  uint32_t header = 0;
  memcpy(&header, data.data(), tpMin(sizeof(header), data.size()));
  boost::endian::little_to_native_inplace(header);

  //-- Parse the header ----------------------------------------------------------------------------
  //0,1,2,3
  //1,2,4,8 Bytes
  size_t idSize = header & 3;
  size_t idBytes = (idSize==0)?1:(idSize==1)?2:(idSize==2)?4:8;
  size_t sizeSize = (header>>2) & 3;
  size_t size = header>>4;

  size_t mask = (sizeSize==0)?0x00000000F:((sizeSize==1)?0x00000FFF:((sizeSize==2)?0x000FFFFF:0x0FFFFFFF));
  size &= mask;

  size_t requiredSize = (sizeSize+1) + (idBytes*size);
  if(data.size()<requiredSize)
    return std::vector<std::pair<std::string, int64_t>>();

  const char* s = data.data();

  //Skip the header
  s+=sizeSize+1;

  //-- Parse the IDs -------------------------------------------------------------------------------
  std::vector<int64_t> ids;
  for(size_t i=0; i<size; i++)
  {
    int64_t key=0;
    memcpy(&key, s, idBytes);
    ids.push_back(boost::endian::little_to_native(key));
    s+=idBytes;
  }

  //-- Parse the strings ---------------------------------------------------------------------------
  (*headerSize) = int(requiredSize);
  std::vector<std::string> strings = expandStrings(data, int(ids.size()), headerSize);
  if(ids.size() != strings.size())
    return std::vector<std::pair<std::string, int64_t>>();

  std::vector<std::pair<std::string, int64_t>> results;
  results.reserve(ids.size());
  for(size_t i=0; i<ids.size(); i++)
    results.emplace_back(strings.at(i), ids.at(i));

  return results;
}

}
