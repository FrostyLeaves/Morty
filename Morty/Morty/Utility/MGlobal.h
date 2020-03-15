#ifndef _M_GLOBAL_H_
#define _M_GLOBAL_H_

#ifdef MORTY_EXPORTS
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
	#define MORTY_MAC
#endif

#pragma warnind( disable: 4251 )


typedef unsigned long MObjectID;
typedef unsigned long MResourceID;
typedef unsigned char MByte;


//Asset
extern const char* SUFFIX_VERTEX_SHADER;
extern const char* SUFFIX_PIXEL_SHADER;
//Default Asset
extern const char* DEFAULT_MATERIAL_STATIC;
extern const char* DEFAULT_MATERIAL_SKINNED;
extern const char* DEFAULT_MATERIAL_DRAW2D;
extern const char* DEFAULT_MATERIAL_DRAW3D;
extern const char* DEFAULT_MATERIAL_SKYBOX;
extern const char* DEFAULT_MATERIAL_SHADOW;
extern const char* DEFAULT_MATERIAL_SHADOW_ANIM;

extern const char* DEFAULT_TEXTURE_WHITE;
extern const char* DEFAULT_TEXTURE_BLACK;
extern const char* DEFAULT_TEXTURE_NORMALMAP;

const unsigned int SHADER_PARAM_CODE_MESH_MATRIX = 0;
const unsigned int SHADER_PARAM_CODE_WORLD_MATRIX = 1;
const unsigned int SHADER_PARAM_CODE_ANIMATION = 2;
const unsigned int SHADER_PARAM_CODE_LIGHT = 3;
const unsigned int SHADER_PARAM_CODE_WORLDINFO = 4;
const unsigned int MINTERNAL_SHADER_CBUFFER_NUMBER = 5;

const unsigned int SHADER_PARAM_CODE_SHADOW_MAP = 0;
const unsigned int MINTERNAL_SHADER_TEXTURE_NUMBER = 1;

const unsigned int SHADER_PARAM_CODE_DEFAULT_SAMPLER = 0;
const unsigned int SHADER_PARAM_CODE_SHADOW_SAMPLER = 1;
const unsigned int MINTERNAL_SHADER_SAMPLER_NUMBER = 2;

const unsigned int SHADER_PARAM_CODE_AUTO_UPDATE = 100;
const unsigned int SHADER_PARAM_CODE_MATERIAL = 101;

const unsigned int SHADER_PARAM_CODE_DEFAULT = 1000;


//Shadowmap size
extern const unsigned int MSHADOW_TEXTURE_SIZE;

extern const unsigned int MPOINT_LIGHT_MAX_NUMBER;
extern const unsigned int MPOINT_LIGHT_PIXEL_NUMBER;

extern const unsigned int MSPOT_LIGHT_MAX_NUMBER;
extern const unsigned int MSPOT_LIGHT_PIXEL_NUMBER;

extern const bool MCALC_NORMAL_IN_VS;

//gles
#define MORTY_OPENGLES 1
//directx11
#define MORTY_DIRECTX_11 2

#define RENDER_GRAPHICS MORTY_DIRECTX_11


#define MORTY_RENDER_DATA_STATISTICS true

enum class MEKeyState
{
	DOWN = 1,
	UP = 2,
};


#endif