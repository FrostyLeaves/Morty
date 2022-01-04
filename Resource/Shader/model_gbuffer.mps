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

    float3 worldPos : WORLD_POS;
};

struct Material
{
    float fAlphaFactor;
    float bUseHeightMap;
    
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
[[vk::binding(6,0)]]Texture2D U_mat_texHeight;

struct PS_OUT
{
    float4 f3Base_fMetal: SV_Target0;
    float4 f3Albedo_fAmbientOcc: SV_Target1;
    float4 f3Normal_fRoughness: SV_Target2;
    float4 fDepth: SV_Target3;
};



float2 ParallaxMapping(float2 uv, float3 f3ViewDir, float fScale)
{ 
    float height =  U_mat_texHeight.Sample(U_defaultSampler, uv).r;    
    float2 p = f3ViewDir.xy / f3ViewDir.z * (height * fScale);
    return uv - p;    
}

PS_OUT PS(VS_OUT input) : SV_Target
{
    PS_OUT output;

    float2 uv = input.uv;

    //使用法线贴图，法向量在view space，CameraDir也在view space
    float3 T = normalize(input.tangent);
    float3 B = normalize(input.bitangent);
    float3 N = normalize(input.normal);
    float3x3 TBN = float3x3(T,B,N);

    if (U_mat.bUseHeightMap > 0)
    {
        float3 f3ViewDir = mul(U_f3CameraPosition, TBN) - mul(input.worldPos, TBN);
        f3ViewDir = normalize(f3ViewDir);
        uv = ParallaxMapping(uv, f3ViewDir, U_mat.bUseHeightMap);
    }

    float3 f3Normal = float3(0.0f, 0.0f, -1.0f);

    f3Normal = U_mat_texNormal.Sample(U_defaultSampler, uv).xyz;
    f3Normal = mul(f3Normal, TBN);
    f3Normal = normalize(f3Normal);
    f3Normal = (f3Normal + 1.0f) * 0.5f;


    float3 f3Albedo   = pow(U_mat_texAlbedo.Sample(U_defaultSampler, uv).rgb, float3(2.2));
    float fMetallic   = U_mat_texMetallic.Sample(U_defaultSampler, uv).r;
    float fRoughness  = U_mat_texRoughness.Sample(U_defaultSampler, uv).r;
    float fAmbientOcc = U_mat_texAmbientOcc.Sample(U_defaultSampler, uv).r;

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    output.f3Base_fMetal.rgb = lerp(float3(0.04), f3Albedo, fMetallic);
    output.f3Base_fMetal.a = 0.0f;




    output.f3Normal_fRoughness.rgb = f3Normal;
    output.f3Normal_fRoughness.a = fRoughness;

    output.f3Albedo_fAmbientOcc.rgb = f3Albedo;
    output.f3Albedo_fAmbientOcc.a = fAmbientOcc;

    output.fDepth = FloatToFloat4((input.depth - U_matZNearFar.x) / (U_matZNearFar.y - U_matZNearFar.x));

    return output;
}
    