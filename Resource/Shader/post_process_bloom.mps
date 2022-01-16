
[[vk::binding(1,0)]]Texture2D U_HighLight_Tex;
[[vk::binding(2,0)]]sampler LinearSampler;

[[vk::binding(3,0)]]cbuffer {
    float2 U_BlurOffset;
    float U_Gauss[21];
};

struct VS_OUT_POST
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PS(VS_OUT_POST input) : SV_Target
{
    float4 f4Color = U_HighLight_Tex.Sample(LinearSampler, input.uv) * U_Gauss[0];
  
    for (int i = 1; i < 21; i++) {
        float2 uv1 = input.uv - U_BlurOffset * i;
        if (uv1.x >= 0.0 && uv1.y >= 0.0)
        {
            f4Color += U_HighLight_Tex.Sample(LinearSampler, uv1) * U_Gauss[i];
        }
 
        float2 uv2 = input.uv + U_BlurOffset * i;
        if (uv2.x <= 1.0 && uv2.y <= 1.0)
        {
            f4Color += U_HighLight_Tex.Sample(LinearSampler, uv2) * U_Gauss[i];
        }
    }
    return f4Color;
}