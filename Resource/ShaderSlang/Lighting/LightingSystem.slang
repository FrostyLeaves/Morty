implementing LightingModule;

__include BRDF;

internal struct DirectionalLight
{
    internal static float3 ComputeLighting(SurfaceData surfaceData, LightData lightData)
    {
        float3 output = 0;

        float fNdotL = dot(surfaceData.normalWS, -lightData.dir);

        if (fNdotL >= 0)
        {
            float3 lightColor = lightData.intensity;

            output = BRDF(lightColor, surfaceData.cameraDir, lightData.dir, surfaceData.normalWS, surfaceData.albedo, surfaceData.roughness, surfaceData.metalness);
        }

        return output;
    }
}

internal struct SpotLight
{
    internal static float3 ComputeLighting(SurfaceData surfaceData, LightData lightData)
    {
        float3 lightDir = normalize(lightData.positionWS - surfaceData.positionWS);

        float fTheta = dot(lightData.dir, -lightDir);
        if (fTheta > lightData.extension.y)
        {
            float fEpsilon = lightData.extension.x - lightData.extension.y;
            float intensityCutOff = clamp((fTheta - lightData.extension.y) / fEpsilon, 0.0, 1.0);

            float3 lightColor = lightData.intensity * intensityCutOff;
            return BRDF(lightColor, surfaceData.cameraDir, lightDir, surfaceData.normalWS, surfaceData.albedo, surfaceData.roughness, surfaceData.metalness);
        }
        else
        {
            return float3(0.0f, 0.0f, 0.0f);
        }
    }
}

internal struct PointLight
{
    internal static float3 ComputeLighting(SurfaceData surfaceData, LightData lightData)
    {
        const float3 lightDir = normalize(lightData.positionWS - surfaceData.positionWS);

        const float distance = length(lightData.positionWS - surfaceData.positionWS);
        const float linear = lightData.extension.y;
        const float quadratic = lightData.extension.z;
        const float attenuation = 1.0f / (1.0f + linear * distance + quadratic * distance * distance);

        const float3 lightColor = lightData.intensity * attenuation;
        return BRDF(lightColor, surfaceData.cameraDir, lightDir, surfaceData.normalWS, surfaceData.albedo, surfaceData.roughness, surfaceData.metalness);
    }
}

public struct LightSystemInfo
{
    int lightNum = 0;
}

public struct LightingSystem
{
    public LightingOutput ComputeLighting(SurfaceData surfaceData)
    {
        LightingOutput output;
        output.color = 0;

        for (int lightIdx = 0; lightIdx < min(lightSystemInfo.lightNum, allLightData.getCount()); ++lightIdx)
        {
            var lightData = allLightData[lightIdx];

            if (lightData.type == LightType.Directional)
            {
                output.color += DirectionalLight.ComputeLighting(surfaceData, lightData);
            }
            else if (lightData.type == LightType.Spot)
            {
                output.color += SpotLight.ComputeLighting(surfaceData, lightData);
            }
            else if (lightData.type == LightType.Point)
            {
                output.color += PointLight.ComputeLighting(surfaceData, lightData);
            }
        }

        return output;
    }
}