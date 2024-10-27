#pragma once

#include <map>
#include <unordered_map>

namespace morty
{

template<typename K, typename V> const V& FIND_OR_DEFAULT(const std::map<K, V>& m, const K& key, const V& defval)
{
    typename std::map<K, V>::const_iterator it = m.find(key);
    if (it == m.end()) { return defval; }
    else { return it->second; }
}

template<typename K, typename V>
const V& FIND_OR_DEFAULT(const std::unordered_map<K, V>& m, const K& key, const V& defval)
{
    typename std::unordered_map<K, V>::const_iterator it = m.find(key);
    if (it == m.end()) { return defval; }
    else { return it->second; }
}

}// namespace morty