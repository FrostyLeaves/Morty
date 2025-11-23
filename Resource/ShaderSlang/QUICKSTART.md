# Slang Shader Framework - 快速开始指南

## 安装和配置

### 1. 项目结构

确保你的项目有以下Shader文件结构：

```
ShaderSlang/
├── Internal/          # 核心定义和接口
├── Lighting/          # 光照系统
├── Model/            # 模型和材质
├── Deferred/         # 延迟渲染
├── Forward/          # 前向渲染
├── PostProcess/      # 后处理
├── Skybox/          # 天空盒
└── Examples/        # 示例代码
```

### 2. 编译Shader

使用Slang编译器编译着色器：

```bash
# 编译为SPIR-V
slangc shader.slang -profile glsl_450 -target spirv -o output.spv

# 编译为DXIL
slangc shader.slang -profile sm_6_0 -target dxil -o output.dxil

# 编译为GLSL
slangc shader.slang -profile glsl_450 -o output.glsl
```

## 创建你的第一个Shader

### 示例1：简单的Unlit Shader

```slang
import InternalModule;
import Model.ModelModule;

// 材质数据
struct SimpleMaterialData
{
    float4 color;
}

ParameterBlock<SimpleMaterialData> material;
[[vk::binding(0, 0)]] Texture2D mainTexture;

[shader("vertex")]
VertexOut VS_MAIN(VertexIn input, uint INSTANCE_ID: SV_InstanceID)
{
    FrameData frameData = GlobalFrameData;
    return ModelUtil.ComputeVertex(input, frameData.mainCamera, INSTANCE_ID);
}

[shader("pixel")]
float4 PS_MAIN(VertexOut input) : SV_Target
{
    float4 texColor = mainTexture.Sample(LinearSampler, input.uv);
    return texColor * material.color;
}
```

### 示例2：基础PBR材质

```slang
import InternalModule;
import Model.ModelModule;
import Forward.ForwardModule;

[shader("vertex")]
VertexOut VS_MAIN(VertexIn input, uint INSTANCE_ID: SV_InstanceID)
{
    FrameData frameData = GlobalFrameData;
    return ModelUtil.ComputeVertex(input, frameData.mainCamera, INSTANCE_ID);
}

[shader("pixel")]
float4 PS_MAIN(VertexOut input) : SV_Target
{
    FrameData frameData = GlobalFrameData;
    
    // 使用内置材质
    ModelMaterial material;
    var pixelData = ModelUtil.ComputePixelData(input);
    var surfaceData = material.GetSurfaceData(pixelData);
    surfaceData.cameraDir = normalize(frameData.mainCamera.positionWS - surfaceData.positionWS);
    
    // 计算光照（最多8个光源）
    float3 lighting = ForwardLighting.ComputeForwardLighting(surfaceData, 8);
    
    return float4(lighting, 1.0);
}
```

## CPU端数据设置

### C++ 示例（使用Vulkan）

```cpp
// 1. 设置全局帧数据
struct FrameData {
    glm::mat4 viewMatrix;
    glm::vec3 cameraPosition;
};

FrameData frameData;
frameData.viewMatrix = camera.GetViewProjectionMatrix();
frameData.cameraPosition = camera.GetPosition();

// 绑定到descriptor set
vkUpdateDescriptorSets(...);

// 2. 设置光照数据
struct LightData {
    glm::vec3 intensity;
    uint32_t type; // 0=Directional, 1=Spot, 2=Point
    glm::vec3 direction;
    glm::vec3 position;
    glm::vec3 extension;
};

std::vector<LightData> lights;
// 添加光源...

// 3. 设置材质数据
struct MaterialParams {
    glm::vec3 albedoTint = glm::vec3(1.0f);
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
};
```

### C# 示例（使用Unity或其他引擎）

```csharp
// 设置全局数据
Shader.SetGlobalMatrix("_ViewMatrix", camera.projectionMatrix * camera.worldToCameraMatrix);
Shader.SetGlobalVector("_CameraPosition", camera.transform.position);

// 设置光照数据
ComputeBuffer lightBuffer = new ComputeBuffer(lights.Count, sizeof(LightData));
lightBuffer.SetData(lights.ToArray());
material.SetBuffer("allLightData", lightBuffer);

// 设置材质参数
material.SetColor("_BaseColor", color);
material.SetFloat("_Metallic", metallic);
material.SetFloat("_Roughness", roughness);
```

## 常见使用场景

### 场景1：延迟渲染管线

```slang
// Pass 1: GBuffer生成
import Deferred.DeferredModule;

[shader("pixel")]
GBufferOutput PS_GBuffer(VertexOut input)
{
    ModelMaterial material;
    var surfaceData = material.GetSurfaceData(pixelData);
    return DeferredUtil.ComputePixel(surfaceData);
}

// Pass 2: 光照计算
import Lighting.LightingModule;

[shader("pixel")]
float4 PS_Lighting(float2 uv) : SV_Target
{
    GBufferLoader loader;
    var surfaceData = loader.LoadGBuffer(uv);
    
    LightingSystem lighting;
    return float4(lighting.ComputeLighting(surfaceData).color, 1.0);
}
```

### 场景2：带后处理的完整管线

```slang
// 主渲染pass输出HDR
[shader("pixel")]
float4 PS_Scene(...) : SV_Target
{
    // 渲染场景...
    return float4(hdrColor, 1.0);
}

// 后处理pass
import PostProcess.PostProcessModule;

[shader("pixel")]
float4 PS_PostProcess(float2 uv) : SV_Target
{
    float3 color = sceneTexture.Sample(LinearSampler, uv).rgb;
    
    // 色调映射
    ToneMappingData tmData;
    tmData.mode = ToneMappingMode.ACESFitted;
    tmData.exposure = 1.0;
    tmData.gamma = 2.2;
    
    color = ToneMapping.Apply(color, tmData);
    return float4(color, 1.0);
}
```

### 场景3：自定义材质

```slang
import InternalModule;

// 定义你自己的材质
struct MyCustomMaterial : IMaterial
{
    [[vk::binding(1, 0)]] Texture2D diffuseMap;
    [[vk::binding(2, 0)]] Texture2D specularMap;
    
    SurfaceData GetSurfaceData(PixelData pixelData)
    {
        SurfaceData data;
        
        // 自定义采样和计算逻辑
        data.albedo = diffuseMap.Sample(LinearSampler, pixelData.uv).rgb;
        data.roughness = 1.0 - specularMap.Sample(LinearSampler, pixelData.uv).r;
        data.metalness = 0.0;
        data.normalWS = normalize(pixelData.TBN[2]);
        data.positionWS = pixelData.positionWS;
        data.ao = 1.0;
        
        return data;
    }
}
```

## 性能优化技巧

### 1. 减少光源数量

```slang
// 限制每个物体接收的光源数量
const int MAX_LIGHTS_PER_OBJECT = 4;
float3 lighting = ForwardLighting.ComputeForwardLighting(surfaceData, MAX_LIGHTS_PER_OBJECT);
```

### 2. 使用LOD

```slang
// 根据距离选择不同的材质复杂度
float distanceToCamera = length(surfaceData.positionWS - cameraPos);

if (distanceToCamera > 50.0)
{
    // 简化材质
    surfaceData.roughness = 0.5;
    surfaceData.metalness = 0.0;
}
```

### 3. 动态分支优化

```slang
// 使用[branch]或[flatten]属性
[branch]
if (enableFeature)
{
    // 复杂计算
}
```

## 调试技巧

### 1. 可视化法线

```slang
import Internal.Utils;

[shader("pixel")]
float4 PS_Debug(VertexOut input) : SV_Target
{
    float3 normal = normalize(input.normal);
    return float4(DebugUtils.VisualizeNormal(normal), 1.0);
}
```

### 2. 可视化光照强度

```slang
[shader("pixel")]
float4 PS_Debug(VertexOut input) : SV_Target
{
    float3 lighting = ComputeLighting(...);
    float intensity = ColorUtils.Luminance(lighting);
    return float4(DebugUtils.HeatmapColor(intensity), 1.0);
}
```

## 下一步

1. 查看 `Examples/` 文件夹中的完整示例
2. 阅读 `README.md` 了解详细的API文档
3. 探索不同的后处理效果组合
4. 实验IBL和阴影系统

## 常见问题

**Q: 编译错误：找不到模块**
```
A: 确保使用 import 而不是 #include，并且模块路径正确
```

**Q: 光照看起来不对**
```
A: 检查法线是否在世界空间，确保normalWS正确计算
```

**Q: 性能问题**
```
A: 减少光源数量、使用延迟渲染、启用视锥体剔除
```

## 资源

- [Slang语言规范](https://shader-slang.org/slang/user-guide/)
- [PBR理论](https://learnopengl.com/PBR/Theory)
- [示例项目](./Examples/)
