#ifndef _M_GLOBAL_H_
#define _M_GLOBAL_H_

#include <stdint.h>


//define MORTY_WIN
//define MORTY_MACOS
//#define MORTY_ANDROID
//#define MORTY_IOS

#ifdef MORTY_WIN
    #ifdef MORTY_EXPORTS
        #define MORTY_API __declspec(dllexport)
    #else
        #define MORTY_API __declspec(dllimport)
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

extern MORTY_API const int M_INVALID_INDEX;

//Asset
extern MORTY_API const char* SUFFIX_VERTEX_SHADER;
extern MORTY_API const char* SUFFIX_PIXEL_SHADER;
extern MORTY_API const char* SUFFIX_MATERIAL;
extern MORTY_API const char* SUFFIX_SKELETON;
extern MORTY_API const char* SUFFIX_MESH;
extern MORTY_API const char* SUFFIX_NODE;
extern MORTY_API const char* SUFFIX_SKELANIM;

//Default Asset
extern MORTY_API const char* DEFAULT_MATERIAL_MODEL_STATIC_MESH;
extern MORTY_API const char* DEFAULT_MATERIAL_MODEL_SKELETON_MESH;
extern MORTY_API const char* DEFAULT_MATERIAL_DRAW2D;
extern MORTY_API const char* DEFAULT_MATERIAL_DRAW3D;
extern MORTY_API const char* DEFAULT_MATERIAL_SHADOW_STATIC;
extern MORTY_API const char* DEFAULT_MATERIAL_SHADOW_SKELETON;
extern MORTY_API const char* DEFAULT_MATERIAL_DEPTH_PEEL_BLEND;
extern MORTY_API const char* DEFAULT_MATERIAL_DEPTH_PEEL_FILL;
extern MORTY_API const char* DEFAULT_MESH_SCREEN_DRAW;

extern MORTY_API const char* DEFAULT_TEXTURE_WHITE;
extern MORTY_API const char* DEFAULT_TEXTURE_BLACK;
extern MORTY_API const char* DEFAULT_TEXTURE_NORMALMAP;

extern MORTY_API const char* SHADER_PARAM_NAME_DIFFUSE;
extern MORTY_API const char* SHADER_PARAM_NAME_NORMAL;


//Shadowmap size
extern MORTY_API const uint32_t MSHADOW_TEXTURE_SIZE;

extern MORTY_API const uint32_t MPOINT_LIGHT_MAX_NUMBER;
extern MORTY_API const uint32_t MPOINT_LIGHT_PIXEL_NUMBER;

extern MORTY_API const uint32_t MSPOT_LIGHT_MAX_NUMBER;
extern MORTY_API const uint32_t MSPOT_LIGHT_PIXEL_NUMBER;

extern MORTY_API const bool MCALC_NORMAL_IN_VS;


extern MORTY_API const uint32_t MMESH_LOD_LEVEL_RANGE;


//Bones number per Vertex.
#define MBONES_PER_VERTEX (4)
#define MBONES_MAX_NUMBER (128)

//Material Macro
extern MORTY_API const char* MATERIAL_MACRO_SKELETON_ENABLE;

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
