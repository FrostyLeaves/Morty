#include "inner_constant.hlsl"
#include "inner_functional.hlsl"
#include "model_struct.hlsl"

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;

    float depth : DEPTH;
    
    float3 normal : NORMAL;
    float3 tangent : Tangent;
    float3 bitangent : BITANGENT;
};

struct Material
{
    float fAlphaFactor;
};

//PS
[[vk::binding(0,0)]]cbuffer cbMaterial
{
    Material U_mat;
};

//Textures
[[vk::binding(1,0)]]Texture2D U_mat_texAlbedo;
[[vk::binding(2,0)]]Texture2D U_mat_texNormal;
[[vk::binding(3,0)]]Texture2D U_mat_texMetallic;
[[vk::binding(4,0)]]Texture2D U_mat_texRoughness;
[[vk::binding(5,0)]]Texture2D U_mat_texAmbientOcc;

struct PS_OUT
{
    float4 f3Base_fMetal: SV_Target0;
    float4 f3Albedo_fAmbientOcc: SV_Target1;
    float4 f3Normal_fRoughness: SV_Target2;
    float4 fDepth: SV_Target3;
};

float3 GetNormal(VS_OUT input)
{
    float3 f3Normal = float3(0.0f, 0.0f, -1.0f);

    f3Normal = U_mat_texNormal.Sample(U_defaultSampler, input.uv).xyz;
    
    //使用法线贴图，法向量在view space，CameraDir也在view space
    float3 T = normalize(input.tangent);
    float3 B = normalize(input.bitangent);
    float3 N = normalize(input.normal);
    float3x3 TBN = float3x3(T,B,N);

    f3Normal = mul(f3Normal, TBN);
    f3Normal = normalize(f3Normal);
    f3Normal = (f3Normal + 1.0f) * 0.5f;

    return f3Normal;
}

PS_OUT PS(VS_OUT input) : SV_Target
{
    PS_OUT output;

    float3 f3Albedo   = pow(U_mat_texAlbedo.Sample(U_defaultSampler, input.uv).rgb, float3(2.2));
    float fMetallic   = U_mat_texMetallic.Sample(U_defaultSampler, input.uv).r;
    float fRoughness  = U_mat_texRoughness.Sample(U_defaultSampler, input.uv).r;
    float fAmbientOcc = U_mat_texAmbientOcc.Sample(U_defaultSampler, input.uv).r;

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    output.f3Base_fMetal.rgb = lerp(float3(0.04), f3Albedo, fMetallic);
    output.f3Base_fMetal.a = fMetallic;

    output.f3Normal_fRoughness.rgb = GetNormal(input);
    output.f3Normal_fRoughness.a = fRoughness;

    output.f3Albedo_fAmbientOcc.rgb = f3Albedo;
    output.f3Albedo_fAmbientOcc.a = fAmbientOcc;

    output.fDepth = FloatToFloat4((input.depth - U_matZNearFar.x) / (U_matZNearFar.y - U_matZNearFar.x));

    return output;
}
    