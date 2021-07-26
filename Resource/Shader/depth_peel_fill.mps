[[vk::binding(1,0)]]Texture2D frontTex;
[[vk::binding(2,0)]]Texture2D backTex;
[[vk::binding(3,0)]]sampler defaultSampler;

struct VS_OUT_DP
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PS_OUT
{
    float4 f4FrontColor: SV_Target0;
    float4 fBackColor: SV_Target1;
    float fFrontDepth: SV_Target2;
    float fBackDepth: SV_Target3;
};

PS_OUT PS(VS_OUT_DP input) : SV_Target
{
    PS_OUT output;

    output.f4FrontColor = float4(0, 0, 0, 0);
    output.fBackColor = float4(0, 0, 0, 0);
    output.fFrontDepth = 0;
    output.fBackDepth = backTex.Sample(defaultSampler, input.uv);

    return output;
}
