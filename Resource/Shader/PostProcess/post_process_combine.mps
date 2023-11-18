
[[vk::binding(1,0)]]Texture2D u_texScreenTexture0;
[[vk::binding(2,0)]]Texture2D u_texScreenTexture1;
[[vk::binding(3,0)]]sampler LinearSampler;

struct VS_OUT_POST
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PS_MAIN(VS_OUT_POST input) : SV_Target
{
    float4 f4Color = float4(0, 0, 0, 0);
    f4Color += u_texScreenTexture0.Sample(LinearSampler, input.uv);
    f4Color += u_texScreenTexture1.Sample(LinearSampler, input.uv);

    return f4Color;
}