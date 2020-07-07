struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 originPos : OPOS;
};


float time;

float2 screenSize;

float4 PS( VS_OUTPUT input) : SV_Target
{
    float3 uv = float3(0, 0, 0);

    uv.xy = input.pos.xy / screenSize;

    float3 color = 0.5 + 0.5 * cos(time + uv.xyz + float3(0, 2, 4));

    return float4(color.x, color.y, color.z, 1.0f);
}
