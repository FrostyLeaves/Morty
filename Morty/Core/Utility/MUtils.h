#pragma once
#include "MGlobal.h"


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