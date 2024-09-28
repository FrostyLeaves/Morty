[[vk::binding(1,0)]]Texture2D BackTexture;
[[vk::binding(2,0)]]sampler LinearSampler;

struct VS_OUT
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

PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;

    output.f4FrontColor = float4(0, 0, 0, 0);
    output.fBackColor = float4(0, 0, 0, 0);
    output.fFrontDepth = 0;
    output.fBackDepth = BackTexture.Sample(LinearSampler, input.uv).r;

    return output;
}
