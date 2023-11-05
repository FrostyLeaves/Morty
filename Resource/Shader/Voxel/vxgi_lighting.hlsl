#ifndef _VXGI_LIGHTING_H_
#define _VXGI_LIGHTING_H_

#include "../Internal/internal_uniform_global.hlsl"
#include "../Voxel/voxel_function.hlsl"

float4 SampleVoxelMap(Texture3D<float4> texVoxelMap, float3 f3VoxelUVW)
{
    float4 f4Color = texVoxelMap.SampleLevel(LinearSampler, f3VoxelUVW, 0);

    return f4Color;
}

float4 VoxelConeTrace(Texture3D<float4> texVoxelMap, VoxelMapSetting setting, float3 f3StartPosition, float3 f3Normal, in float coneAperture, uint nConeIdx)
{
    float4 f4Result = float4(0.0f, 0.0f, 0.0f, 1.0f);

    uint nClipLevel = 0;
    float fAlpha = 0.0f;
    float fTraceDist = 0.0f;

    // cone width = ConeWidthCoefficient * TraceDistance
	const float fConeWidthCoefficient = 2 * tan(coneAperture * 0.5);
	
    //offset for avoid sampling self in voxel.
    f3StartPosition += f3Normal * setting.vClipmap[nClipLevel].fVoxelSize;

    float3 f3ConeDirection = VOXEL_DIFFUSE_CONE_DIRECTIONS[nConeIdx];

    while(fAlpha < 1.0f && nClipLevel < VOXEL_GI_CLIP_MAP_NUM)
    {
        float3 f3TracePosition = f3StartPosition + f3ConeDirection * fTraceDist;
       
        if (!ValidVoxelWorldPosition(setting, nClipLevel, f3TracePosition))
        {
            ++nClipLevel;
            continue;
        }

        const float fVoxelSize = setting.vClipmap[nClipLevel].fVoxelSize;
	    const float fVoxelSizeRcp = rcp(fVoxelSize);
        float fDiameter = max(fVoxelSize, fConeWidthCoefficient * fTraceDist);
		float fClipLevel = clamp(log2(fDiameter * fVoxelSizeRcp), 0, VOXEL_GI_CLIP_MAP_NUM - 1);
		uint nSampleClipLevel = floor(fClipLevel);

        float3 f3Coord = WorldPositionToVoxelCoord(setting, nSampleClipLevel, f3TracePosition);
        float3 f3VoxelUVW = GetVoxelTextureUVW(voxelMapSetting, f3Coord, nSampleClipLevel, nConeIdx);

        float4 f4VoxelSampleValue = SampleVoxelMap(texVoxelMap, f3VoxelUVW);

        fTraceDist += fDiameter;
        f4Result.rgb += (1.0f - fAlpha) * f4VoxelSampleValue.rgb;
        fAlpha += (1.0f - fAlpha) * f4VoxelSampleValue.a;
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
        float fCosTheta = dot(f3Normal, f3ConeDirection);
        if (fCosTheta <= 0.0f)
        {
            continue;
        }

        float4 f4TraceResult = VoxelConeTrace(texVoxelMap, setting, f3StartPosition, f3Normal, VOXEL_DIFFUSE_CONE_APERTURE, nConeIdx);

        f4TraceSum += f4TraceResult * fCosTheta;
        fTraceCosTheta += fCosTheta;
    }

    float4 f4Result = max(0, f4TraceSum / fTraceCosTheta);
    f4Result.a = 1.0f;
    return f4Result;
}

#endif