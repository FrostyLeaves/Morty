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
    float4 f3Albedo_fMetallic: SV_Target0;
    float4 f2Normal_fRoughness_fAO: SV_Target1;

#if GBUFFER_UNIFIED_FORMAT
    float4 f4Depth : SV_Target2;
#else
    float fDepth : SV_TARGET2;
#endif
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


    float3 f3Albedo   = U_mat_texAlbedo.Sample(U_defaultSampler, uv).rgb;
    float fMetallic   = U_mat_texMetallic.Sample(U_defaultSampler, uv).r;
    float fRoughness  = U_mat_texRoughness.Sample(U_defaultSampler, uv).r;
    float fAmbientOcc = U_mat_texAmbientOcc.Sample(U_defaultSampler, uv).r;

    output.f3Albedo_fMetallic.rgb = f3Albedo;
    output.f3Albedo_fMetallic.a = fMetallic;

    output.f2Normal_fRoughness_fAO.rg = f3Normal.rg;
    output.f2Normal_fRoughness_fAO.b = fRoughness;
    output.f2Normal_fRoughness_fAO.a = fAmbientOcc;

#if GBUFFER_UNIFIED_FORMAT
    output.f4Depth = FloatToFloat4((input.depth - U_matZNearFar.x) / (U_matZNearFar.y - U_matZNearFar.x));
#else
    output.fDepth = input.depth;
#endif

    return output;
}
    