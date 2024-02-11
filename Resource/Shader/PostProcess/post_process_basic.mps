#include "../PostProcess/post_process_header.hlsl"

[[vk::binding(1,0)]]Texture2D u_texInputTexture;

struct VS_OUT_POST
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PS_MAIN(VS_OUT_POST input) : SV_Target
{

    float4 color = u_texInputTexture.Sample(LinearSampler, input.uv);
    // HDR -> LDR
    color.rgb = color.rgb / (color.rgb + float3(1.0, 1.0, 1.0));
    // Gamma
    color.rgb = pow(color.rgb, float3(1.0/2.2, 1.0/2.2, 1.0/2.2));


    return color;
}