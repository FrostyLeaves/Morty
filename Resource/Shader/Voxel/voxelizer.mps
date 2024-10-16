#include "../Internal/internal_uniform_global.hlsl"
#include "../Internal/internal_uniform_model.hlsl"
#include "../Internal/internal_functional.hlsl"
#include "../Lighting/pbr_uniform_material.hlsl"
#include "../Lighting/pbr_lighting.hlsl"
#include "../Voxel/voxel_function.hlsl"
#include "../Model/universal_vsout.hlsl"
#include "../Voxel/vxgi_lighting.hlsl"


bool IntersectAABB(float3 aabb_min, float3 aabb_max, float3 pos)
{   
    if (pos.x < aabb_min.x || pos.y < aabb_min.y || pos.z < aabb_min.z)
    {
        return false;
    }
    if (pos.x > aabb_max.x || pos.y > aabb_max.y || pos.z > aabb_max.z)
    {
        return false;
    }
    
    return true;
}

float4 PS_MAIN(VS_OUT input) : SV_Target
{
    if (!ValidVoxelWorldPosition(voxelMapSetting, input.worldPos.xyz))
    {
        discard;
    }

    float fVoxelizerScaleInv = float(voxelMapSetting.nResolution) / float(voxelMapSetting.nViewportSize);

    //uint3 n3Coord = uint3(input.pos.x * fVoxelizerScaleInv, voxelMapSetting.nResolution - (input.pos.y * fVoxelizerScaleInv) - 1, input.pos.z * voxelMapSetting.nResolution);
    uint3 n3Coord = uint3(WorldPositionToVoxelCoord(voxelMapSetting, input.worldPos.xyz));


#ifdef VOXELIZER_CONSERVATIVE_RASTERIZATION

	if (!IntersectAABB(input.aabbMin, input.aabbMax, input.worldPos))
    {
		discard;
    }

#endif

	int voxelTableIdx = VoxelCoordToInstanceId(voxelMapSetting, n3Coord);
    
    float3 f3WorldPosition = input.worldPos.xyz;

    float3 f3Normal = input.normal;
    float3 f3CameraDir = f3Normal;
    
    float2 uv = input.uv;
    float3 f3Albedo   = u_mat_texAlbedo.Sample(LinearSampler, uv).rgb * u_xMaterial.f4Albedo.rgb;
    float fMetallic   = u_mat_texMetallic.Sample(LinearSampler, uv)[u_xMaterial.nMetallicChannel] * u_xMaterial.fMetallic;
    float fRoughness  = u_mat_texRoughness.Sample(LinearSampler, uv)[u_xMaterial.nRoughnessChannel] * u_xMaterial.fRoughness;
    float fAmbientOcc = u_mat_texAmbientOcc.Sample(LinearSampler, uv).r;

    float3 f3BaseColor = float3(0.0f, 0.0f, 0.0f);
    f3BaseColor = lerp(f3BaseColor, f3Albedo, fMetallic);

    SurfaceData pointData;
    pointData.f3CameraDir = f3CameraDir;
    pointData.f3Normal = f3Normal;
    pointData.f3WorldPosition = f3WorldPosition;
    pointData.f3BaseColor = f3BaseColor;
    pointData.f3Albedo = f3Albedo;
    pointData.fRoughness = fRoughness;
    pointData.fMetallic = fMetallic;
    pointData.bReceiveShadow = false;

    float4 f4LightingColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    //f4LightingColor.rgb = PbrLighting(pointData);
    
    float shadow = GetDirectionShadow(u_texShadowMap, pointData.f3WorldPosition, pointData.f3Normal, -u_xDirectionalLight.f3LightDir);
    f4LightingColor.rgb = u_xDirectionalLight.f3Intensity * f3BaseColor * dot(f3Normal, - u_xDirectionalLight.f3LightDir) * shadow;

    //float4 f4VXGIColor = VoxelDiffuseTracing(u_texVoxelMap, voxelMapSetting, f3WorldPosition, f3Normal);
    //f4LightingColor.rgb = mad(f4LightingColor.rgb, 1.0f - f4VXGIColor.a, f4VXGIColor.rgb);

    float3 f3AnisoDirection = f3Normal;
    float3 f3DirectionWeight = abs(f3Normal);
    f3DirectionWeight = normalize(f3DirectionWeight);

    uint temp = 0;

    if (f3DirectionWeight.x > 0)
    {
        float4 color = f4LightingColor * f3DirectionWeight.x;
        uint nAnisoIdx = f3AnisoDirection.x < 0 ? 0 : 1;

        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nVoxelCount[nAnisoIdx], 1, temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 0], VoxelFloatToUint(color.r), temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 1], VoxelFloatToUint(color.g), temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 2], VoxelFloatToUint(color.b), temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 3], VoxelFloatToUint(color.a), temp);
    }

    if (f3DirectionWeight.y > 0)
    {
        float4 color = f4LightingColor * f3DirectionWeight.y;
        uint nAnisoIdx = f3AnisoDirection.y < 0 ? 2 : 3;

        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nVoxelCount[nAnisoIdx], 1, temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 0], VoxelFloatToUint(color.r), temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 1], VoxelFloatToUint(color.g), temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 2], VoxelFloatToUint(color.b), temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 3], VoxelFloatToUint(color.a), temp);
    }

    if (f3DirectionWeight.z > 0)
    {
        float4 color = f4LightingColor * f3DirectionWeight.z;
        uint nAnisoIdx = f3AnisoDirection.z < 0 ? 4 : 5;

        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nVoxelCount[nAnisoIdx], 1, temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 0], VoxelFloatToUint(color.r), temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 1], VoxelFloatToUint(color.g), temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 2], VoxelFloatToUint(color.b), temp);
        InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 3], VoxelFloatToUint(color.a), temp);
    }




    return f4LightingColor;
}