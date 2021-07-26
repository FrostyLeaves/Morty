struct VS_OUT_EMPTY
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
    float3 test3 : TEST3;
};

float4 PS(VS_OUT_EMPTY input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

