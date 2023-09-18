#include "Internal/internal_constant.hlsl"
#include "Internal/internal_functional.hlsl"
#include "Lighting/pbr_lighting.hlsl"
#include "Shadow/shadow.hlsl"

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

//Textures
[[vk::binding(0,0)]]Texture2D u_mat_f3Albedo_fMetallic;
[[vk::binding(1,0)]]Texture2D u_mat_f3Normal_fRoughness;
[[vk::binding(2,0)]]Texture2D u_mat_f3Position_fAmbientOcc;
[[vk::binding(3,0)]]Texture2D u_mat_DepthMap;


float3 GetWorldPosition(VS_OUT input, float fDepth)
{
    float2 uv = input.uv;
    uv.y = 1.0 - uv.y;

    float2 pos = input.uv * 2.0 - 1.0;
    
    float4 f4ViewportToWorldPos = mul(float4(pos.x, pos.y, u_matZNearFar.x, 1.0f), u_matCamProjInv);

    float3 f3WorldPosition = f4ViewportToWorldPos.xyz / f4ViewportToWorldPos.w;

    return u_f3CameraPosition + normalize(f3WorldPosition - u_f3CameraPosition) * fDepth;
}

float3 AdditionAllLights(VS_OUT input)
{
    float3 f3Color = float3(0.0f, 0.0f, 0.0f);

    float4 f3Albedo_fMetallic = u_mat_f3Albedo_fMetallic.Sample(NearestSampler, input.uv);
    float4 f3Normal_fRoughness = u_mat_f3Normal_fRoughness.Sample(NearestSampler, input.uv);
    float4 f3Position_fAmbientOcc = u_mat_f3Position_fAmbientOcc.Sample(NearestSampler, input.uv);
    float fDepth = u_mat_DepthMap.Sample(NearestSampler, input.uv).x;

    float3 f3Albedo = pow(f3Albedo_fMetallic.rgb, float3(2.2, 2.2, 2.2));
    float fMetallic = f3Albedo_fMetallic.a; 
    float3 f3Normal = f3Normal_fRoughness.rgb;
    float fRoughness = f3Normal_fRoughness.a;
    float fAmbientOcc = f3Position_fAmbientOcc.a;
    float3 f3WorldPosition = f3Position_fAmbientOcc.rgb;
    float3 f3CameraDir = normalize(u_f3CameraPosition - f3WorldPosition);

    float3 f3BaseColor = float3(0.04, 0.04, 0.04);
    f3BaseColor = lerp(f3BaseColor, f3Albedo, fMetallic);
    
    float3 f3Ambient = float3(0.0, 0.0, 0.0);

    if(u_bEnvironmentMapEnabled)
    {
        float3 kS = FresnelSchlickRoughness(max(dot(f3Normal, f3CameraDir), 0.0), f3BaseColor, fRoughness);
        float3 kD = (1.0f - kS) * (1.0f - fMetallic);

        float3 f3Irradiance = u_texIrradianceMap.SampleLevel(LinearSampler, f3Normal, 0).rgb;
        float3 f3Diffuse = f3Irradiance * f3Albedo;

        const float MAX_REFLECTION_LOD = 4.0f;
        float3 f3Reflect = reflect(-f3CameraDir, f3Normal); 
        float3 f3PrefilteredColor = u_texPrefilterMap.SampleLevel(LinearSampler, f3Reflect,  fRoughness * MAX_REFLECTION_LOD).rgb;    
        float2 brdf = u_texBrdfLUT.Sample(LinearSampler, float2(max(dot(f3Normal, f3CameraDir), 0.0), fRoughness)).rg;
        float3 f3Specular = f3PrefilteredColor * (kS * brdf.x + brdf.y);

        f3Ambient = (kD * f3Diffuse + f3Specular) * fAmbientOcc;
    }
    else
    {
        f3Ambient = float3(0.1, 0.1, 0.1) * f3Albedo * fAmbientOcc;
    }


    LightPointData pointData;
    pointData.f3CameraDir = f3CameraDir;
    pointData.f3Normal = f3Normal;
    pointData.f3WorldPosition = f3WorldPosition;
    pointData.f3BaseColor = f3BaseColor;
    pointData.f3Albedo = f3Albedo;
    pointData.fRoughness = fRoughness;
    pointData.fMetallic = fMetallic;


    if (u_bDirectionLightEnabled > 0)
    {
        float3 f3LightInverseDirection = -u_xDirectionalLight.f3LightDir;
        
        float shadow = GetDirectionShadow(u_texShadowMap, f3WorldPosition, f3Normal, f3LightInverseDirection);

        f3Color += shadow * AdditionDirectionLight(u_xDirectionalLight, pointData);
    }


    for(int nPointLightIdx = 0; nPointLightIdx < min(MPOINT_LIGHT_PIXEL_NUMBER, u_nValidPointLightsNumber); ++nPointLightIdx)
    {
        f3Color += AdditionPointLight(u_vPointLights[nPointLightIdx], pointData);
    }


    for(int nSpotLightIdx = 0; nSpotLightIdx < min(MSPOT_LIGHT_PIXEL_NUMBER, u_nValidSpotLightsNumber); ++nSpotLightIdx)
    {
        f3Color += AdditionSpotLight(u_vSpotLights[nSpotLightIdx], pointData);
    }


    f3Color = f3Color + f3Ambient;

    // HDR tonemapping
    //f3Color = f3Color / (f3Color + float3(1.0, 1.0, 1.0));
    // gamma correct
    //f3Color = pow(f3Color, float3(1.0/2.2, 1.0/2.2, 1.0/2.2)); 

    return f3Color;
}
