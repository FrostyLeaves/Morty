[[vk::binding(1,0)]]Texture2D frontTex;
[[vk::binding(2,0)]]Texture2D backTex;
[[vk::binding(3,0)]]sampler defaultSampler;

struct VS_OUT_DP
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PS(VS_OUT_DP input) : SV_Target
{
    float4 frontColor = frontTex.Sample(defaultSampler, input.uv);
    float4 backColor = backTex.Sample(defaultSampler, input.uv);

    float3 color = backColor.rgb * (1 - frontColor.a) + frontColor.rgb;
    float alpha = frontColor.a + (1 - frontColor.a) * backColor.a;

    return float4(color, alpha);
}

