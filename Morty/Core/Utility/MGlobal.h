#ifndef _M_GLOBAL_H_
#define _M_GLOBAL_H_

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

#pragma warning( disable: 4251 )
#pragma warning( disable: 4275 )

typedef unsigned long MObjectID;
typedef unsigned long MResourceID;
typedef unsigned char MByte;
typedef size_t MEntityID;


#include <map>
#include <set>
#include <stack>
#include <queue>
#include <array>
#include <mutex>
#include <vector>
#include <thread>
#include <assert.h>
#include <algorithm>
#include <functional>

#include "MString.h"


class MORTY_API MGlobal
{
public:
    static const int M_INVALID_INDEX = -1;

};



class MEngine;

#endif
