#include "../Internal/internal_uniform_global.hlsl"

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 uvw : UVW;
};

[[vk::binding(7,0)]]TextureCube u_texSkyBox;

float4 PS_MAIN(VS_OUT input) : SV_Target
{
    float4 color = u_texSkyBox.Sample(LinearSampler, input.uvw);
    
    // HDR -> LDR
    //color.rgb = color.rgb / (color.rgb + float3(1.0, 1.0, 1.0));
    // Gamma
    //color.rgb = pow(color.rgb, float3(1.0/2.2, 1.0/2.2, 1.0/2.2));

    return color;
}

