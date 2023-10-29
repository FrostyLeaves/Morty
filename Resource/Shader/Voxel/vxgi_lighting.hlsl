#ifndef _VXGI_LIGHTING_H_
#define _VXGI_LIGHTING_H_

#include "../Internal/internal_uniform_global.hlsl"
#include "../Voxel/voxel_function.hlsl"

float4 SampleVoxelMap(Texture3D<float4> texVoxelMap, float3 f3VoxelUVW)
{
    float4 f4Color = texVoxelMap.SampleLevel(LinearSampler, f3VoxelUVW, 0);

    return f4Color;
}

float4 VoxelConeTrace(Texture3D<float4> texVoxelMap, VoxelMapSetting setting, float3 f3StartPosition, uint nConeIdx)
{
    float4 f4Result;

    float fAlpha = 0.0f;
    float fTraceDist = 1.0f;
    uint nClipLevel = 0;


    float3 f3ConeDirection = VOXEL_DIFFUSE_CONE_DIRECTIONS[nConeIdx];
    float3 f3AnisoDirection = -f3ConeDirection;
    float3 f3DirectionWeights = abs(f3AnisoDirection);

    while(fAlpha < 1.0f && nClipLevel < VOXEL_GI_CLIP_MAP_NUM)
    {
        float3 f3TracePosition = f3StartPosition + f3ConeDirection * fTraceDist;

        if (!ValidVoxelWorldPosition(setting, nClipLevel, nClipLevel))
        {
            ++nClipLevel;
            continue;
        }

        float3 f3Coord = WorldPositionToVoxelCoord(setting, nClipLevel, f3TracePosition);
        float3 f3VoxelUVW = GetVoxelTextureUVW(voxelMapSetting, f3Coord, nConeIdx);

        float4 f4VoxelSampleValue = SampleVoxelMap(texVoxelMap, f3VoxelUVW);


        fTraceDist += setting.vClipmap[nClipLevel].fVoxelSize;
        f4Result += (1.0f - fAlpha) * f4VoxelSampleValue;
        fAlpha = f4Result.a;
    }


    return f4Result;
}


float4 VoxelDiffuseTracing(Texture3D<float4> texVoxelMap, VoxelMapSetting setting, float3 f3StartPosition, float3 f3Normal)
{
    float4 f4TraceSum = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float fTraceCosTheta = 0;
    for (uint nConeIdx = 0; nConeIdx < VOXEL_DIFFUSE_CONE_COUNT; ++nConeIdx)
    {
        float3 f3ConeDirection = VOXEL_DIFFUSE_CONE_DIRECTIONS[nConeIdx];
        float fNdotC = dot(f3Normal, f3ConeDirection);
        if (fNdotC <= 0.0f)
        {
            continue;
        }

        float4 f4TraceResult = VoxelConeTrace(texVoxelMap, setting, f3StartPosition, nConeIdx);

        f4TraceSum += f4TraceResult;
        fTraceCosTheta += fNdotC;
    }

    float4 f4Result = max(0, f4TraceSum);
    return f4Result;
}

#endif