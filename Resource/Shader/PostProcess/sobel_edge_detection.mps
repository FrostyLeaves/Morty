#include "../PostProcess/post_process_header.hlsl"

[[vk::binding(1,0)]]Texture2D u_texInputTexture;

struct VS_OUT_POST
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

void SampleForKernel(inout float4 output[9], Texture2D tex, float2 f2Coord)
{
	float w = 1.0 / u_f2ViewportSize.x;
	float h = 1.0 / u_f2ViewportSize.y;

	output[0] = tex.Sample(LinearSampler, f2Coord + float2( -w, -h));
	output[1] = tex.Sample(LinearSampler, f2Coord + float2(0.0, -h));
	output[2] = tex.Sample(LinearSampler, f2Coord + float2(  w, -h));
	output[3] = tex.Sample(LinearSampler, f2Coord + float2( -w, 0.0));
	output[4] = tex.Sample(LinearSampler, f2Coord);
	output[5] = tex.Sample(LinearSampler, f2Coord + float2(  w, 0.0));
	output[6] = tex.Sample(LinearSampler, f2Coord + float2( -w, h));
	output[7] = tex.Sample(LinearSampler, f2Coord + float2(0.0, h));
	output[8] = tex.Sample(LinearSampler, f2Coord + float2(  w, h));
}

float4 PS_MAIN(VS_OUT_POST input) : SV_Target
{
	float4 f4Filter[9];
	SampleForKernel(f4Filter, u_texInputTexture, input.uv );

	float4 f4SobelHor = f4Filter[2] + (2.0*f4Filter[5]) + f4Filter[8] - (f4Filter[0] + (2.0*f4Filter[3]) + f4Filter[6]);
  	float4 f4SobelVer = f4Filter[0] + (2.0*f4Filter[1]) + f4Filter[2] - (f4Filter[6] + (2.0*f4Filter[7]) + f4Filter[8]);
	float4 f4Sobel = sqrt((f4SobelHor * f4SobelHor) + (f4SobelVer * f4SobelVer));

    return f4Sobel;
}
