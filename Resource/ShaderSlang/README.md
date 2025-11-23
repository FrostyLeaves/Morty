# Slang Shader Framework 文档

## 概述

这是一个基于 Slang 着色语言的现代渲染引擎shader框架，支持延迟渲染、前向渲染、PBR材质、阴影、IBL、后处理等功能。

## 架构设计

### 模块系统

框架采用模块化设计，主要模块包括：

1. **InternalModule** - 核心模块，提供基础数据结构和接口
2. **LightingModule** - 光照系统，包含PBR、阴影和IBL
3. **ModelModule** - 模型和材质系统
4. **DeferredModule** - 延迟渲染管线
5. **ForwardModule** - 前向渲染管线
6. **PostProcessModule** - 后处理效果
7. **SkyboxModule** - 天空盒和大气散射

## 核心数据结构

### FrameData
```slang
struct FrameData
{
    CameraData mainCamera;
}
```
全局帧数据，通过 `GlobalFrameData` 访问。

### SurfaceData
```slang
struct SurfaceData
{
    float3 albedo;          // 反照率
    float roughness;        // 粗糙度
    float3 normalWS;        // 世界空间法线
    float metalness;        // 金属度
    float3 positionWS;      // 世界空间位置
    float ao;               // 环境光遮蔽
    float3 cameraDir;       // 摄像机方向
}
```

### LightData
```slang
struct LightData
{
    float3 intensity;       // 光强度
    LightType type;         // 光源类型
    float3 dir;             // 方向
    float3 positionWS;      // 世界空间位置
    float3 extension;       // 扩展参数
}
```

支持三种光源类型：
- **Directional** - 平行光
- **Point** - 点光源（extension: x=constant, y=linear, z=quadratic）
- **Spot** - 聚光灯（extension: x=innerCutoff, y=outerCutoff）

## 渲染管线

### 1. 延迟渲染（Deferred Rendering）

#### GBuffer Pass
```slang
import InternalModule;
import Model.ModelModule;
import Deferred.DeferredModule;

[shader("vertex")]
VertexOut VS_MAIN(VertexIn input, uint INSTANCE_ID: SV_InstanceID)
{
    FrameData frameData = GlobalFrameData;
    return ModelUtil.ComputeVertex(input, frameData.mainCamera, INSTANCE_ID);
}

[shader("pixel")]
PixelOut PS_MAIN(VertexOut input)
{
    ModelMaterial material;
    var pixelData = ModelUtil.ComputePixelData(input);
    var surfaceData = material.GetSurfaceData(pixelData);
    
    GBufferPixelData gbufferData = DeferredUtil.ComputePixel(surfaceData);
    // 输出到多个渲染目标
}
```

#### Lighting Pass
```slang
import Deferred.DeferredModule;
import Lighting.LightingModule;

[shader("pixel")]
PixelOut PS_MAIN(VertexOut input)
{
    GBufferLoader gBufferLoader;
    var surfaceData = gBufferLoader.LoadGBuffer(input.uv);
    
    LightingSystem lightingSystem;
    LightingOutput output = lightingSystem.ComputeLighting(surfaceData);
}
```

### 2. 前向渲染（Forward Rendering）

```slang
import Forward.ForwardModule;
import Lighting.LightingModule;

[shader("pixel")]
PixelOut PS_MAIN(VertexOut input)
{
    ModelMaterial material;
    var surfaceData = material.GetSurfaceData(pixelData);
    
    // 计算光照
    float3 lighting = ForwardLighting.ComputeForwardLighting(surfaceData, 16);
}
```

## 光照系统

### PBR材质

框架使用Cook-Torrance BRDF模型：

```slang
float3 BRDF(float3 lightColor, float3 cameraDir, float3 lightDir, 
            float3 normal, float3 albedo, float roughness, float metalness)
```

包含：
- **法线分布函数（NDF）**: GGX/Trowbridge-Reitz
- **几何函数**: Smith's Schlick-GGX
- **菲涅尔方程**: Schlick approximation

### 阴影系统

支持PCF（Percentage Closer Filtering）软阴影：

```slang
ShadowSystem shadowSystem;
float shadow = shadowSystem.CalculateShadowPCF(positionWS, shadowData, normalWS, lightDir);
```

### 基于图像的照明（IBL）

```slang
IBLSystem iblSystem;
float3 ambient = iblSystem.CalculateIBL(surfaceData, iblData);
```

需要提供：
- **irradianceMap** - 漫反射辐照度贴图
- **prefilterMap** - 预滤波环境贴图
- **brdfLUT** - BRDF查找表

## 材质系统

### 自定义材质

实现 `IMaterial` 接口：

```slang
struct CustomMaterial : IMaterial
{
    SurfaceData GetSurfaceData(PixelData pixelData)
    {
        SurfaceData data;
        // 从纹理采样和计算
        data.albedo = albedoTexture.Sample(LinearSampler, pixelData.uv).rgb;
        data.normalWS = normalTexture.Sample(LinearSampler, pixelData.uv).xyz;
        // ...
        return data;
    }
}
```

### 内置材质

1. **ModelMaterial** - 标准PBR材质
   - albedoTexture
   - normalTexture
   - metallicTexture
   - roughnessTexture
   - aoTexture

2. **UnlitMaterial** - 无光照材质

## 后处理效果

### 色调映射（Tone Mapping）

支持多种色调映射算法：

```slang
ToneMappingData tmData;
tmData.mode = ToneMappingMode.ACESFitted;
tmData.exposure = 1.0;
tmData.gamma = 2.2;

float3 color = ToneMapping.Apply(hdrColor, tmData);
```

支持的模式：
- Reinhard
- ReinhardLuminance
- Uncharted2
- ACES
- ACESFitted（推荐）

### Bloom（辉光）

```slang
BloomData bloomData;
bloomData.threshold = 1.0;
bloomData.intensity = 0.5;

// 提取高亮区域
float3 bright = Bloom.Prefilter(color, bloomData);

// 降采样
float3 blurred = Bloom.DownsampleBox13(tex, uv, texelSize);

// 升采样
float3 result = Bloom.UpsampleTent(tex, uv, texelSize, 1.0);
```

### FXAA（快速近似抗锯齿）

```slang
FXAAData fxaaData;
fxaaData.texelSize = float2(1.0 / width, 1.0 / height);
fxaaData.contrastThreshold = 0.0312;
fxaaData.relativeThreshold = 0.063;
fxaaData.subpixelBlending = 0.75;

float3 antialiased = FXAA.Apply(colorTexture, uv, fxaaData);
```

### 颜色分级（Color Grading）

```slang
ColorGradingData cgData;
cgData.saturation = 1.0;
cgData.contrast = 1.0;
cgData.brightness = 1.0;
cgData.colorFilter = float3(1.0, 1.0, 1.0);
cgData.temperature = 0.0;
cgData.tint = 0.0;

float3 graded = ColorGrading.Apply(color, cgData);
```

## 天空盒系统

### 标准天空盒

```slang
Skybox skybox;
SkyboxData skyboxData;
skyboxData.intensity = 1.0;
skyboxData.rotation = 0.0;
skyboxData.tint = float3(1.0, 1.0, 1.0);

float3 color = skybox.Sample(direction, skyboxData);
```

### 大气散射

物理准确的大气散射模拟：

```slang
AtmosphericData atmosData;
atmosData.sunDirection = normalize(float3(0.0, 1.0, 0.3));
atmosData.sunIntensity = 20.0;
atmosData.rayleighScattering = float3(5.8e-6, 13.5e-6, 33.1e-6);
atmosData.mieScattering = 21e-6;

float3 color = AtmosphericScattering.CalculateScattering(rayOrigin, rayDir, maxDist, atmosData);
```

## 使用示例

### 完整的前向渲染Shader

```slang
import InternalModule;
import Model.ModelModule;
import Forward.ForwardModule;
import Lighting.LightingModule;

[shader("vertex")]
VertexOut VS_MAIN(VertexIn input, uint INSTANCE_ID: SV_InstanceID)
{
    FrameData frameData = GlobalFrameData;
    return ModelUtil.ComputeVertex(input, frameData.mainCamera, INSTANCE_ID);
}

[shader("pixel")]
PixelOut PS_MAIN(VertexOut input)
{
    FrameData frameData = GlobalFrameData;
    
    // 获取材质数据
    ModelMaterial material;
    var pixelData = ModelUtil.ComputePixelData(input);
    var surfaceData = material.GetSurfaceData(pixelData);
    surfaceData.cameraDir = normalize(frameData.mainCamera.positionWS - surfaceData.positionWS);
    
    // 计算光照
    float3 lighting = ForwardLighting.ComputeForwardLighting(surfaceData, 16);
    
    // 应用环境光遮蔽
    lighting *= surfaceData.ao;
    
    PixelOut output;
    output.color = float4(lighting, 1.0);
    return output;
}
```

### 后处理管线

```slang
import PostProcess.PostProcessModule;

[shader("pixel")]
float4 PS_PostProcess(PostProcessPixel input) : SV_Target
{
    // 1. FXAA
    float3 color = FXAA.Apply(sceneTexture, input.uv, fxaaData);
    
    // 2. Bloom
    float3 bloom = bloomTexture.Sample(LinearSampler, input.uv).rgb;
    color += bloom * bloomIntensity;
    
    // 3. Color Grading
    color = ColorGrading.Apply(color, colorGradingData);
    
    // 4. Tone Mapping
    color = ToneMapping.Apply(color, toneMappingData);
    
    return float4(color, 1.0);
}
```

## 性能优化建议

1. **光源数量控制**：前向渲染建议最多16个光源，延迟渲染可支持更多
2. **阴影级联**：对于大场景使用级联阴影贴图
3. **LOD系统**：根据距离调整材质复杂度
4. **剔除**：使用视锥体剔除和遮挡剔除
5. **批处理**：使用实例化渲染减少Draw Call

## 扩展指南

### 添加新的光源类型

1. 在 `CommonStruct.slang` 中添加枚举值
2. 在 `LightingSystem.slang` 中实现光照计算
3. 在 `ForwardLighting.slang` 中添加处理逻辑

### 添加新的后处理效果

1. 在 `PostProcess` 文件夹创建新的 `.slang` 文件
2. 实现 `implementing PostProcessModule`
3. 在 `PostProcessModule.slang` 中包含新文件

### 自定义材质系统

继承 `IMaterial` 接口并实现 `GetSurfaceData` 方法。

## 最佳实践

1. **使用模块导入**：保持代码模块化和可维护性
2. **参数化材质**：使用 `ParameterBlock` 传递材质参数
3. **法线贴图**：总是使用切线空间法线贴图
4. **线性空间**：所有计算在线性空间进行，最后应用Gamma校正
5. **HDR管线**：使用浮点渲染目标和色调映射

## 常见问题

**Q: 如何添加自定义纹理？**
A: 使用 `[[vk::binding(N, M)]]` 注解声明纹理，其中N是binding点，M是set。

**Q: 如何优化阴影性能？**
A: 使用级联阴影、降低阴影贴图分辨率、减少PCF采样数量。

**Q: 为什么颜色看起来太暗？**
A: 检查色调映射的exposure参数，确保HDR值正确映射到LDR范围。

## 参考资料

- [Slang官方文档](https://shader-slang.org/)
- [PBR理论基础](https://learnopengl.com/PBR/Theory)
- [实时渲染](http://www.realtimerendering.com/)
