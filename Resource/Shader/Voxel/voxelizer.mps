#include "../Internal/internal_uniform_global.hlsl"
#include "../Internal/internal_uniform_model.hlsl"
#include "../Internal/internal_functional.hlsl"
#include "../Lighting/pbr_uniform_material.hlsl"
#include "../Lighting/pbr_lighting.hlsl"
#include "../Voxel/voxel_function.hlsl"
#include "../Deferred/deferred_vsout.hlsl"

float4 PS_MAIN(VS_OUT input) : SV_Target
{
    
    uint3 voxelUVW = WorldPositionToVoxelUVW(voxelMapSetting, input.worldPos.xyz);

	int voxelTableIdx = VoxelUVWToInstanceId(voxelMapSetting, voxelUVW);
    
    float3 f3WorldPosition = input.worldPos.xyz;
    float3 f3CameraDir = normalize(u_f3CameraPosition - f3WorldPosition);
    float3 f3Normal = input.normal;
    
    float2 uv = input.uv;
    float3 f3Albedo   = u_mat_texAlbedo.Sample(LinearSampler, uv).rgb * u_xMaterial.f4Albedo.rgb;
    float fMetallic   = u_mat_texMetallic.Sample(LinearSampler, uv).r * u_xMaterial.fMetallic;
    float fRoughness  = u_mat_texRoughness.Sample(LinearSampler, uv).r * u_xMaterial.fRoughness;
    float fAmbientOcc = u_mat_texAmbientOcc.Sample(LinearSampler, uv).r;

    float3 f3BaseColor = float3(0.04, 0.04, 0.04);
    f3BaseColor = lerp(f3BaseColor, f3Albedo, fMetallic);

    SurfaceData pointData;
    pointData.f3CameraDir = f3CameraDir;
    pointData.f3Normal = f3Normal;
    pointData.f3WorldPosition = f3WorldPosition;
    pointData.f3BaseColor = f3BaseColor;
    pointData.f3Albedo = f3Albedo;
    pointData.fRoughness = fRoughness;
    pointData.fMetallic = fMetallic;

    float3 color = PbrLighting(pointData);

    uint temp = 0;
    InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nVoxelCount, 1, temp);
    InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor_R, VoxelFloatToUint(color.r), temp);
    InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor_G, VoxelFloatToUint(color.g), temp);
    InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor_B, VoxelFloatToUint(color.b), temp);

    return float4(color.r, color.g, color.b, 1.0f);
}