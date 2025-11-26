implementing DeferredModule;

public struct DeferredUtil
{
    public static GBufferPixelData ComputePixel(SurfaceData surfaceData)
    {
        GBufferPixelData output;
        output.albedoMetalness.rgb = surfaceData.albedo;
        output.albedoMetalness.a = surfaceData.metalness;

        output.normalRoughness.rgb = surfaceData.normalWS;
        output.normalRoughness.a = surfaceData.roughness;

        output.ao.r = surfaceData.ao;

        return output;
    }
    
    // 从深度重建世界空间位置
    public static float3 ReconstructPositionWS(float depth, float2 screenUV, float4x4 invViewProj)
    {
        // NDC空间坐标
        float4 ndc;
        ndc.xy = screenUV * 2.0 - 1.0;
        ndc.y = -ndc.y; // 翻转Y轴（根据API调整）
        ndc.z = depth;
        ndc.w = 1.0;
        
        // 变换到世界空间
        float4 worldPos = mul(ndc, invViewProj);
        worldPos /= worldPos.w;
        
        return worldPos.xyz;
    }
    
    // 替代方法：使用视图射线重建（更高效）
    public static float3 ReconstructPositionWS_ViewRay(float depth, float2 screenUV, float3 cameraPos, float3 viewRayWS)
    {
        // viewRayWS 应该是从相机到远平面的归一化方向
        // depth 是线性深度或可以转换为线性深度
        return cameraPos + viewRayWS * depth;
    }
}