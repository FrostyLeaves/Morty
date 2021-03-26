#include "model_header_pbr.hlsl"

#define BIAS 0.000001f 

#ifdef MTRANSPARENT_DEPTH_PEELING

struct PS_OUT
{
    float4 f4FrontColor: SV_Target0;
    float4 fBackColor: SV_Target1;
    float fFrontDepth: SV_Target2;
    float fBackDepth: SV_Target3;
};

#else

struct PS_OUT
{
    float4 target0: SV_Target0;
};

#endif

float3 GetPixelColor(VS_OUT input, float3 f3Color)
{
    return AdditionAllLights(input, f3Color);
}

PS_OUT PS(VS_OUT input) : SV_Target
{
    PS_OUT output;
    
    float3 f3Color = float3(0.0, 0.0, 0.0);
    float fAlpha = saturate(U_mat.fAlphaFactor);

#ifdef MTRANSPARENT_DEPTH_PEELING

    float fZDepth = input.pos.z;
    float fZFront = U_texSubpassInput0.SubpassLoad();
    float fZBack = U_texSubpassInput1.SubpassLoad();

    output.f4FrontColor = float4(0, 0, 0, 0);
    output.fBackColor = float4(0, 0, 0, 0);
    output.fFrontDepth = 1;
    output.fBackDepth = 0;

    // a <= b
    clip(fZDepth + BIAS - fZFront);
    clip(fZBack + BIAS - fZDepth);

    if(fZDepth - BIAS > fZFront && fZDepth + BIAS < fZBack)
    {
        output.fFrontDepth = input.pos.z;
        output.fBackDepth = input.pos.z;
        return output;
    }

    float3 f3Color = GetPixelColor(input, f3Color);

    // color = destColor + srcColor * srcAlpha * (1 - destAlpha)
    // return [srcColor * srcAlpha] as srcColor
    // blend destColor * 1 + srcColor * (1 - destAlpha)
    if(fZFront - BIAS <= fZDepth && fZDepth <= fZFront + BIAS)
        output.f4FrontColor = float4(f3Color * fAlpha, fAlpha);
    else
        output.fBackColor = float4(f3Color, fAlpha);
#else
    output.target0 = float4(GetPixelColor(input, f3Color), fAlpha);
#endif
    
    return output;
}
