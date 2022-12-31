struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
    float3 test3 : TEST3;
};

float4 PS_MAIN(VS_OUT input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

