#ifndef _M_GLOBAL_H_
#define _M_GLOBAL_H_

#include <stdint.h>

#ifdef Morty_EXPORTS
#if defined(__WINDOWS_) || defined(_WINDOWS) || defined(WIN32)
        #define MORTY_CLASS __declspec(dllexport)
    #else
        #define MORTY_CLASS
    #endif
#else
	#if defined(__WINDOWS_) || defined(_WINDOWS) || defined(WIN32)
        #define MORTY_CLASS __declspec(dllimport)
    #else
        #define MORTY_CLASS
    #endif
#endif

#if defined(__WINDOWS_) || defined(_WINDOWS) || defined(WIN32)
	#define MORTY_WIN
#else
	#define MORTY_ANDROID
    #define MORTY_IOS
#endif


#ifndef M_PI 
#define M_PI (3.14159265358979323846)
#endif

#define ROW_MAJOR 1
#define COL_MAJOR 2

//#define MATRIX_MAJOR ROW_MAJOR
#define MATRIX_MAJOR COL_MAJOR



#pragma warnind( disable: 4251 )
#pragma warnind( disable: 4275 )


typedef unsigned long MObjectID;
typedef unsigned long MResourceID;
typedef unsigned char MByte;

extern const int M_INVALID_INDEX;

//Asset
extern const char* SUFFIX_VERTEX_SHADER;
extern const char* SUFFIX_PIXEL_SHADER;
extern const char* SUFFIX_MATERIAL;
extern const char* SUFFIX_SKELETON;
extern const char* SUFFIX_MESH;
extern const char* SUFFIX_NODE;
extern const char* SUFFIX_SKELANIM;

//Default Asset
extern const char* DEFAULT_MATERIAL_MODEL_STATIC_MESH;
extern const char* DEFAULT_MATERIAL_MODEL_SKELETON_MESH;
extern const char* DEFAULT_MATERIAL_DRAW2D;
extern const char* DEFAULT_MATERIAL_DRAW3D;
extern const char* DEFAULT_MATERIAL_SHADOW_STATIC;
extern const char* DEFAULT_MATERIAL_SHADOW_SKELETON;
extern const char* DEFAULT_MATERIAL_DEPTH_PEEL_BLEND;
extern const char* DEFAULT_MATERIAL_DEPTH_PEEL_FILL;

extern const char* DEFAULT_TEXTURE_WHITE;
extern const char* DEFAULT_TEXTURE_BLACK;
extern const char* DEFAULT_TEXTURE_NORMALMAP;

extern const char* SHADER_PARAM_NAME_DIFFUSE;
extern const char* SHADER_PARAM_NAME_NORMAL;


//Shadowmap size
extern const uint32_t MSHADOW_TEXTURE_SIZE;

extern const uint32_t MPOINT_LIGHT_MAX_NUMBER;
extern const uint32_t MPOINT_LIGHT_PIXEL_NUMBER;

extern const uint32_t MSPOT_LIGHT_MAX_NUMBER;
extern const uint32_t MSPOT_LIGHT_PIXEL_NUMBER;

extern const bool MCALC_NORMAL_IN_VS;


extern const uint32_t MMESH_LOD_LEVEL_RANGE;


//Bones number per Vertex.
#define MBONES_PER_VERTEX (4)
#define MBONES_MAX_NUMBER (128)

//Material Macro
extern const char* MATERIAL_MACRO_SKELETON_ENABLE;

#define M_BUFFER_NUM 3

#define M_VALID_SHADER_SET_NUM 4
const uint32_t SHADER_PARAM_SET_MATERIAL = 0;
const uint32_t SHADER_PARAM_SET_FRAME = 1;
const uint32_t SHADER_PARAM_SET_MESH = 2;
const uint32_t SHADER_PARAM_SET_SKELETON = 3;

//vulkan
#define MORTY_VULKAN 1
//directx11
#define MORTY_DIRECTX_11 2

#define RENDER_GRAPHICS MORTY_VULKAN


#define MORTY_RENDER_DATA_STATISTICS true

enum class MEKeyState
{
	DOWN = 1,
	UP = 2,
};



#include "MLogManager.h"
#endif