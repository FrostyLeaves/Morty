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

#define ROW_MAJOR 1
#define COL_MAJOR 2

//#define MATRIX_MAJOR ROW_MAJOR
#define MATRIX_MAJOR COL_MAJOR

//#pragma warning( disable: 4251 )
//#pragma warning( disable: 4275 )

typedef unsigned long MObjectID;
typedef unsigned long MResourceID;
typedef unsigned char MByte;

#include <cmath>
#include <cfloat>

#include <map>
#include <set>
#include <stack>
#include <queue>
#include <array>
#include <mutex>
#include <memory>
#include <vector>
#include <list>
#include <tuple>
#include <thread>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <condition_variable>

#include <flatbuffers/buffer.h>
#include <flatbuffers/flatbuffer_builder.h>

#include "doctest/doctest.h"
#include "Utility/MString.h"

#if MORTY_DEBUG
#define MORTY_ASSERT assert
#else
#define MORTY_ASSERT
#endif

template<class... T> void MORTY_UNUSED(T&&...) {}

class MORTY_API MGlobal
{
public:
    static constexpr int M_INVALID_INDEX = -1;
    static constexpr uint32_t M_INVALID_UINDEX = static_cast<uint32_t>(M_INVALID_INDEX);
    static constexpr float M_FLOAT_BIAS = 1e-6;

};

class MEngine;
