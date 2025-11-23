implementing LightingModule;

public struct ShadowData
{
    public float4x4 lightSpaceMatrix;
    public float2 shadowMapSize;
    public float bias;
    public float normalBias;
    public bool enablePCF;  // 是否启用PCF软阴影
}

public struct ShadowSystemConfig
{
    public Texture2D<float> shadowMap;
    public SamplerComparisonState shadowSampler;
    public ShadowData shadowData;
}

public struct ShadowSystem
{
    public Texture2D<float> shadowMap;
    public SamplerComparisonState shadowSampler;
    public ShadowData shadowData;

    // 统一的阴影计算接口 - 对表面数据和光源计算阴影
    public float ComputeShadow(SurfaceData surfaceData, LightData lightData)
    {
        // 只有平行光支持阴影（可以扩展到聚光灯和点光源）
        if (lightData.type != LightType.Directional)
        {
            return 1.0;
        }
        
        if (shadowData.enablePCF)
        {
            return CalculateShadowPCF(surfaceData.positionWS, surfaceData.normalWS, lightData.dir);
        }
        else
        {
            return CalculateShadow(surfaceData.positionWS, surfaceData.normalWS, lightData.dir);
        }
    }
    
    // 简化的接口 - 只需要位置和法线
    public float ComputeShadow(float3 positionWS, float3 normalWS, float3 lightDir)
    {
        if (shadowData.enablePCF)
        {
            return CalculateShadowPCF(positionWS, normalWS, lightDir);
        }
        else
        {
            return CalculateShadow(positionWS, normalWS, lightDir);
        }
    }

    // PCF (Percentage Closer Filtering) Shadow
    internal float CalculateShadowPCF(float3 positionWS, float3 normalWS, float3 lightDir)
    {
        // Transform to light space
        float4 positionLS = mul(float4(positionWS, 1.0), shadowData.lightSpaceMatrix);
        float3 projCoords = positionLS.xyz / positionLS.w;
        
        // Transform to [0,1] range
        projCoords.xy = projCoords.xy * 0.5 + 0.5;
        projCoords.y = 1.0 - projCoords.y;
        
        // Check if outside shadow map bounds
        if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
            projCoords.y < 0.0 || projCoords.y > 1.0 ||
            projCoords.z > 1.0)
        {
            return 1.0;
        }
        
        // Apply bias
        float NdotL = max(dot(normalWS, -lightDir), 0.0);
        float bias = max(shadowData.bias * (1.0 - NdotL), shadowData.normalBias);
        float currentDepth = projCoords.z - bias;
        
        // PCF filtering
        float shadow = 0.0;
        float2 texelSize = 1.0 / shadowData.shadowMapSize;
        
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                float2 offset = float2(x, y) * texelSize;
                shadow += shadowMap.SampleCmpLevelZero(shadowSampler, projCoords.xy + offset, currentDepth);
            }
        }
        shadow /= 9.0;
        
        return shadow;
    }
    
    // Simple shadow calculation
    internal float CalculateShadow(float3 positionWS, float3 normalWS, float3 lightDir)
    {
        float4 positionLS = mul(float4(positionWS, 1.0), shadowData.lightSpaceMatrix);
        float3 projCoords = positionLS.xyz / positionLS.w;
        
        projCoords.xy = projCoords.xy * 0.5 + 0.5;
        projCoords.y = 1.0 - projCoords.y;
        
        if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
            projCoords.y < 0.0 || projCoords.y > 1.0 ||
            projCoords.z > 1.0)
        {
            return 1.0;
        }
        
        float NdotL = max(dot(normalWS, -lightDir), 0.0);
        float bias = max(shadowData.bias * (1.0 - NdotL), shadowData.normalBias);
        float currentDepth = projCoords.z - bias;
        
        return shadowMap.SampleCmpLevelZero(shadowSampler, projCoords.xy, currentDepth);
    }
}
