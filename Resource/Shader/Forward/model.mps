#include "Forward/light_forward.hlsl"

#ifdef MTRANSPARENT_DEPTH_PEELING

struct PS_OUT
{
    float4 f4FrontColor: SV_Target0;
    float4 fBackColor: SV_Target1;
    float fFrontDepth: SV_Target2;
    float fBackDepth: SV_Target3;
};

[[vk::input_attachment_index(0)]] [[vk::binding(10, 1)]] SubpassInput u_texSubpassInput0;
[[vk::input_attachment_index(1)]] [[vk::binding(11, 1)]] SubpassInput u_texSubpassInput1;

#else

struct PS_OUT
{
    float4 target0: SV_Target;
};

#endif

PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;
    
    float4 f3AmbiColor = u_texDiffuse.Sample(LinearSampler, input.uv);

    float3 f3Color = u_xMaterial.f3Ambient * f3AmbiColor.xyz * 0.2f;

    float fAlpha = saturate(u_xMaterial.fAlphaFactor) * f3AmbiColor.w;

    if (u_xMaterial.bUseTransparentTex > 0)
    {
        float4 transparentColor = u_texTransparent.Sample(LinearSampler, input.uv);
        fAlpha *= transparentColor.a;
        clip(fAlpha - 0.1f);
    }

    
#ifdef MTRANSPARENT_DEPTH_PEELING

    float fZDepth = input.pos.z;
    float fZFront = u_texSubpassInput0.SubpassLoad();
    float fZBack = u_texSubpassInput1.SubpassLoad();

    output.f4FrontColor = float4(0, 0, 0, 0);
    output.fBackColor = float4(0, 0, 0, 0);
    output.fFrontDepth = 1;
    output.fBackDepth = 0;

    // a <= b
    clip(fZDepth + NUM_BIAS - fZFront);
    clip(fZBack + NUM_BIAS - fZDepth);

    if(fZDepth - NUM_BIAS > fZFront && fZDepth + NUM_BIAS < fZBack)
    {
        output.fFrontDepth = input.pos.z;
        output.fBackDepth = input.pos.z;
        return output;
    }

    if (u_xMaterial.bUseEmissiveTex > 0)
    {
        float3 f3EmissiveColor = u_texEmissive.Sample(LinearSampler, input.uv);
        if(length(f3EmissiveColor) <= 0.0f)
        {
            f3Color = AdditionAllLights(f3Color, f3AmbiColor, input);
        }
        else
        {
            f3Color += f3EmissiveColor;
        }
    }
    else
    {
        f3Color = AdditionAllLights(f3Color, f3AmbiColor, input);
    }
    // color = destColor + srcColor * srcAlpha * (1 - destAlpha)
    // return [srcColor * srcAlpha] as srcColor
    // blend destColor * 1 + srcColor * (1 - destAlpha)
    if(fZFront - NUM_BIAS <= fZDepth && fZDepth <= fZFront + NUM_BIAS)
        output.f4FrontColor = float4(f3Color * fAlpha, fAlpha);
    else
        output.fBackColor = float4(f3Color, fAlpha);
#else
    if (u_xMaterial.bUseEmissiveTex > 0)
    {
        float3 f3EmissiveColor = u_texEmissive.Sample(LinearSampler, input.uv).xyz;
        if(length(f3EmissiveColor) <= 0.0f)
        {
            f3Color = AdditionAllLights(f3Color, f3AmbiColor, input);
        }
        else
        {
            f3Color += f3EmissiveColor;
        }
    }
    else
    {
        f3Color = AdditionAllLights(f3Color, f3AmbiColor, input);
    }
    output.target0 = float4(f3Color, fAlpha);
#endif
    
    return output;
}
