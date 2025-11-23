implementing ForwardModule;

public struct ForwardLighting
{
    public static float3 ComputeForwardLighting(SurfaceData surfaceData, int maxLights)
    {
        float3 totalLighting = float3(0.0, 0.0, 0.0);
        
        // Iterate through lights
        int lightCount = min(maxLights, min(lightSystemInfo.lightNum, allLightData.getCount()));
        
        for (int i = 0; i < lightCount; ++i)
        {
            LightData lightData = allLightData[i];
            
            float3 lighting = float3(0.0, 0.0, 0.0);
            
            switch (lightData.type)
            {
                case LightType.Directional:
                {
                    float3 lightDir = lightData.dir;
                    float NdotL = max(dot(surfaceData.normalWS, -lightDir), 0.0);
                    
                    if (NdotL > 0.0)
                    {
                        lighting = BRDF(lightData.intensity, surfaceData.cameraDir, lightDir, 
                                      surfaceData.normalWS, surfaceData.albedo, 
                                      surfaceData.roughness, surfaceData.metalness);
                    }
                    break;
                }
                
                case LightType.Point:
                {
                    float3 lightDir = normalize(lightData.positionWS - surfaceData.positionWS);
                    float distance = length(lightData.positionWS - surfaceData.positionWS);
                    
                    // Attenuation
                    float constant = lightData.extension.x;
                    float linear = lightData.extension.y;
                    float quadratic = lightData.extension.z;
                    float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);
                    
                    float3 attenuatedIntensity = lightData.intensity * attenuation;
                    
                    lighting = BRDF(attenuatedIntensity, surfaceData.cameraDir, lightDir,
                                  surfaceData.normalWS, surfaceData.albedo,
                                  surfaceData.roughness, surfaceData.metalness);
                    break;
                }
                
                case LightType.Spot:
                {
                    float3 lightDir = normalize(lightData.positionWS - surfaceData.positionWS);
                    float distance = length(lightData.positionWS - surfaceData.positionWS);
                    
                    // Spot cone
                    float theta = dot(lightData.dir, -lightDir);
                    float innerCutoff = lightData.extension.x;
                    float outerCutoff = lightData.extension.y;
                    float epsilon = innerCutoff - outerCutoff;
                    float intensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);
                    
                    if (intensity > 0.0)
                    {
                        // Attenuation
                        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
                        float3 attenuatedIntensity = lightData.intensity * attenuation * intensity;
                        
                        lighting = BRDF(attenuatedIntensity, surfaceData.cameraDir, lightDir,
                                      surfaceData.normalWS, surfaceData.albedo,
                                      surfaceData.roughness, surfaceData.metalness);
                    }
                    break;
                }
            }
            
            totalLighting += lighting;
        }
        
        return totalLighting;
    }
}
