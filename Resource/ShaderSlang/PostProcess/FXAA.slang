implementing PostProcessModule;

public struct FXAAData
{
    public float2 texelSize;
    public float contrastThreshold;
    public float relativeThreshold;
    public float subpixelBlending;
}

public struct FXAA
{
    public static float rgb2luma(float3 rgb)
    {
        return sqrt(dot(rgb, float3(0.299, 0.587, 0.114)));
    }
    
    public static float3 Apply(Texture2D tex, float2 uv, FXAAData data)
    {
        float3 colorCenter = tex.Sample(LinearSampler, uv).rgb;
        
        // Sample neighbor pixels
        float lumaCenter = rgb2luma(colorCenter);
        float lumaDown = rgb2luma(tex.Sample(LinearSampler, uv, int2(0, -1)).rgb);
        float lumaUp = rgb2luma(tex.Sample(LinearSampler, uv, int2(0, 1)).rgb);
        float lumaLeft = rgb2luma(tex.Sample(LinearSampler, uv, int2(-1, 0)).rgb);
        float lumaRight = rgb2luma(tex.Sample(LinearSampler, uv, int2(1, 0)).rgb);
        
        // Find min and max luma
        float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
        float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));
        
        // Compute contrast
        float lumaRange = lumaMax - lumaMin;
        
        // Skip if contrast is too low
        if (lumaRange < max(data.contrastThreshold, lumaMax * data.relativeThreshold))
        {
            return colorCenter;
        }
        
        // Sample corner pixels
        float lumaDownLeft = rgb2luma(tex.Sample(LinearSampler, uv, int2(-1, -1)).rgb);
        float lumaUpRight = rgb2luma(tex.Sample(LinearSampler, uv, int2(1, 1)).rgb);
        float lumaUpLeft = rgb2luma(tex.Sample(LinearSampler, uv, int2(-1, 1)).rgb);
        float lumaDownRight = rgb2luma(tex.Sample(LinearSampler, uv, int2(1, -1)).rgb);
        
        // Combine
        float lumaDownUp = lumaDown + lumaUp;
        float lumaLeftRight = lumaLeft + lumaRight;
        
        float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
        float lumaDownCorners = lumaDownLeft + lumaDownRight;
        float lumaRightCorners = lumaDownRight + lumaUpRight;
        float lumaUpCorners = lumaUpRight + lumaUpLeft;
        
        // Compute edge direction
        float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + 
                              abs(-2.0 * lumaCenter + lumaDownUp) * 2.0 + 
                              abs(-2.0 * lumaRight + lumaRightCorners);
        float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) + 
                            abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 + 
                            abs(-2.0 * lumaDown + lumaDownCorners);
        
        bool isHorizontal = (edgeHorizontal >= edgeVertical);
        
        // Select edge pixels
        float luma1 = isHorizontal ? lumaDown : lumaLeft;
        float luma2 = isHorizontal ? lumaUp : lumaRight;
        
        float gradient1 = luma1 - lumaCenter;
        float gradient2 = luma2 - lumaCenter;
        
        bool is1Steepest = abs(gradient1) >= abs(gradient2);
        
        float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));
        
        float stepLength = isHorizontal ? data.texelSize.y : data.texelSize.x;
        
        float lumaLocalAverage = 0.0;
        
        if (is1Steepest)
        {
            stepLength = -stepLength;
            lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
        }
        else
        {
            lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
        }
        
        float2 currentUv = uv;
        if (isHorizontal)
        {
            currentUv.y += stepLength * 0.5;
        }
        else
        {
            currentUv.x += stepLength * 0.5;
        }
        
        float2 offset = isHorizontal ? float2(data.texelSize.x, 0.0) : float2(0.0, data.texelSize.y);
        
        // Sample along edge
        float2 uv1 = currentUv - offset;
        float2 uv2 = currentUv + offset;
        
        float lumaEnd1 = rgb2luma(tex.Sample(LinearSampler, uv1).rgb);
        float lumaEnd2 = rgb2luma(tex.Sample(LinearSampler, uv2).rgb);
        lumaEnd1 -= lumaLocalAverage;
        lumaEnd2 -= lumaLocalAverage;
        
        bool reached1 = abs(lumaEnd1) >= gradientScaled;
        bool reached2 = abs(lumaEnd2) >= gradientScaled;
        bool reachedBoth = reached1 && reached2;
        
        if (!reached1)
        {
            uv1 -= offset;
        }
        if (!reached2)
        {
            uv2 += offset;
        }
        
        // Compute subpixel offset
        float distance1 = isHorizontal ? (uv.x - uv1.x) : (uv.y - uv1.y);
        float distance2 = isHorizontal ? (uv2.x - uv.x) : (uv2.y - uv.y);
        
        bool isDirection1 = distance1 < distance2;
        float distanceFinal = min(distance1, distance2);
        
        float edgeThickness = (distance1 + distance2);
        
        float pixelOffset = -distanceFinal / edgeThickness + 0.5;
        
        bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
        
        bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;
        
        float finalOffset = correctVariation ? pixelOffset : 0.0;
        
        // Subpixel antialiasing
        float lumaAverage = (1.0 / 12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
        float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
        float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
        float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * data.subpixelBlending;
        
        finalOffset = max(finalOffset, subPixelOffsetFinal);
        
        float2 finalUv = uv;
        if (isHorizontal)
        {
            finalUv.y += finalOffset * stepLength;
        }
        else
        {
            finalUv.x += finalOffset * stepLength;
        }
        
        return tex.Sample(LinearSampler, finalUv).rgb;
    }
}
