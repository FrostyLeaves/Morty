
struct VS_OUT
{
    float4 posH : SV_POSITION;
    float2 uv : UV;
};

sampler sampler0;
Texture2D texture0;

float4 PS(VS_OUT input) : SV_Target
{
    return texture0.Sample(sampler0, input.uv);
}

