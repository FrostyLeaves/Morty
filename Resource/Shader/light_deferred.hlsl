#include "inner_constant.hlsl"
#include "inner_functional.hlsl"
#include "brdf_functional.hlsl"
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



// spot light
float3 AdditionSpotLight(SpotLight spotLight, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3WorldPixelPosition, float3 f3BaseColor, float3 f3Albedo, float fRoughness, float fMetallic)
{
    float fTheta = dot(spotLight.f3Direction, -f3LightDir);
    if (fTheta > spotLight.fHalfOuterCutOff)
    {
        float fDiff = max(dot(f3Normal, f3LightDir), 0.0f);
        float3 fReflectDir = reflect(-f3LightDir, f3Normal);

        float fEpsilon = spotLight.fHalfInnerCutOff - spotLight.fHalfOuterCutOff;
        float fIntensity = clamp((fTheta - spotLight.fHalfOuterCutOff) / fEpsilon, 0.0, 1.0);

        float3 f3LightColor = spotLight.f3Intensity * fIntensity;
                    
        return BRDF(f3LightColor, f3CameraDir, f3LightDir, f3Normal, f3BaseColor, f3Albedo, fRoughness, fMetallic);
    }
    else
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
}

// point light
float3 AdditionPointLight(PointLight pointLight, float3 worldPos, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3BaseColor, float3 f3Albedo, float fRoughness, float fMetallic)
{
    float fDistance = length(pointLight.f3WorldPosition - worldPos);
    float fAttenuation = 1.0f / (1.0f + pointLight.fLinear * fDistance + pointLight.fQuadratic * fDistance * fDistance);

    float3 f3LightColor = pointLight.f3Intensity * fAttenuation;

    return BRDF(f3LightColor, f3CameraDir, f3LightDir, f3Normal, f3BaseColor, f3Albedo, fRoughness, fMetallic);
}

// point light
float3 AdditionDirectionLight(DirectionLight dirLight, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3BaseColor, float3 f3Albedo, float fRoughness, float fMetallic)
{
    float fNdotL = dot(f3Normal, -f3LightDir);

    if (fNdotL >= 0)
    {
        float3 f3LightColor = dirLight.f3Intensity;

        return BRDF(f3LightColor, f3CameraDir, f3LightDir, f3Normal, f3BaseColor, f3Albedo, fRoughness, fMetallic);
    }

    return float3(0, 0, 0);
}

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
        float3 kS = FresnelSchlick(max(dot(f3Normal, f3CameraDir), 0.0), f3BaseColor);
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




    if (u_bDirectionLightEnabled > 0)
    {
        float3 f3DirLightDir = u_f3DirectionLight;
        
        float shadow = GetDirectionShadow(u_texShadowMap, f3WorldPosition, f3Normal, -f3DirLightDir);

        f3Color += shadow * AdditionDirectionLight(  u_xDirectionalLight, f3CameraDir, f3DirLightDir, f3Normal,
                                        f3BaseColor, f3Albedo, fRoughness, fMetallic );
    }


    for(int nPointLightIdx = 0; nPointLightIdx < min(MPOINT_LIGHT_PIXEL_NUMBER, u_nValidPointLightsNumber); ++nPointLightIdx)
    {
        float3 f3LightDir = normalize(u_vPointLights[nPointLightIdx].f3WorldPosition - f3WorldPosition);

        f3Color += AdditionPointLight(  u_vPointLights[nPointLightIdx], f3WorldPosition, f3CameraDir, f3LightDir,
                                    f3Normal, f3BaseColor, f3Albedo, fRoughness, fMetallic );
    }


    for(int nSpotLightIdx = 0; nSpotLightIdx < min(MSPOT_LIGHT_PIXEL_NUMBER, u_nValidSpotLightsNumber); ++nSpotLightIdx)
    {
        float3 f3LightDir = normalize(u_vSpotLights[nSpotLightIdx].f3WorldPosition - f3WorldPosition);
        f3Color += AdditionSpotLight(   u_vSpotLights[nSpotLightIdx], f3CameraDir, f3LightDir, f3Normal,
                                    f3WorldPosition, f3BaseColor, f3Albedo, fRoughness, fMetallic );
    }


    f3Color = f3Color + f3Ambient;


    // HDR tonemapping
    f3Color = f3Color / (f3Color + float3(1.0, 1.0, 1.0));
    // gamma correct
    f3Color = pow(f3Color, float3(1.0/2.2, 1.0/2.2, 1.0/2.2)); 

    return f3Color;
}
