#include "light_forward.hlsl"

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 uvw : UVW;
};

[[vk::binding(7,0)]]TextureCube SkyTexCube;

float4 PS_MAIN(VS_OUTPUT input) : SV_Target
{
    float4 color = SkyTexCube.Sample(LinearSampler, input.uvw);
    
    // HDR -> LDR
    //color.rgb = color.rgb / (color.rgb + float3(1.0, 1.0, 1.0));
    // Gamma
    //color.rgb = pow(color.rgb, float3(1.0/2.2, 1.0/2.2, 1.0/2.2));

    return color;
}

