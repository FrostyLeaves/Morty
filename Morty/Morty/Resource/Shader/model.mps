#include "modelHeader.hlsl"

#define BIAS 0.000001f 

struct PS_OUT
{
    float4 target0: SV_Target0;
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

    f3Color = AdditionAllLights(f3Color, f3AmbiColor, input);

    output.target0 = float4(f3Color, fAlpha);

    return output;
}
