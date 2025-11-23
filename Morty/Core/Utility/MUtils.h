#pragma once
#include "MGlobal.h"

namespace morty
{

class MORTY_API MUtils
{
public:

    template<class T> static MHashCode Hash(const T& v)
    {
        std::hash<T> h;
        return h(v);
    }

    template<class T> static void HashCombine(MHashCode& s, const T& v)
    {
        std::hash<T> h;
        s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
    }
};

}// namespace morty