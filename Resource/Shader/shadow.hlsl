#include "inner_constant.hlsl"

//shadow
float CalcShadow(Texture2DArray texShadowMap, float3 f3WorldPosition, float fDepth, float fNdotL)
{
    float4 viewSpacePos = mul(float4(f3WorldPosition, 1.0f), U_matView);

    // Get cascade index for the current fragment's view position
    uint nCascadeIndex = 0;
    for(uint nSplitIdx = 0; nSplitIdx < CASCADED_SHADOW_MAP_NUM - 1; ++nSplitIdx)
    {
        if (viewSpacePos.z > U_vCascadeSplits[nSplitIdx])
        {
            nCascadeIndex = nSplitIdx + 1;
        }
    }
    
    float4 f4DirLightSpacePos = mul(float4(f3WorldPosition, 1.0f), U_matLightProj[nCascadeIndex]);

    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (f4DirLightSpacePos.x / f4DirLightSpacePos.w * 0.5f);

    //DirectX Y 1 -> 0
    shadowTexCoords.y = 0.5f - (f4DirLightSpacePos.y / f4DirLightSpacePos.w * 0.5f);
    //Vulkan Y 0 -> 1
//    shadowTexCoords.y = 0.5f + (f4DirLightSpacePos.y / f4DirLightSpacePos.w * 0.5f);
    
	if (saturate(shadowTexCoords.x) == shadowTexCoords.x && saturate(shadowTexCoords.y) == shadowTexCoords.y)
    {     
        float margin = acos(saturate(fNdotL));
        float epsilon = clamp(margin / 1.570796, 0.001f, 0.003f);

        float lighting = 0.0f;
        
        float pixelDepth = min(f4DirLightSpacePos.z / f4DirLightSpacePos.w, 1.0f);
        float shadowDepth = texShadowMap.Sample(LinearSampler, float3(shadowTexCoords.xy, nCascadeIndex)).r;

        //current pixel is nearer.
        if (pixelDepth < shadowDepth + epsilon)
            lighting += 1.0f;

            
        return lighting;
    }

    return 1.0f;
}
