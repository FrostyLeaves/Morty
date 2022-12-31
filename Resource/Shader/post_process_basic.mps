

[[vk::binding(1,0)]]Texture2D u_texFilterTexture;
[[vk::binding(2,0)]]sampler LinearSampler;

struct VS_OUT_POST
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PS_MAIN(VS_OUT_POST input) : SV_Target
{
    return u_texFilterTexture.Sample(LinearSampler, input.uv);
}