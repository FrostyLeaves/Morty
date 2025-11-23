// 工具函数集合
implementing InternalModule;

// ==================== 数学工具 ====================

public struct MathUtils
{
    // 安全的归一化
    public static float3 SafeNormalize(float3 v)
    {
        float len = length(v);
        return len > 0.0001 ? v / len : float3(0.0, 0.0, 1.0);
    }
    
    // 构建正交基
    public static void BuildOrthonormalBasis(float3 normal, out float3 tangent, out float3 bitangent)
    {
        float3 up = abs(normal.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
        tangent = normalize(cross(up, normal));
        bitangent = cross(normal, tangent);
    }
    
    // 线性插值
    public static float Remap(float value, float oldMin, float oldMax, float newMin, float newMax)
    {
        return newMin + (value - oldMin) * (newMax - newMin) / (oldMax - oldMin);
    }
    
    // 平滑步进
    public static float SmoothStep(float edge0, float edge1, float x)
    {
        float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
        return t * t * (3.0 - 2.0 * t);
    }
    
    // 更平滑的步进
    public static float SmootherStep(float edge0, float edge1, float x)
    {
        float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
        return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    }
}

// ==================== 纹理采样工具 ====================

public struct TextureUtils
{
    // 三线性纹理采样
    public static float4 SampleTriplanar(Texture2D tex, float3 positionWS, float3 normalWS, float scale)
    {
        // 计算混合权重
        float3 blendWeights = abs(normalWS);
        blendWeights = blendWeights / (blendWeights.x + blendWeights.y + blendWeights.z);
        
        // 三个投影面采样
        float4 xProjection = tex.Sample(LinearSampler, positionWS.yz * scale);
        float4 yProjection = tex.Sample(LinearSampler, positionWS.xz * scale);
        float4 zProjection = tex.Sample(LinearSampler, positionWS.xy * scale);
        
        // 混合结果
        return xProjection * blendWeights.x + 
               yProjection * blendWeights.y + 
               zProjection * blendWeights.z;
    }
    
    // 带导数的纹理采样（用于各向异性过滤）
    public static float4 SampleWithDerivatives(Texture2D tex, float2 uv, float2 ddx, float2 ddy)
    {
        return tex.SampleGrad(LinearSampler, uv, ddx, ddy);
    }
}

// ==================== 颜色转换工具 ====================

public struct ColorUtils
{
    // sRGB to Linear
    public static float3 SRGBToLinear(float3 srgb)
    {
        return float3(
            srgb.r <= 0.04045 ? srgb.r / 12.92 : pow((srgb.r + 0.055) / 1.055, 2.4),
            srgb.g <= 0.04045 ? srgb.g / 12.92 : pow((srgb.g + 0.055) / 1.055, 2.4),
            srgb.b <= 0.04045 ? srgb.b / 12.92 : pow((srgb.b + 0.055) / 1.055, 2.4)
        );
    }
    
    // Linear to sRGB
    public static float3 LinearToSRGB(float3 linear)
    {
        return float3(
            linear.r <= 0.0031308 ? linear.r * 12.92 : 1.055 * pow(linear.r, 1.0/2.4) - 0.055,
            linear.g <= 0.0031308 ? linear.g * 12.92 : 1.055 * pow(linear.g, 1.0/2.4) - 0.055,
            linear.b <= 0.0031308 ? linear.b * 12.92 : 1.055 * pow(linear.b, 1.0/2.4) - 0.055
        );
    }
    
    // RGB to Luminance
    public static float Luminance(float3 rgb)
    {
        return dot(rgb, float3(0.2126, 0.7152, 0.0722));
    }
    
    // RGB to HSV
    public static float3 RGBToHSV(float3 rgb)
    {
        float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        float4 p = lerp(float4(rgb.bg, K.wz), float4(rgb.gb, K.xy), step(rgb.b, rgb.g));
        float4 q = lerp(float4(p.xyw, rgb.r), float4(rgb.r, p.yzx), step(p.x, rgb.r));
        
        float d = q.x - min(q.w, q.y);
        float e = 1.0e-10;
        return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    
    // HSV to RGB
    public static float3 HSVToRGB(float3 hsv)
    {
        float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        float3 p = abs(frac(hsv.xxx + K.xyz) * 6.0 - K.www);
        return hsv.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), hsv.y);
    }
}

// ==================== 噪声函数 ====================

public struct NoiseUtils
{
    // 伪随机噪声
    public static float Random(float2 st)
    {
        return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
    }
    
    // 值噪声
    public static float ValueNoise(float2 st)
    {
        float2 i = floor(st);
        float2 f = frac(st);
        
        float a = Random(i);
        float b = Random(i + float2(1.0, 0.0));
        float c = Random(i + float2(0.0, 1.0));
        float d = Random(i + float2(1.0, 1.0));
        
        float2 u = f * f * (3.0 - 2.0 * f);
        
        return lerp(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
    }
    
    // 分形布朗运动（FBM）
    public static float FBM(float2 st, int octaves)
    {
        float value = 0.0;
        float amplitude = 0.5;
        float frequency = 1.0;
        
        for (int i = 0; i < octaves; ++i)
        {
            value += amplitude * ValueNoise(st * frequency);
            frequency *= 2.0;
            amplitude *= 0.5;
        }
        
        return value;
    }
}

// ==================== 视差映射工具 ====================

public struct ParallaxUtils
{
    // 简单视差映射
    public static float2 ParallaxMapping(float2 uv, float3 viewDir, Texture2D heightMap, float heightScale)
    {
        float height = heightMap.Sample(LinearSampler, uv).r;
        float2 p = viewDir.xy / viewDir.z * (height * heightScale);
        return uv - p;
    }
    
    // 陡峭视差映射
    public static float2 SteepParallaxMapping(float2 uv, float3 viewDir, Texture2D heightMap, float heightScale, int numLayers)
    {
        float layerDepth = 1.0 / float(numLayers);
        float currentLayerDepth = 0.0;
        float2 P = viewDir.xy * heightScale;
        float2 deltaTexCoords = P / float(numLayers);
        
        float2 currentTexCoords = uv;
        float currentDepthMapValue = heightMap.Sample(LinearSampler, currentTexCoords).r;
        
        for (int i = 0; i < numLayers; ++i)
        {
            if (currentLayerDepth >= currentDepthMapValue)
                break;
                
            currentTexCoords -= deltaTexCoords;
            currentDepthMapValue = heightMap.Sample(LinearSampler, currentTexCoords).r;
            currentLayerDepth += layerDepth;
        }
        
        return currentTexCoords;
    }
}

// ==================== 法线处理工具 ====================

public struct NormalUtils
{
    // 从高度图计算法线
    public static float3 CalculateNormalFromHeight(Texture2D heightMap, float2 uv, float2 texelSize, float strength)
    {
        float h0 = heightMap.Sample(LinearSampler, uv).r;
        float hX = heightMap.Sample(LinearSampler, uv + float2(texelSize.x, 0.0)).r;
        float hY = heightMap.Sample(LinearSampler, uv + float2(0.0, texelSize.y)).r;
        
        float3 normal;
        normal.x = (h0 - hX) * strength;
        normal.y = (h0 - hY) * strength;
        normal.z = 1.0;
        
        return normalize(normal);
    }
    
    // 混合法线（重新归一化）
    public static float3 BlendNormals(float3 normal1, float3 normal2)
    {
        return normalize(float3(normal1.xy + normal2.xy, normal1.z * normal2.z));
    }
    
    // 混合法线（UDN方法）
    public static float3 BlendNormalsUDN(float3 normal1, float3 normal2)
    {
        float3 t = normal1.xyz + float3(0.0, 0.0, 1.0);
        float3 u = normal2.xyz * float3(-1.0, -1.0, 1.0);
        return normalize(t * dot(t, u) - u * t.z);
    }
}

// ==================== 视锥体剔除工具 ====================

public struct FrustumUtils
{
    public struct Plane
    {
        float3 normal;
        float distance;
    }
    
    public struct Frustum
    {
        Plane planes[6]; // left, right, bottom, top, near, far
    }
    
    // 点是否在视锥体内
    public static bool PointInFrustum(float3 point, Frustum frustum)
    {
        for (int i = 0; i < 6; ++i)
        {
            if (dot(frustum.planes[i].normal, point) + frustum.planes[i].distance < 0.0)
                return false;
        }
        return true;
    }
    
    // 球体是否在视锥体内
    public static bool SphereInFrustum(float3 center, float radius, Frustum frustum)
    {
        for (int i = 0; i < 6; ++i)
        {
            if (dot(frustum.planes[i].normal, center) + frustum.planes[i].distance < -radius)
                return false;
        }
        return true;
    }
}

// ==================== 调试工具 ====================

public struct DebugUtils
{
    // 将值可视化为热图颜色
    public static float3 HeatmapColor(float value)
    {
        value = saturate(value);
        float3 red = float3(1.0, 0.0, 0.0);
        float3 yellow = float3(1.0, 1.0, 0.0);
        float3 green = float3(0.0, 1.0, 0.0);
        float3 cyan = float3(0.0, 1.0, 1.0);
        float3 blue = float3(0.0, 0.0, 1.0);
        
        if (value < 0.25)
            return lerp(blue, cyan, value * 4.0);
        else if (value < 0.5)
            return lerp(cyan, green, (value - 0.25) * 4.0);
        else if (value < 0.75)
            return lerp(green, yellow, (value - 0.5) * 4.0);
        else
            return lerp(yellow, red, (value - 0.75) * 4.0);
    }
    
    // 可视化法线
    public static float3 VisualizeNormal(float3 normal)
    {
        return normal * 0.5 + 0.5;
    }
    
    // 可视化深度
    public static float3 VisualizeDepth(float depth, float near, float far)
    {
        float linearDepth = (2.0 * near) / (far + near - depth * (far - near));
        return float3(linearDepth, linearDepth, linearDepth);
    }
}
