struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

[[vk::binding(0,2)]]Texture2D image;
[[vk::binding(1,2)]]Texture2DArray arrayImage;
[[vk::binding(2,2)]]sampler LinearSampler;
[[vk::binding(3,2)]]cbuffer U_IMGUI_PS : register(b2)
{
    int imageType;
    int imageArray;
    int imageIndex;
};



float4 PS( VS_OUTPUT input) : SV_Target
{

    if (imageArray == 0)
    {
        if (imageType == 0)
            return input.color * image.Sample(LinearSampler, input.uv);
        
        else if(imageType == 1)
        {
            return input.color * float(image.Sample(LinearSampler, input.uv).r);
        }

        else if(imageType == 2)
        {
            return float4(input.color.rgb * image.Sample(LinearSampler, input.uv).rgb, 1.0f);
        }

        else if(imageType == 3)
        {
            return input.color * float(image.Sample(LinearSampler, input.uv).w);
        }
    }
    else
    {
        if (imageType == 0)
            return input.color * arrayImage.Sample(LinearSampler, float3(input.uv, imageIndex));
        
        else if(imageType == 1)
        {
            return input.color * float(arrayImage.Sample(LinearSampler, float3(input.uv, imageIndex)).r);
        }

        else if(imageType == 2)
        {
            return float4(input.color.rgb * arrayImage.Sample(LinearSampler, float3(input.uv, imageIndex)).rgb, 1.0f);
        }

        else if(imageType == 3)
        {
            return input.color * float(arrayImage.Sample(LinearSampler, float3(input.uv, imageIndex)).w);
        }
    }

    return input.color;
}
