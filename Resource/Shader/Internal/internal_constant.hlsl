#ifndef _M_INTERNAL_CONSTANT_HLSL_
#define _M_INTERNAL_CONSTANT_HLSL_


#ifndef MPOINT_LIGHT_PIXEL_NUMBER
    #define MPOINT_LIGHT_PIXEL_NUMBER 1
#endif

#ifndef MSPOT_LIGHT_PIXEL_NUMBER
    #define MSPOT_LIGHT_PIXEL_NUMBER 1
#endif

#ifndef MPOINT_LIGHT_MAX_NUMBER
    #define MPOINT_LIGHT_MAX_NUMBER 1
#endif

#ifndef MSPOT_LIGHT_MAX_NUMBER
    #define MSPOT_LIGHT_MAX_NUMBER 1
#endif

#ifndef CASCADED_SHADOW_MAP_NUM
    #define CASCADED_SHADOW_MAP_NUM 1
#endif

#ifndef VOXEL_GI_CLIP_MAP_NUM
    #define VOXEL_GI_CLIP_MAP_NUM 1
#endif

#ifndef MESH_LOD_LEVEL_RANGE
    #define MESH_LOD_LEVEL_RANGE 1
#endif

#ifndef NUM_PI
    #define NUM_PI (3.1415926535898)
    #define NUM_PI2 (6.283185307179586)
#endif

#define NUM_BIAS (0.000001f)

#if MTRANSPARENT_POLICY && MEN_TRANSPARENT
    #define MTRANSPARENT_DEPTH_PEELING
#endif

struct DirectionLight
{
    float3 f3Intensity;
    float3 f3LightDir;
    float fLightSize;
};

struct PointLight
{
    float3 f3WorldPosition;

    float3 f3Intensity;
    float fConstant;
    float fLinear;
    float fQuadratic;

};

struct SpotLight
{
    float3 f3WorldPosition;
    float fHalfInnerCutOff;
    float3 f3Direction;
    float fHalfOuterCutOff;
    float3 f3Intensity;
};


struct SurfaceData
{
    float3 f3CameraDir;
    float3 f3Normal;
    float3 f3WorldPosition;
    float3 f3BaseColor;
    float3 f3Albedo;
    float fRoughness;
    float fMetallic;
};

struct VoxelizerOutput
{
    uint nBaseColor[4 * 6];
    uint nVoxelCount[6];
};


struct VoxelClipmap
{
    float3 f3VoxelOrigin;       //voxel map origin position in world space.
    float fVoxelSize;           //how much width does a voxel.
};

struct VoxelMapSetting
{
    VoxelClipmap vClipmap[VOXEL_GI_CLIP_MAP_NUM];
    uint nResolution;          //voxel table resolution.
    uint nClipmapIdx;
};


#endif