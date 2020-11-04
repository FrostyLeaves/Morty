#include "modelHeader.hlsl"

#define BIAS 0.000001f 

struct PS_OUT
{
    float4 f4FrontColor: SV_Target0;
    float4 fBackColor: SV_Target1;
    float4 fFrontDepth: SV_Target2;
    float4 fBackDepth: SV_Target3;
};

PS_OUT PS(VS_OUT input) : SV_Target
{
    PS_OUT output;
    
    float4 f3AmbiColor = U_mat_texDiffuse.Sample(U_defaultSampler, input.uv);

    float3 f3Color = U_mat.f3Ambient * f3AmbiColor.xyz * 0.2f;

    float fAlpha = saturate(U_mat.fAlphaFactor) * f3AmbiColor.w;

    if (U_mat.bUseTransparentTex > 0)
    {
        float4 transparentColor = U_mat_texTransparent.Sample(U_defaultSampler, input.uv);
        fAlpha *= transparentColor.a;

        clip(fAlpha - 0.1f);
    }

    float fZDepth = input.pos.z;
    float2 f2DepthFrontUV = input.pos.xy;
    f2DepthFrontUV.x /= U_f2ViewportSize.x;
    f2DepthFrontUV.y /= U_f2ViewportSize.y;
    float fZFront = U_texDepthFront.Sample(U_defaultSampler, f2DepthFrontUV.xy);
    float fZBack = U_texDepthBack.Sample(U_defaultSampler, f2DepthFrontUV.xy);

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
      

    f3Color = AdditionAllLights(f3Color, f3AmbiColor, input);


    if(fZFront - BIAS <= fZDepth && fZDepth <= fZFront + BIAS)
    {
        // color = destColor + srcColor * srcAlpha * (1 - destAlpha)
        // return [srcColor * srcAlpha] as srcColor
        // blend destColor * 1 + srcColor * (1 - destAlpha)
        output.f4FrontColor = float4(f3Color * fAlpha, fAlpha);
    }
    else
    {
        output.fBackColor = float4(f3Color, fAlpha);
    }

    return output;
}

