struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
};

[[vk::binding(0,0)]]cbuffer Frame
{
    float time;
};

[[vk::binding(1,0)]]cbuffer Global
{
    float2 screenSize;
};

[[vk::binding(2,0)]]Texture2D image;

[[vk::binding(3,0)]]sampler samp;

float4 PS( VS_OUTPUT input) : SV_Target
{
    float3 uv = float3(0, 0, 0);

    uv.x = input.pos.x / screenSize.x;
    uv.y = input.pos.y / screenSize.y;

    float3 color = 0.5 + 0.5 * cos(time + uv.xyz + float3(0, 2, 4));

    color = color * 0.5 + image.Sample(samp, float2(uv.x, uv.y)) * 0.5;

    return float4(color.x, color.y, color.z, 1.0f);
}
