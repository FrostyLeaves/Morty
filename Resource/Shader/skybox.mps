#include "inner_constant.hlsl"
#include "inner_functional.hlsl"
#include "model_struct.hlsl"

struct VS_OUT_SKYBOX
{
    float4 pos : SV_POSITION;
    float3 uvw : UVW;
};

[[vk::binding(5,0)]]TextureCube SkyTexCube;

float4 PS(VS_OUT_SKYBOX input) : SV_Target
{
    float4 color = SkyTexCube.Sample(U_defaultSampler, input.uvw);
    return color;
}

