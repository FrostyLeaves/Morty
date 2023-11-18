#ifndef _M_VOXEL_STRUCT_DEFINE_HLSL_
#define _M_VOXEL_STRUCT_DEFINE_HLSL_

#include "../Internal/internal_constant.hlsl"

static const float VOXEL_DIFFUSE_CONE_APERTURE = 0.872665f;

static const float3 VOXEL_DIFFUSE_CONE_DIRECTIONS[VOXEL_DIFFUSE_CONE_COUNT] = {
	float3(0.57735f, 0.57735f, 0.57735f),
	float3(0.57735f, -0.57735f, -0.57735f),
	float3(-0.57735f, 0.57735f, -0.57735f),
	float3(-0.57735f, -0.57735f, 0.57735f),
	float3(-0.903007f, -0.182696f, -0.388844f),
	float3(-0.903007f, 0.182696f, 0.388844f),
	float3(0.903007f, -0.182696f, 0.388844f),
	float3(0.903007f, 0.182696f, -0.388844f),
	float3(-0.388844f, -0.903007f, -0.182696f),
	float3(0.388844f, -0.903007f, 0.182696f),
	float3(0.388844f, 0.903007f, -0.182696f),
	float3(-0.388844f, 0.903007f, 0.182696f),
	float3(-0.182696f, -0.388844f, -0.903007f),
	float3(0.182696f, 0.388844f, -0.903007f),
	float3(-0.182696f, 0.388844f, 0.903007f),
	float3(0.182696f, -0.388844f, 0.903007f)
};

uint VoxelFloatToUint(float value)
{
    return uint(value * 1000.0f);
}

float VoxelUintToFloat(uint value)
{
    return float(value) / 1000.0f;
}

uint VoxelCoordToInstanceId(VoxelMapSetting setting, uint3 n3Coord)
{
    uint nResolution = setting.nResolution;

    uint instanceId = uint(n3Coord.z * (nResolution * nResolution) +
                    n3Coord.y * nResolution +
                    n3Coord.x);
    
    return instanceId;
}

uint GetVoxelSimilarFaceIndex(float3 f3Direction)
{
    float3 f3DirectionWeight = abs(f3Direction);
    if (f3DirectionWeight.x > f3DirectionWeight.y && f3DirectionWeight.x > f3DirectionWeight.z)
    {
        return f3Direction.x < 0 ? 0 : 1;
    }
    else if (f3DirectionWeight.y > f3DirectionWeight.z)
    {
        return f3Direction.y < 0 ? 2 : 3;
    }

    return f3Direction.z < 0 ? 4 : 5;
}

uint3 GetVoxelFaceIndex(float3 f3Direction)
{
    uint3 n3Result;
    float3 f3DirectionWeight = abs(f3Direction);
    
    n3Result.x = f3Direction.x < 0 ? 0 : 1;
    n3Result.y = f3Direction.y < 0 ? 2 : 3;
    n3Result.z = f3Direction.z < 0 ? 4 : 5;

    return n3Result;
}

uint3 InstanceIdToVoxelCoord(VoxelMapSetting setting, uint instanceId)
{
    VoxelClipmap clipmap = setting.vClipmap[setting.nClipmapIdx];
    uint nResolution = setting.nResolution;

    int num = instanceId;

    uint3 n3Coord;
    n3Coord.z = num / (nResolution * nResolution);
    num = num - n3Coord.z * (nResolution * nResolution);

    n3Coord.y = num / nResolution;
    num = num - n3Coord.y * nResolution;

    n3Coord.x = num;
    
    return n3Coord;
}

uint ValidVoxelWorldPosition(VoxelMapSetting setting, uint nClipmapIdx, float3 position)
{
    VoxelClipmap clipmap = setting.vClipmap[nClipmapIdx];

    float3 f3VoxelSize = float(setting.nResolution - 1) * clipmap.fVoxelSize;
    if (all(position == clamp(position, clipmap.f3VoxelOrigin, clipmap.f3VoxelOrigin + f3VoxelSize)))
    {
        return 1;
    }

    return 0;
}

uint ValidVoxelWorldPosition(VoxelMapSetting setting, float3 position)
{
    return ValidVoxelWorldPosition(setting, setting.nClipmapIdx, position);
}

float3 WorldPositionToVoxelCoord(VoxelMapSetting setting, uint nClipmapIdx, float3 position)
{
    VoxelClipmap clipmap = setting.vClipmap[nClipmapIdx];

    float3 addition = (position - clipmap.f3VoxelOrigin);
    float3 f3Coord = float3( addition / clipmap.fVoxelSize );

    return f3Coord;
}

float3 WorldPositionToVoxelCoord(VoxelMapSetting setting, float3 position)
{
    return WorldPositionToVoxelCoord(setting, setting.nClipmapIdx, position);
}

// return world position of the voxel center point.
float3 VoxelCoordToWorldPosition(VoxelMapSetting setting, uint3 n3Coord)
{
    VoxelClipmap clipmap = setting.vClipmap[setting.nClipmapIdx];

    float3 position = clipmap.f3VoxelOrigin + 
            (float3(n3Coord) + float3(0.5f, 0.5f, 0.5f)) * clipmap.fVoxelSize;

    return position;
}

// get voxel texture3D uvw for sample.
float3 GetVoxelTextureUVW(VoxelMapSetting setting, float3 f3Coord, uint nClipmapIdx, uint nConeIdx)
{
    float3 f3PixelIdx = f3Coord + float3(setting.nResolution * nConeIdx, setting.nResolution * nClipmapIdx, 0);

    float3 f3VoxelUVW = f3PixelIdx / float3(setting.nResolution * VOXEL_DIFFUSE_CONE_COUNT, setting.nResolution  * VOXEL_GI_CLIP_MAP_NUM, setting.nResolution);

    return f3VoxelUVW;
}

// get voxel texture3D uvw for write.
uint3 GetVoxelTexturePixelIdx(VoxelMapSetting setting, uint3 n3Coord, uint nClipmapIdx, uint nConeIdx)
{
    uint3 n3PixelIdx = n3Coord + uint3(setting.nResolution * nConeIdx, setting.nResolution * nClipmapIdx, 0);

    return n3PixelIdx;
}


#endif