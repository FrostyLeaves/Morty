implementing LightingModule;

__include BRDF;

public struct IBLData
{
    public float intensity;
    public float maxReflectionLod;

    public TextureCube irradianceMap;
    public TextureCube prefilterMap;
    public Texture2D brdfLUT;
}

public struct IBLSystem
{
    public float3 CalculateIBL(SurfaceData surfaceData, IBLData iblData)
    {
        float3 N = surfaceData.normalWS;
        float3 V = surfaceData.cameraDir;
        float3 R = reflect(-V, N);
        
        // Fresnel for IBL
        float3 F0 = lerp(float3(0.04, 0.04, 0.04), surfaceData.albedo, surfaceData.metalness);
        float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, surfaceData.roughness);
        
        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - surfaceData.metalness);
        
        // Diffuse IBL
        float3 irradiance = iblData.irradianceMap.Sample(GlobalData.LinearSampler, N).rgb;
        float3 diffuse = irradiance * surfaceData.albedo;

        // Specular IBL
        float3 prefilteredColor = iblData.prefilterMap.SampleLevel(GlobalData.LinearSampler, R, surfaceData.roughness * iblData.maxReflectionLod).rgb;
        float2 envBRDF = iblData.brdfLUT.Sample(GlobalData.LinearSampler, float2(max(dot(N, V), 0.0), surfaceData.roughness)).rg;
        float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
        
        float3 ambient = (kD * diffuse + specular) * surfaceData.ao * iblData.intensity;
        
        return ambient;
    }
}
