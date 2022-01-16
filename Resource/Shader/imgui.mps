struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

[[vk::binding(0,2)]]Texture2D image;
[[vk::binding(1,2)]]sampler LinearSampler;
[[vk::binding(2,2)]]cbuffer U_IMGUI_PS : register(b2)
{
    int imageType;
};
float4 PS( VS_OUTPUT input) : SV_Target
{
    if (imageType == 0)
        return input.color * image.Sample(LinearSampler, input.uv);
    
    else if(imageType == 1)
    {
        return input.color * float(image.Sample(LinearSampler, input.uv));
    }

    else if(imageType == 2)
    {
        return float4(input.color.rgb * image.Sample(LinearSampler, input.uv).rgb, 1.0f);
    }

    else if(imageType == 3)
    {
        return input.color * float(image.Sample(LinearSampler, input.uv).w);
    }

    return input.color;
}
