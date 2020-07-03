struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 originPos : OPOS;
};


float4 PS( VS_OUTPUT input) : SV_Target
{
    return float4(input.originPos.x, input.originPos.y, 0.0f, 1.0f);
}