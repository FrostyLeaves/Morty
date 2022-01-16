
[[vk::binding(1,0)]]Texture2D U_Post_Texture;
[[vk::binding(2,0)]]sampler LinearSampler;

struct VS_OUT_POST
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PS(VS_OUT_POST input) : SV_Target
{
    return U_Post_Texture.Sample(LinearSampler, input.uv);
}