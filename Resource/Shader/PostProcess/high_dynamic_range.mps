
[[vk::binding(1,0)]]Texture2D u_texScreenTexture;
[[vk::binding(2,0)]]sampler LinearSampler;
[[vk::binding(3,0)]]float u_HDR_AverageLum;

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PS_OUT
{
    float4 color0 : SV_Target0;
    float4 color1 : SV_Target1;
};

float3 ACESToneMapping(float3 color, float adapted_lum)
{
    /*
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
    */

    float lum = dot(color , float3(0.27,0.67,0.06));
    color *= 0.5 * lum / adapted_lum;
    color /= float3(float3(1.0,1.0,1.0) + color);
    return color;
}

PS_OUT PS_MAIN(VS_OUT input) : SV_Target
{
    PS_OUT output;

    float4 f4Color = u_texScreenTexture.Sample(LinearSampler, input.uv);
    f4Color.xyz = ACESToneMapping(f4Color.xyz, u_HDR_AverageLum);
    
    output.color0 = f4Color;

    float fBrightness = dot(f4Color.rgb, float3(0.2126, 0.7152, 0.0722));
    float fThreshold = 0.75f;

    output.color1.rgb = (fBrightness > fThreshold) ? f4Color.rgb : float3(0.0, 0.0, 0.0);
    output.color1.a = f4Color.a;

    return output;
}