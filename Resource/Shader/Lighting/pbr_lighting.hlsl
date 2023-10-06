#include "Internal/internal_constant.hlsl"
#include "Internal/internal_functional.hlsl"
#include "Lighting/brdf_functional.hlsl"

// spot light
float3 AdditionSpotLight(SpotLight spotLight, LightPointData pointData)
{
    float3 f3LightDir = normalize(spotLight.f3WorldPosition - pointData.f3WorldPosition);

    float fTheta = dot(spotLight.f3Direction, -f3LightDir);
    if (fTheta > spotLight.fHalfOuterCutOff)
    {
        float fDiff = max(dot(pointData.f3Normal, f3LightDir), 0.0f);
        float3 fReflectDir = reflect(-f3LightDir, pointData.f3Normal);

        float fEpsilon = spotLight.fHalfInnerCutOff - spotLight.fHalfOuterCutOff;
        float fIntensity = clamp((fTheta - spotLight.fHalfOuterCutOff) / fEpsilon, 0.0, 1.0);

        float3 f3LightColor = spotLight.f3Intensity * fIntensity;
                    
        return BRDF(f3LightColor, pointData.f3CameraDir, f3LightDir, pointData.f3Normal, pointData.f3BaseColor, pointData.f3Albedo, pointData.fRoughness, pointData.fMetallic);
    }
    else
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
}

// point light
float3 AdditionPointLight(PointLight pointLight, LightPointData pointData)
{
    float3 f3LightDir = normalize(pointLight.f3WorldPosition - pointData.f3WorldPosition);

    float fDistance = length(pointLight.f3WorldPosition - pointData.f3WorldPosition);
    float fAttenuation = 1.0f / (1.0f + pointLight.fLinear * fDistance + pointLight.fQuadratic * fDistance * fDistance);

    float3 f3LightColor = pointLight.f3Intensity * fAttenuation;

    return BRDF(f3LightColor, pointData.f3CameraDir, f3LightDir, pointData.f3Normal, pointData.f3BaseColor, pointData.f3Albedo, pointData.fRoughness, pointData.fMetallic);
}

// direction light
float3 AdditionDirectionLight(DirectionLight dirLight, LightPointData pointData)
{
    float fNdotL = dot(pointData.f3Normal, -dirLight.f3LightDir);

    if (fNdotL >= 0)
    {
        float3 f3LightColor = dirLight.f3Intensity;

        return BRDF(f3LightColor, pointData.f3CameraDir, dirLight.f3LightDir, pointData.f3Normal, pointData.f3BaseColor, pointData.f3Albedo, pointData.fRoughness, pointData.fMetallic);
    }

    return float3(0, 0, 0);
}


float3 PbrLighting()
{
    
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