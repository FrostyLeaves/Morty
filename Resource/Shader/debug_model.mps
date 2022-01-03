#include "light_forward.hlsl"

struct PS_OUT
{
    float4 target0: SV_Target0;
};

PS_OUT PS(VS_OUT input) : SV_Target
{
    PS_OUT output;
    
    float4 f3AmbiColor = U_mat_texDiffuse.Sample(U_defaultSampler, input.uv);

    if(f3AmbiColor.a < 0.8f)
        discard;

    output.target0 = float4(f3AmbiColor);
    
    return output;
}
