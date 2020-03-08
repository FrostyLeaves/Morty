#include "privateHeader.hlsl"

struct VS_OUT_SKYBOX
{
    float4 pos : SV_POSITION;
    float3 uvw : UVW;
};

sampler sampler0
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

TextureCube SkyTexCube;

float4 PS(VS_OUT_SKYBOX input) : SV_Target
{
    float4 color = SkyTexCube.Sample(sampler0, input.uvw);
    return color;
}

