#include "../Internal/internal_functional.hlsl"

struct VS_INPUT
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD;
    float4 u32Color : u32COLOR;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 u32Color : u32COLOR;
};

[[vk::binding(0,0)]] cbuffer u_IMGUI
{
    float2 u_f2Scale;
    float2 u_f2Translate;
};


[[vk::binding(0,2)]]Texture2D image;
[[vk::binding(1,2)]]Texture2DArray arrayImage;
[[vk::binding(2,2)]]Texture2D<uint> uintImage;
[[vk::binding(3,2)]]Texture2DArray<uint> uintArrayImage;

[[vk::binding(4,2)]]sampler LinearSampler;
[[vk::binding(5,2)]]cbuffer u_IMGUI_PS
{
    int u_nImageType;
    int u_nSingleChannelFlag;
    int u_nImageIndex;
    float2 u_nImageSize;
};

VS_OUT VS_MAIN( VS_INPUT input)
{
    VS_OUT output;

    output.pos = float4(input.pos * u_f2Scale + u_f2Translate, 0.0f, 1.0f);
    output.uv = input.uv;
    output.u32Color = input.u32Color;
    return output;
}


float4 PS_MAIN( VS_OUT input) : SV_Target
{

    float4 sampleResult = float4(0, 0, 0, 0);

    if (u_nImageType == 0)
    {
        sampleResult = image.Sample(LinearSampler, input.uv);

    }
    else if (u_nImageType == 1)
    {
        sampleResult = arrayImage.Sample(LinearSampler, float3(input.uv, u_nImageIndex));

    }
    else if (u_nImageType == 2)
    {
        sampleResult = uintImage.Load(int3(input.uv.x * u_nImageSize.x, input.uv.y * u_nImageSize.y, 0));
    }
    else if (u_nImageType == 3)
    {
        sampleResult = uintArrayImage.Load(int4(input.uv.x * u_nImageSize.x, input.uv.y * u_nImageSize.y, 0, u_nImageIndex));
    }

    if(u_nSingleChannelFlag == 1)
    {
        sampleResult.gba = float3(sampleResult.r, sampleResult.r, 1.0f);
    }

    return input.u32Color * sampleResult;
}
