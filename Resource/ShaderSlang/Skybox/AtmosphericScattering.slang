implementing SkyboxModule;

public struct AtmosphericData
{
    public float3 sunDirection;
    public float sunIntensity;
    public float3 rayleighScattering;
    public float mieScattering;
    public float rayleighScaleHeight;
    public float mieScaleHeight;
    public float planetRadius;
    public float atmosphereRadius;
    public int numSamples;
    public int numOpticalDepthSamples;
}

public struct AtmosphericScattering
{
    // Rayleigh phase function
    public static float PhaseRayleigh(float cosTheta)
    {
        return (3.0 / (16.0 * PI)) * (1.0 + cosTheta * cosTheta);
    }
    
    // Mie phase function (Henyey-Greenstein)
    public static float PhaseMie(float cosTheta, float g)
    {
        float g2 = g * g;
        return (1.0 / (4.0 * PI)) * ((1.0 - g2) / pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
    }
    
    // Ray-sphere intersection
    public static bool RaySphereIntersection(float3 rayOrigin, float3 rayDir, float3 sphereCenter, float sphereRadius, out float t0, out float t1)
    {
        float3 oc = rayOrigin - sphereCenter;
        float b = 2.0 * dot(rayDir, oc);
        float c = dot(oc, oc) - sphereRadius * sphereRadius;
        float discriminant = b * b - 4.0 * c;
        
        if (discriminant < 0.0)
        {
            t0 = 0.0;
            t1 = 0.0;
            return false;
        }
        
        float sqrtDiscriminant = sqrt(discriminant);
        t0 = (-b - sqrtDiscriminant) / 2.0;
        t1 = (-b + sqrtDiscriminant) / 2.0;
        
        return true;
    }
    
    // Calculate optical depth
    public static float2 CalculateOpticalDepth(float3 rayOrigin, float3 rayDir, float rayLength, AtmosphericData data, float3 planetCenter)
    {
        float3 densityPoint = rayOrigin;
        float stepSize = rayLength / float(data.numOpticalDepthSamples - 1);
        float2 opticalDepth = float2(0.0, 0.0);
        
        for (int i = 0; i < data.numOpticalDepthSamples; ++i)
        {
            float height = length(densityPoint - planetCenter) - data.planetRadius;
            float localDensityRayleigh = exp(-height / data.rayleighScaleHeight);
            float localDensityMie = exp(-height / data.mieScaleHeight);
            
            opticalDepth.x += localDensityRayleigh * stepSize;
            opticalDepth.y += localDensityMie * stepSize;
            
            densityPoint += rayDir * stepSize;
        }
        
        return opticalDepth;
    }
    
    // Calculate atmospheric scattering
    public static float3 CalculateScattering(float3 rayOrigin, float3 rayDir, float maxDist, AtmosphericData data)
    {
        float3 planetCenter = float3(0.0, -data.planetRadius, 0.0);
        
        // Intersect with atmosphere
        float t0, t1;
        if (!RaySphereIntersection(rayOrigin, rayDir, planetCenter, data.atmosphereRadius, t0, t1))
        {
            return float3(0.0, 0.0, 0.0);
        }
        
        // Intersect with planet
        float tPlanet0, tPlanet1;
        bool hitPlanet = RaySphereIntersection(rayOrigin, rayDir, planetCenter, data.planetRadius, tPlanet0, tPlanet1);
        
        if (hitPlanet && tPlanet0 > 0.0)
        {
            t1 = min(t1, tPlanet0);
        }
        
        t0 = max(0.0, t0);
        t1 = min(t1, maxDist);
        
        float rayLength = t1 - t0;
        float stepSize = rayLength / float(data.numSamples - 1);
        
        float3 scatteringRayleigh = float3(0.0, 0.0, 0.0);
        float3 scatteringMie = float3(0.0, 0.0, 0.0);
        
        float2 opticalDepth = float2(0.0, 0.0);
        
        for (int i = 0; i < data.numSamples; ++i)
        {
            float t = t0 + stepSize * float(i);
            float3 samplePoint = rayOrigin + rayDir * t;
            
            float height = length(samplePoint - planetCenter) - data.planetRadius;
            
            float localDensityRayleigh = exp(-height / data.rayleighScaleHeight);
            float localDensityMie = exp(-height / data.mieScaleHeight);
            
            opticalDepth.x += localDensityRayleigh * stepSize;
            opticalDepth.y += localDensityMie * stepSize;
            
            // Calculate optical depth to sun
            float tSun0, tSun1;
            RaySphereIntersection(samplePoint, data.sunDirection, planetCenter, data.atmosphereRadius, tSun0, tSun1);
            float rayLengthSun = tSun1;
            
            float2 opticalDepthSun = CalculateOpticalDepth(samplePoint, data.sunDirection, rayLengthSun, data, planetCenter);
            
            float2 totalOpticalDepth = opticalDepth + opticalDepthSun;
            
            float3 attenuationRayleigh = exp(-data.rayleighScattering * totalOpticalDepth.x);
            float3 attenuationMie = exp(-data.mieScattering * totalOpticalDepth.y);
            
            scatteringRayleigh += attenuationRayleigh * localDensityRayleigh * stepSize;
            scatteringMie += attenuationMie * localDensityMie * stepSize;
        }
        
        float cosTheta = dot(rayDir, data.sunDirection);
        float phaseRayleigh = PhaseRayleigh(cosTheta);
        float phaseMie = PhaseMie(cosTheta, 0.76);
        
        float3 totalScattering = scatteringRayleigh * data.rayleighScattering * phaseRayleigh +
                                scatteringMie * data.mieScattering * phaseMie;
        
        return totalScattering * data.sunIntensity;
    }
}
