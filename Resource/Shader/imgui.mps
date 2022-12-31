struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

[[vk::binding(0,2)]]Texture2D image;
[[vk::binding(1,2)]]Texture2DArray arrayImage;
[[vk::binding(2,2)]]sampler LinearSampler;
[[vk::binding(3,2)]]cbuffer u_IMGUI_PS : register(b2)
{
    int u_nImageType;
    int u_nImageArray;
    int u_nImageIndex;
};



float4 PS_MAIN( VS_OUTPUT input) : SV_Target
{

    if (u_nImageArray == 0)
    {
        if (u_nImageType == 0)
            return input.color * image.Sample(LinearSampler, input.uv);
        
        else if(u_nImageType == 1)
        {
            return input.color * float(image.Sample(LinearSampler, input.uv).r);
        }

        else if(u_nImageType == 2)
        {
            return float4(input.color.rgb * image.Sample(LinearSampler, input.uv).rgb, 1.0f);
        }

        else if(u_nImageType == 3)
        {
            return input.color * float(image.Sample(LinearSampler, input.uv).w);
        }
    }
    else
    {
        if (u_nImageType == 0)
            return input.color * arrayImage.Sample(LinearSampler, float3(input.uv, u_nImageIndex));
        
        else if(u_nImageType == 1)
        {
            return input.color * float(arrayImage.Sample(LinearSampler, float3(input.uv, u_nImageIndex)).r);
        }

        else if(u_nImageType == 2)
        {
            return float4(input.color.rgb * arrayImage.Sample(LinearSampler, float3(input.uv, u_nImageIndex)).rgb, 1.0f);
        }

        else if(u_nImageType == 3)
        {
            return input.color * float(arrayImage.Sample(LinearSampler, float3(input.uv, u_nImageIndex)).w);
        }
    }

    return input.color;
}
