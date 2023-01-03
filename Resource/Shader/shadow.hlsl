#include "inner_constant.hlsl"
#include "Shadow/PCSS.hlsl"


#define PCF_ENABLE true
#define PCSS_ENABLE true


//shadow
float GetDirectionShadow(Texture2DArray texShadowMap, float3 f3WorldPosition, float3 f3Normal, float3 f3LightInverseDirection)
{
    float fNdotL = dot(f3Normal, f3LightInverseDirection);

    float4 viewSpacePos = mul(float4(f3WorldPosition, 1.0f), u_matView);

    // Get cascade index for the current fragment's view position
    uint nCascadeIndex = 0;
    for(uint nSplitIdx = 0; nSplitIdx < CASCADED_SHADOW_MAP_NUM - 1; ++nSplitIdx)
    {
        if (viewSpacePos.z > u_vCascadeSplits[nSplitIdx])
        {
            nCascadeIndex = nSplitIdx + 1;
        }
    }

    float4 f4DirLightSpacePos = mul(float4(f3WorldPosition, 1.0f), u_vLightProjectionMatrix[nCascadeIndex]);

    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (f4DirLightSpacePos.x / f4DirLightSpacePos.w * 0.5f);
    shadowTexCoords.y = 0.5f - (f4DirLightSpacePos.y / f4DirLightSpacePos.w * 0.5f);
    
	if (saturate(shadowTexCoords.x) == shadowTexCoords.x && saturate(shadowTexCoords.y) == shadowTexCoords.y)
    {     
        float fMargin = acos(saturate(fNdotL));
        float fEpsilon = clamp(fMargin / 1.570796, 0.001f, 0.003f);

        fEpsilon = fEpsilon / (nCascadeIndex + 1);

        float fPixelDepth = min(f4DirLightSpacePos.z / f4DirLightSpacePos.w, 1.0f);
       
        float fLightSize = u_xDirectionalLight.fLightSize;
    
#if PCSS_ENABLE

        return PCSS(texShadowMap, nCascadeIndex, shadowTexCoords, fLightSize, f4DirLightSpacePos, fPixelDepth, fEpsilon);
    
#elif PCF_ENABLE

        return PCF(texShadowMap, nCascadeIndex, shadowTexCoords, fPixelDepth, 5.0f, fEpsilon);

#else
        float fPixelDepth = texShadowMap.Sample(NearestSampler, float3(shadowTexCoords.xy, nCascadeIndex)).r;
        
        //pixelDepth < fPixelDepth + fEpsilon
        return step(pixelDepth, fPixelDepth + fEpsilon);
#endif

    }

    return 1.0f;
}
