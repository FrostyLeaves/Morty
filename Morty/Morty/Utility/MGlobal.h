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

class MORTY_API MGlobal
{
public:
    static const int M_INVALID_INDEX;

    //Asset
    static const char* SUFFIX_VERTEX_SHADER;
    static const char* SUFFIX_PIXEL_SHADER;
    static const char* SUFFIX_MATERIAL;
    static const char* SUFFIX_SKELETON;
    static const char* SUFFIX_MESH;
    static const char* SUFFIX_NODE;
    static const char* SUFFIX_SKELANIM;

    //Default Asset
    static const char* DEFAULT_MATERIAL_MODEL_STATIC_MESH;
	static const char* DEFAULT_MATERIAL_MODEL_SKELETON_MESH;
	static const char* DEFAULT_MATERIAL_MODEL_STATIC_MESH_PBR;
	static const char* DEFAULT_MATERIAL_MODEL_SKELETON_MESH_PBR;
    static const char* DEFAULT_MATERIAL_DRAW2D;
    static const char* DEFAULT_MATERIAL_DRAW3D;
    static const char* DEFAULT_MATERIAL_SHADOW_STATIC;
    static const char* DEFAULT_MATERIAL_SHADOW_SKELETON;
    static const char* DEFAULT_MATERIAL_DEPTH_PEEL_BLEND;
    static const char* DEFAULT_MATERIAL_DEPTH_PEEL_FILL;
    static const char* DEFAULT_MESH_SCREEN_DRAW;

    static const char* DEFAULT_TEXTURE_WHITE;
    static const char* DEFAULT_TEXTURE_BLACK;
    static const char* DEFAULT_TEXTURE_NORMALMAP;

    static const char* SHADER_PARAM_NAME_DIFFUSE;
    static const char* SHADER_PARAM_NAME_NORMAL;


    //Shadowmap size
    static const uint32_t MSHADOW_TEXTURE_SIZE;

    static const uint32_t MPOINT_LIGHT_MAX_NUMBER;
    static const uint32_t MPOINT_LIGHT_PIXEL_NUMBER;

    static const uint32_t MSPOT_LIGHT_MAX_NUMBER;
    static const uint32_t MSPOT_LIGHT_PIXEL_NUMBER;

    static const bool MCALC_NORMAL_IN_VS;

    static const uint32_t MMESH_LOD_LEVEL_RANGE;


	//Material Macro
    static const char* MATERIAL_MACRO_SKELETON_ENABLE;


    static const uint32_t SHADER_PARAM_SET_MATERIAL;
    static const uint32_t SHADER_PARAM_SET_FRAME;
    static const uint32_t SHADER_PARAM_SET_MESH;
    static const uint32_t SHADER_PARAM_SET_SKELETON;
};

//Bones number per Vertex.
#define MBONES_PER_VERTEX (4)
#define MBONES_MAX_NUMBER (128)

#define M_BUFFER_NUM 3

#define M_VALID_SHADER_SET_NUM 4


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
