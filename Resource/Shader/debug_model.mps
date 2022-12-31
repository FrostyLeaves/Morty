#include "light_forward.hlsl"

struct PS_OUT
{
    float4 target0: SV_Target0;
};

PS_OUT PS_MAIN(VS_OUT input) : SV_Target
{
    PS_OUT output;
    
    float4 f4AmbiColor = u_texDiffuse.Sample(LinearSampler, input.uv);

    if(f4AmbiColor.a < 0.8f)
        discard;


    output.target0 = f4AmbiColor;
    return output;
}
