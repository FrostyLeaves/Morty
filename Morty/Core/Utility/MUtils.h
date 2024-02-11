#pragma once
#include "MGlobal.h"

MORTY_SPACE_BEGIN

class MORTY_API MUtils
{
public: 
    template <class T>
    static void HashCombine(std::size_t& s, const T& v)
    {
        std::hash<T> h;
        s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
    }
};

MORTY_SPACE_END