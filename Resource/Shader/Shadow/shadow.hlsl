#include "../Internal/internal_uniform_global.hlsl"
#include "../Shadow/PCSS.hlsl"


#define PCF_ENABLE false
#define PCSS_ENABLE true
#define CSM_TRANSITION true

float GetDirectionShadowFromCascadeLevel(
    Texture2DArray texShadowMap
    , float3 f3WorldPosition
    , uint nCascadeIndex
    , float fNdotL
){
    float4 f4DirLightSpacePos = mul(float4(f3WorldPosition, 1.0f), u_vLightProjectionMatrix[nCascadeIndex]);

    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (f4DirLightSpacePos.x / f4DirLightSpacePos.w * 0.5f);
    shadowTexCoords.y = 0.5f - (f4DirLightSpacePos.y / f4DirLightSpacePos.w * 0.5f);
    
	if (saturate(shadowTexCoords.x) == shadowTexCoords.x && saturate(shadowTexCoords.y) == shadowTexCoords.y)
    {     
        float fMargin = acos(saturate(fNdotL));
        float fEpsilon = clamp(fMargin / 1.570796, 0.001f, 0.003f);

        fEpsilon = fEpsilon / (nCascadeIndex + 1);

#if PCSS_ENABLE
        
        float fPixelDepth = min(f4DirLightSpacePos.z / f4DirLightSpacePos.w, 1.0f);
        float fLightRadiusNDCSpace = u_xDirectionalLight.fLightSize / u_vCascadeSplits[nCascadeIndex].z;
        float fLightPosNDCSpace = u_vCascadeSplits[nCascadeIndex].w;

        return PCSS(texShadowMap, nCascadeIndex, shadowTexCoords, fPixelDepth, fLightRadiusNDCSpace, fLightPosNDCSpace, fEpsilon);
    
#elif PCF_ENABLE

        float fPixelDepth = min(f4DirLightSpacePos.z / f4DirLightSpacePos.w, 1.0f);
        float fLightRadiusNDCSpace = u_xDirectionalLight.fLightSize / u_vCascadeSplits[nCascadeIndex].z;

        return PCF(texShadowMap, nCascadeIndex, shadowTexCoords, fPixelDepth, fLightRadiusNDCSpace, fEpsilon);

#else
        float fPixelDepth = min(f4DirLightSpacePos.z / f4DirLightSpacePos.w, 1.0f);
        float fShadowDepth = texShadowMap.Sample(NearestSampler, float3(shadowTexCoords.xy, nCascadeIndex)).r;
        //fShadowDepth < fPixelDepth + fEpsilon
        return 1.0f - step(fShadowDepth + fEpsilon, fPixelDepth);
#endif

    }

    return 1.0f;
}

//shadow
float GetDirectionShadow(Texture2DArray texShadowMap, float3 f3WorldPosition, float3 f3Normal, float3 f3LightInverseDirection)
{
    float fNdotL = dot(f3Normal, f3LightInverseDirection);

    float4 viewSpacePos = mul(float4(f3WorldPosition, 1.0f), u_matView);
    float fCameraDistance = viewSpacePos.z;

    // Get cascade index for the current fragment's view position
    uint nCascadeIndex = 0;

#if CSM_TRANSITION
    float fCascadeTransitionLerp = 1.0f;
#endif

    for(uint nSplitIdx = 0; nSplitIdx < CASCADED_SHADOW_MAP_NUM - 1; ++nSplitIdx)
    {
        if (fCameraDistance > u_vCascadeSplits[nSplitIdx].y)
        {
            nCascadeIndex = nSplitIdx + 1;
        }
#if CSM_TRANSITION
        else if (fCameraDistance > u_vCascadeSplits[nSplitIdx].x)
        {
            nCascadeIndex = nSplitIdx;
            fCascadeTransitionLerp = 1.0f - smoothstep(u_vCascadeSplits[nSplitIdx].x, u_vCascadeSplits[nSplitIdx].y, fCameraDistance);
        }
#endif
    }

    float fResult = GetDirectionShadowFromCascadeLevel(texShadowMap, f3WorldPosition, nCascadeIndex, fNdotL);
    
#if CSM_TRANSITION

    if (fCascadeTransitionLerp < 1.0f)
    {
        float fNextLevelShadow = GetDirectionShadowFromCascadeLevel(texShadowMap, f3WorldPosition, nCascadeIndex + 1, fNdotL);
        fResult = lerp(fNextLevelShadow, fResult, fCascadeTransitionLerp);
    }

#endif

    return fResult;
}
