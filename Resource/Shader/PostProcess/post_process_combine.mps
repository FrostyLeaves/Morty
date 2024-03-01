#include "../PostProcess/post_process_header.hlsl"

[[vk::binding(1,0)]]Texture2D u_texInputTexture;
[[vk::binding(2,0)]]Texture2D u_texInputTexture1;

float4 PS_MAIN(VS_OUT_POST input) : SV_Target
{
    float4 f4Color = float4(0, 0, 0, 0);
    f4Color += u_texInputTexture.Sample(LinearSampler, input.uv);
    f4Color += u_texInputTexture1.Sample(LinearSampler, input.uv);

    return f4Color;
}