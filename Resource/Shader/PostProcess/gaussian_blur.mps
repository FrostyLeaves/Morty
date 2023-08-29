
[[vk::binding(1,0)]]Texture2D HDROriginTexture;
[[vk::binding(2,0)]]sampler LinearSampler;

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 uv01 : TEXCOORD01;
    float4 uv23 : TEXCOORD23;
    float4 uv45 : TEXCOORD45;
};

float4 PS_MAIN(VS_OUT input) : SV_Target
{
    float4 f4Color = float4(0, 0, 0, 0);
    
    f4Color += 0.40 * HDROriginTexture.Sample(LinearSampler, input.uv);
    f4Color += 0.15 * HDROriginTexture.Sample(LinearSampler, input.uv01.xy);
    f4Color += 0.15 * HDROriginTexture.Sample(LinearSampler, input.uv01.zw);
    f4Color += 0.10 * HDROriginTexture.Sample(LinearSampler, input.uv23.xy);
    f4Color += 0.10 * HDROriginTexture.Sample(LinearSampler, input.uv23.zw);
    f4Color += 0.05 * HDROriginTexture.Sample(LinearSampler, input.uv45.xy);
    f4Color += 0.05 * HDROriginTexture.Sample(LinearSampler, input.uv45.zw);

    return f4Color;
}