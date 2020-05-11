#include "privateHeader.hlsl"

struct VS_OUT_SKYBOX
{
    float4 pos : SV_POSITION;
    float3 uvw : UVW;
};

TextureCube SkyTexCube;

float4 PS(VS_OUT_SKYBOX input) : SV_Target
{
    float4 color = SkyTexCube.Sample(U_defaultSampler, input.uvw);
    return color;
}

