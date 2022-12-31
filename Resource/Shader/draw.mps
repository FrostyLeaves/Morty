
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 PS_MAIN(VS_OUT input) : SV_Target
{
    return input.color;
}

