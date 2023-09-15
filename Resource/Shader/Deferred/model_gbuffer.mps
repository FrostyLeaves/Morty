#include "Internal/internal_constant.hlsl"
#include "Internal/internal_functional.hlsl"
#include "Internal/internal_model.hlsl"
#include "Deferred/pbr_material.hlsl"

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

struct PS_OUT
{
    float4 f3Albedo_fMetallic: SV_Target0;
    float4 f3Normal_fRoughness: SV_Target1;
    float4 f3Position_fAmbientOcc: SV_Target2;
};



float2 ParallaxMapping(float2 uv, float3 f3ViewDir, float fScale)
{ 
    const float fMinLayers = 8;
    const float fMaxLayers = 32;

    float fNumLayers = clamp(fMaxLayers, fMinLayers, abs(dot(float3(0.0f, 0.0f, 1.0f), f3ViewDir)));
    float fLayerDepth = 1.0f / fNumLayers;

    float fCurrentLayerDepth = 0.0f;

    float2 P = f3ViewDir.xy / f3ViewDir.z * fScale;
    float2 f2DeltaTexCoords = P / fNumLayers;

    float2 f2CurrentTexCoords = uv;
    float fCurrentDepthMapValue = u_mat_texHeight.Sample(LinearSampler, f2CurrentTexCoords).r;

    while(fCurrentLayerDepth < fCurrentDepthMapValue)
    {
        f2CurrentTexCoords -= f2DeltaTexCoords;
        fCurrentDepthMapValue = u_mat_texHeight.Sample(LinearSampler, f2CurrentTexCoords).r;

        fCurrentLayerDepth += fLayerDepth;
    }

    float2 f2PrevTexCoords = f2CurrentTexCoords + f2DeltaTexCoords;

    float fAfterDepth = fCurrentDepthMapValue - fCurrentLayerDepth;
    float fBeforeDepth = u_mat_texHeight.Sample(LinearSampler, f2PrevTexCoords).r - fCurrentLayerDepth + fLayerDepth;

    float fWeight = fAfterDepth / (fAfterDepth - fBeforeDepth);
    float2 f2FinalTexCoords = f2PrevTexCoords * fWeight + f2CurrentTexCoords * (1.0f - fWeight);

    return f2FinalTexCoords;
}

PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;

    float2 uv = input.uv;

    //使用法线贴图，法向量在view space，CameraDir也在view space
    float3 T = normalize(input.tangent);
    float3 B = normalize(input.bitangent);
    float3 N = normalize(input.normal);
    float3x3 TBN = float3x3(T,B,N);


    if (u_xMaterial.bUseHeightMap > 0)
    {
        float3 f3ViewDir = mul(u_f3CameraPosition, TBN) - mul(input.worldPos, TBN);
        f3ViewDir = normalize(f3ViewDir);
//        float3 f3ViewDir = mul(u_f3CameraDirection, TBN);
        uv = ParallaxMapping(uv, f3ViewDir, u_xMaterial.bUseHeightMap);
        uv = saturate(uv);
    }



    float3 f3Normal = float3(0.0f, 0.0f, 1.0f);
    f3Normal = u_texNormal.Sample(LinearSampler, uv).xyz;
    f3Normal = (f3Normal * 2.0f) - 1.0f;
    f3Normal = mul(f3Normal, TBN);
    f3Normal = normalize(f3Normal);

    float3 f3Albedo   = u_mat_texAlbedo.Sample(LinearSampler, uv).rgb;
    float fMetallic   = u_mat_texMetallic.Sample(LinearSampler, uv).r;
    float fRoughness  = u_mat_texRoughness.Sample(LinearSampler, uv).r;
    float fAmbientOcc = u_mat_texAmbientOcc.Sample(LinearSampler, uv).r;

    output.f3Albedo_fMetallic.rgb = f3Albedo * u_xMaterial.f4Albedo.rgb;
    output.f3Albedo_fMetallic.a = fMetallic * u_xMaterial.fMetallic;

    output.f3Normal_fRoughness.rgb = f3Normal;
    output.f3Normal_fRoughness.a = fRoughness * u_xMaterial.fRoughness;

    output.f3Position_fAmbientOcc.rgb = input.worldPos;
    output.f3Position_fAmbientOcc.a = fAmbientOcc;

    return output;
}