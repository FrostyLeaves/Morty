#pragma once


#include <stdint.h>

//define MORTY_WIN
//define MORTY_MACOS
//#define MORTY_ANDROID
//#define MORTY_IOS

#ifdef MORTY_WIN
#ifdef MORTY_EXPORTS
#define MORTY_API
#else
#define MORTY_API
#endif
#else
#define MORTY_API
#endif

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define ROW_MAJOR    1
#define COL_MAJOR    2

//#define MATRIX_MAJOR ROW_MAJOR
#define MATRIX_MAJOR COL_MAJOR

//#pragma warning( disable: 4251 )
//#pragma warning( disable: 4275 )

#include <cfloat>
#include <cmath>

#include <algorithm>
#include <array>
#include <assert.h>
#include <atomic>
#include <chrono>
#include <climits>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <stack>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <flatbuffers/buffer.h>
#include <flatbuffers/flatbuffer_builder.h>

#include "Utility/MSpan.h"
#include "Utility/MString.h"
#include "doctest/doctest.h"

using MObjectID   = unsigned long;
using MResourceID = unsigned long;
using MByte       = unsigned char;

template<class... T> void MORTY_UNUSED(T&&...) {}

#if MORTY_DEBUG
#define MORTY_ASSERT(condition) assert(condition)
#else
#define MORTY_ASSERT MORTY_UNUSED
#endif

template<class T> void MORTY_SAFE_DELETE(T& pointer)
{
    if (pointer)
    {
        delete pointer;
        pointer = nullptr;
    }
}

namespace morty
{
class MORTY_API MGlobal
{
public:
    static constexpr int      M_INVALID_INDEX  = -1;
    static constexpr uint32_t M_INVALID_UINDEX = static_cast<uint32_t>(M_INVALID_INDEX);
    static constexpr float    M_FLOAT_BIAS     = 1e-6;
    static constexpr size_t   M_MAX_THREAD_NUM = 10;
};

class MEngine;
}// namespace morty
