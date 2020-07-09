struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 originPos : OPOS;
};

[[vk::binding(0,0)]]cbuffer Frame
{
    float time;
};

[[vk::binding(1,0)]]cbuffer Global
{
    float2 screenSize;
};

//[[vk::binding(0,1)]]Texture2D image;

//[[vk::binding(0,2)]]sampler samp;

float4 PS( VS_OUTPUT input) : SV_Target
{
    float3 uv = float3(0, 0, 0);

    uv.xy = input.pos.xy / screenSize;

    float3 color = 0.5 + 0.5 * cos(time + uv.xyz + float3(0, 2, 4));

    //color = color * 0.5 + image.Sample(samp, uv) * 0.5;

    return float4(color.x, color.y, color.z, 1.0f);
}
