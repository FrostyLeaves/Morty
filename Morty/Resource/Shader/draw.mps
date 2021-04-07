
struct VS_OUT_DRAW
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 PS(VS_OUT_DRAW input) : SV_Target
{
    return input.color;
}

