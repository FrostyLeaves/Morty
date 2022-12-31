[[vk::binding(1,0)]]Texture2D FrontTexture;
[[vk::binding(2,0)]]Texture2D BackTexture;
[[vk::binding(3,0)]]sampler LinearSampler;

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PS_MAIN(VS_OUT input) : SV_Target
{
    float4 f4FrontColor = FrontTexture.Sample(LinearSampler, input.uv);
    float4 f4BackColor = BackTexture.Sample(LinearSampler, input.uv);

    float3 f3Color = backColor.rgb * (1 - f4FrontColor.a) + f4FrontColor.rgb;
    float fAlpha = f4FrontColor.a + (1 - f4FrontColor.a) * f4BackColor.a;

    return float4(f3Color, fAlpha);
}

