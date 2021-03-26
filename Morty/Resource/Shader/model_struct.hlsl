#include "private_header.hlsl"

struct VS_OUT_EMPTY
{
    float4 pos : SV_POSITION;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    
#if MCALC_NORMAL_IN_VS
    float3 normal : NORMAL;
    float3 dirLightDirTangentSpace : DIRLIGHT_TANGENT;
    float3 toCameraDirTangentSpace : CAMERADIR_TANGENT;

    float3 pointLightDirTangentSpace[MPOINT_LIGHT_MAX_NUMBER] : POINTLIGHT_TANGENT;
    float3 spotLightDirTangentSpace[MSPOT_LIGHT_MAX_NUMBER] : SPOTLIGHT_TANGENT;
#else
    float3 normal : NORMAL;
    float3 tangent : Tangent;
    float3 bitangent : BITANGENT;
#endif 

    float3 vertexPointLight : VERTEXX_POINT_LIGHT;

    float3 worldPos : POSITION;

    float4 dirLightSpacePos : DIRLIGHTSPACEPOS;
};

#ifdef MTRANSPARENT_DEPTH_PEELING

// PS   per render
[[vk::input_attachment_index(2)]] [[vk::binding(7, 1)]] SubpassInput U_texSubpassInput0;
[[vk::input_attachment_index(3)]] [[vk::binding(8, 1)]] SubpassInput U_texSubpassInput1;

#endif

//VS    per mesh
[[vk::binding(0,2)]]cbuffer _M_E_cbMeshMatrix : register(b0)
{
    float4x4 U_matWorld;
    float3x3 U_matNormal;
};

//VS    with bones
[[vk::binding(0,3)]]cbuffer _M_E_cbAnimation : register(b2)
{
    float4x4 U_vBonesMatrix[128];
};


////////////////////////////////////////////////////////////////

struct LightBasicInfo
{
    float3 f3Normal;
    float3 f3CameraDir;
    float3 f3DirLightDir;
};

LightBasicInfo GetLightBasicInfo(VS_OUT input)
{
    LightBasicInfo result;
    result.f3Normal = float3(0.0f, 0.0f, -1.0f);
    result.f3CameraDir = float3(0.0f, 0.0f, 1.0f);
    result.f3DirLightDir = float3(0.0f, 0.0f, 1.0f);
    

    if (U_mat.bUseNormalTex > 0)
    {
        result.f3Normal = U_mat_texNormal.Sample(U_defaultSampler, input.uv).xyz;
        result.f3Normal = result.f3Normal.rgb * 2.0f - 1.0f;
        result.f3Normal = normalize(result.f3Normal);
        
//如果在VS处理Normal
#if MCALC_NORMAL_IN_VS
        //使用法线贴图， 法向量在切线空间， CameraDir也在切线空间
        
        result.f3CameraDir = input.toCameraDirTangentSpace;
        result.f3DirLightDir = input.dirLightDirTangentSpace;
#else
        //使用法线贴图，法向量在view space，CameraDir也在view space

        float3 T = normalize(input.tangent);
        float3 B = normalize(input.bitangent);
        float3 N = normalize(input.normal);
        float3x3 TBN = float3x3(T,B,N);

        result.f3Normal = mul(result.f3Normal, TBN);
        result.f3CameraDir = normalize(U_f3CameraPosition - input.worldPos);
        result.f3DirLightDir = U_f3DirectionLight;
#endif
    }
    else
    {
        //没用法线贴图，法向量在view space，CameraDir也在view space
        result.f3Normal = input.normal;
        result.f3CameraDir = normalize(U_f3CameraPosition - input.worldPos);
        result.f3DirLightDir = U_f3DirectionLight;
    }

    return result;
}

//shadow
float CalcShadow(float4 dirLightSpacePos, float fNdotL)
{
    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (dirLightSpacePos.x / dirLightSpacePos.w * 0.5f);

    //DirectX Y 1 -> 0
    shadowTexCoords.y = 0.5f - (dirLightSpacePos.y / dirLightSpacePos.w * 0.5f);
    //Vulkan Y 0 -> 1
//    shadowTexCoords.y = 0.5f + (dirLightSpacePos.y / dirLightSpacePos.w * 0.5f);
    
	if (saturate(shadowTexCoords.x) == shadowTexCoords.x && saturate(shadowTexCoords.y) == shadowTexCoords.y)
    {     
        float margin = acos(saturate(fNdotL));
        float epsilon = clamp(margin / 1.570796, 0.001f, 0.003f);

        float lighting = 0.0f;
        
        float pixelDepth = dirLightSpacePos.z / dirLightSpacePos.w - epsilon;
        float shadowDepth = U_texShadowMap.Sample(U_defaultSampler, shadowTexCoords.xy);

        //current pixel is nearer.
        if (pixelDepth < shadowDepth)
            lighting += 1.0f;

            
        return lighting;
    }

    return 1.0f;
}
