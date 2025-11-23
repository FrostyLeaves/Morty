implementing PostProcessModule;

public struct ColorGradingData
{
    public float saturation;
    public float contrast;
    public float brightness;
    public float3 colorFilter;
    public float temperature;
    public float tint;
}

public struct ColorGrading
{
    // RGB to HSV conversion
    public static float3 RGBtoHSV(float3 rgb)
    {
        float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        float4 p = lerp(float4(rgb.bg, K.wz), float4(rgb.gb, K.xy), step(rgb.b, rgb.g));
        float4 q = lerp(float4(p.xyw, rgb.r), float4(rgb.r, p.yzx), step(p.x, rgb.r));
        
        float d = q.x - min(q.w, q.y);
        float e = 1.0e-10;
        return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    
    // HSV to RGB conversion
    public static float3 HSVtoRGB(float3 hsv)
    {
        float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        float3 p = abs(frac(hsv.xxx + K.xyz) * 6.0 - K.www);
        return hsv.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), hsv.y);
    }
    
    // White balance
    public static float3 WhiteBalance(float3 color, float temperature, float tint)
    {
        // Temperature
        float t1 = temperature / 65.0;
        float t2 = temperature < 0.0 ? 1.0 : 1.0;
        
        float3x3 LIN_2_LMS_MAT = float3x3(
            3.90405e-1, 5.49941e-1, 8.92632e-3,
            7.08416e-2, 9.63172e-1, 1.35775e-3,
            2.31082e-2, 1.28021e-1, 9.36245e-1
        );
        
        float3x3 LMS_2_LIN_MAT = float3x3(
            2.85847e+0, -1.62879e+0, -2.48910e-2,
            -2.10182e-1,  1.15820e+0,  3.24281e-4,
            -4.18120e-2, -1.18169e-1,  1.06867e+0
        );
        
        float3 lms = mul(color, LIN_2_LMS_MAT);
        lms *= float3(1.0 + temperature, 1.0, 1.0 - temperature);
        lms *= float3(1.0, 1.0 - tint, 1.0 + tint);
        
        return mul(lms, LMS_2_LIN_MAT);
    }
    
    // Apply color grading
    public static float3 Apply(float3 color, ColorGradingData data)
    {
        // White balance
        color = WhiteBalance(color, data.temperature, data.tint);
        
        // Brightness
        color *= data.brightness;
        
        // Contrast
        color = (color - 0.5) * data.contrast + 0.5;
        
        // Saturation
        float3 hsv = RGBtoHSV(color);
        hsv.y *= data.saturation;
        color = HSVtoRGB(hsv);
        
        // Color filter
        color *= data.colorFilter;
        
        return saturate(color);
    }
}
