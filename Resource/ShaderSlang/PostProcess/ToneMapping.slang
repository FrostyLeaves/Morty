implementing PostProcessModule;

public enum ToneMappingMode : uint8_t
{
    None = 0,
    Reinhard,
    ReinhardLuminance,
    Uncharted2,
    ACES,
    ACESFitted
}

public struct ToneMappingData
{
    public ToneMappingMode mode;
    public float exposure;
    public float gamma;
}

public struct ToneMapping
{
    // Reinhard tone mapping
    public static float3 Reinhard(float3 color)
    {
        return color / (1.0 + color);
    }
    
    // Reinhard with luminance
    public static float3 ReinhardLuminance(float3 color)
    {
        float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
        float toneMappedLuma = luma / (1.0 + luma);
        return color * (toneMappedLuma / luma);
    }
    
    // Uncharted 2 tone mapping
    public static float3 Uncharted2Tonemap(float3 x)
    {
        float A = 0.15;
        float B = 0.50;
        float C = 0.10;
        float D = 0.20;
        float E = 0.02;
        float F = 0.30;
        return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
    }
    
    public static float3 Uncharted2(float3 color)
    {
        float W = 11.2;
        float exposureBias = 2.0;
        float3 curr = Uncharted2Tonemap(exposureBias * color);
        float3 whiteScale = 1.0 / Uncharted2Tonemap(float3(W, W, W));
        return curr * whiteScale;
    }
    
    // ACES tone mapping
    public static float3 ACESFilm(float3 x)
    {
        float a = 2.51;
        float b = 0.03;
        float c = 2.43;
        float d = 0.59;
        float e = 0.14;
        return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
    }
    
    // ACES Fitted (more accurate)
    public static float3 ACESFitted(float3 color)
    {
        float3x3 ACESInputMat = float3x3(
            0.59719, 0.35458, 0.04823,
            0.07600, 0.90834, 0.01566,
            0.02840, 0.13383, 0.83777
        );
        
        float3x3 ACESOutputMat = float3x3(
            1.60475, -0.53108, -0.07367,
            -0.10208, 1.10813, -0.00605,
            -0.00327, -0.07276, 1.07602
        );
        
        color = mul(color, ACESInputMat);
        
        // RRT and ODT fit
        float3 a = color * (color + 0.0245786) - 0.000090537;
        float3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
        color = a / b;
        
        color = mul(color, ACESOutputMat);
        return saturate(color);
    }
    
    // Apply tone mapping
    public static float3 Apply(float3 color, ToneMappingData data)
    {
        // Apply exposure
        color *= data.exposure;
        
        // Apply tone mapping
        switch (data.mode)
        {
            case ToneMappingMode.Reinhard:
                color = Reinhard(color);
                break;
            case ToneMappingMode.ReinhardLuminance:
                color = ReinhardLuminance(color);
                break;
            case ToneMappingMode.Uncharted2:
                color = Uncharted2(color);
                break;
            case ToneMappingMode.ACES:
                color = ACESFilm(color);
                break;
            case ToneMappingMode.ACESFitted:
                color = ACESFitted(color);
                break;
            default:
                break;
        }
        
        // Gamma correction
        color = pow(color, float3(1.0 / data.gamma, 1.0 / data.gamma, 1.0 / data.gamma));
        
        return color;
    }
}
