implementing PostProcessModule;

public struct BloomData
{
    public float threshold;
    public float intensity;
    public float softThreshold;
}

public struct Bloom
{
    // Extract bright areas
    public static float3 Prefilter(float3 color, BloomData data)
    {
        float brightness = max(color.r, max(color.g, color.b));
        float knee = data.threshold * data.softThreshold;
        float soft = brightness - data.threshold + knee;
        soft = clamp(soft, 0.0, 2.0 * knee);
        soft = soft * soft / (4.0 * knee + 0.00001);
        float contribution = max(soft, brightness - data.threshold);
        contribution /= max(brightness, 0.00001);
        return color * contribution;
    }
    
    // Gaussian blur weights
    public static float3 DownsampleBox13(Texture2D tex, float2 uv, float2 texelSize)
    {
        float4 A = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(-1.0, -1.0));
        float4 B = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(0.0, -1.0));
        float4 C = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(1.0, -1.0));
        float4 D = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(-0.5, -0.5));
        float4 E = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(0.5, -0.5));
        float4 F = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(-1.0, 0.0));
        float4 G = tex.Sample(GlobalData.LinearSampler, uv);
        float4 H = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(1.0, 0.0));
        float4 I = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(-0.5, 0.5));
        float4 J = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(0.5, 0.5));
        float4 K = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(-1.0, 1.0));
        float4 L = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(0.0, 1.0));
        float4 M = tex.Sample(GlobalData.LinearSampler, uv + texelSize * float2(1.0, 1.0));

        float2 div = (1.0 / 4.0) * float2(0.5, 0.125);

        float3 result = (D.rgb + E.rgb + I.rgb + J.rgb) * div.x;
        result += (A.rgb + B.rgb + G.rgb + F.rgb) * div.y;
        result += (B.rgb + C.rgb + H.rgb + G.rgb) * div.y;
        result += (F.rgb + G.rgb + L.rgb + K.rgb) * div.y;
        result += (G.rgb + H.rgb + M.rgb + L.rgb) * div.y;

        return result;
    }
    
    // Upsample with tent filter
    public static float3 UpsampleTent(Texture2D tex, float2 uv, float2 texelSize, float sampleScale)
    {
        float4 d = texelSize.xyxy * float4(1.0, 1.0, -1.0, 0.0) * sampleScale;

        float3 s = tex.Sample(GlobalData.LinearSampler, uv - d.xy).rgb;
        s += tex.Sample(GlobalData.LinearSampler, uv - d.wy).rgb * 2.0;
        s += tex.Sample(GlobalData.LinearSampler, uv - d.zy).rgb;

        s += tex.Sample(GlobalData.LinearSampler, uv + d.zw).rgb * 2.0;
        s += tex.Sample(GlobalData.LinearSampler, uv).rgb * 4.0;
        s += tex.Sample(GlobalData.LinearSampler, uv + d.xw).rgb * 2.0;

        s += tex.Sample(GlobalData.LinearSampler, uv + d.zy).rgb;
        s += tex.Sample(GlobalData.LinearSampler, uv + d.wy).rgb * 2.0;
        s += tex.Sample(GlobalData.LinearSampler, uv + d.xy).rgb;

        return s * (1.0 / 16.0);
    }
}
