#include "inner_constant.hlsl"

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
        float margin = acos(saturate(fNdotL));
        float epsilon = clamp(margin / 1.570796, 0.001f, 0.003f);

        float pixelDepth = min(f4DirLightSpacePos.z / f4DirLightSpacePos.w, 1.0f);
        float shadowDepth = texShadowMap.Sample(NearestSampler, float3(shadowTexCoords.xy, nCascadeIndex)).r;

        //current pixel is nearer.
        if (pixelDepth < shadowDepth + epsilon )
        {
            return 1.0f;
        }

        return 0.0f;
    }

    return 1.0f;
}
