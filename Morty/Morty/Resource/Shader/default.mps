cbuffer cbPerObject
{
    float4 testColor;
}

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR;
};


float4 PS(VS_OUT input) : SV_Target
{
    return input.color;
}

