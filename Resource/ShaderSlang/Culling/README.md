# Nanite GPU Rendering Pipeline

这是一套基于 Slang 实现的 Nanite GPU 渲染管线，支持动态 LOD 选择、视锥体剔除和间接绘制。

## 系统架构

### 核心组件

1. **[NaniteCulling.slang](NaniteCulling.slang)** - 核心工具和数据结构
   - `NaniteRenderData`: 渲染所需的相机和视锥体数据
   - `NaniteLODSelector`: LOD 选择工具，基于屏幕空间误差
   - `NaniteCulling`: 视锥体剔除工具
   - `NaniteTraversal`: ClusterGroup 树遍历工具

2. **[MeshInstanceCullingModule.slang](MeshInstanceCullingModule.slang)** - 主计算着色器
   - `NaniteInitBuffersCS`: 初始化缓冲区
   - `NaniteInstanceCullingCS`: LOD 选择和剔除
   - `NanitePrepareIndirectDispatchCS`: 准备间接派发参数
   - `NaniteGenerateIndirectDrawCS`: 生成间接绘制命令

3. **[VisibilityCompaction.slang](VisibilityCompaction.slang)** - 可选优化
   - `NaniteVisibilityCompactionCS`: 可见性压缩
   - `NaniteBatchDrawCommandsCS`: 批量绘制优化
   - `NaniteOcclusionCullingCS`: 遮挡剔除（预留接口）

## 渲染管线流程

### CPU 端准备

```cpp
// 1. 准备 MeshInstanceData
struct MeshInstanceData {
    float4x4 matWorld;
    float3x3 matNormal;
    int rootClusterGroupBeginIndex;  // 指向 root ClusterGroup 的起始索引
    int rootClusterGroupCount;       // root ClusterGroup 的数量
};

// 2. 准备 ClusterGroupData（树形 LOD 结构）
struct ClusterGroupData {
    uint parentGroupId;
    uint firstGroupId;        // 第一个子 ClusterGroup 的索引
    uint childGroupCount;     // 子 ClusterGroup 的数量

    uint clusterBeginIndex;   // 该 Group 包含的 Cluster 起始索引
    uint clusterCount;        // Cluster 数量

    float3 position;          // 包围球中心（模型空间）
    float radius;             // 包围球半径
    float error;              // Nanite 误差值
};

// 3. 准备 ClusterData（叶子节点）
struct ClusterData {
    uint indexOffset;         // 索引缓冲区偏移
    uint indexCount;          // 索引数量
    uint valid;               // 是否有效

    float3 position;          // 包围球中心（模型空间）
    float radius;             // 包围球半径
    float error;              // Nanite 误差值

    uint meshInstanceIndex;   // 所属的 MeshInstance 索引
};
```

### GPU 端执行步骤

#### 第 0 步：初始化（每帧一次）

```cpp
Dispatch(NaniteInitBuffersCS, 1, 1, 1);
```

#### 第 1 步：LOD 选择和视锥体剔除

```cpp
// 准备 NaniteRenderData
NaniteRenderData renderData;
renderData.viewProjMatrix = camera.GetViewProjMatrix();
renderData.viewMatrix = camera.GetViewMatrix();
renderData.cameraPositionWS = camera.GetPosition();
renderData.screenSize = float2(screenWidth, screenHeight);
renderData.nearClip = camera.GetNearClip();
renderData.farClip = camera.GetFarClip();

// 从相机矩阵提取视锥体平面
ExtractFrustumPlanes(renderData.viewProjMatrix, renderData.frustumPlanes);

// 派发计算着色器
uint threadGroups = (instanceCount + 63) / 64;
Dispatch(NaniteInstanceCullingCS, threadGroups, 1, 1);
```

**输出：**
- `candidateClusters`: 候选 Cluster 列表
- `candidateCount[0]`: 候选数量

#### 第 2 步：准备间接派发参数

```cpp
Dispatch(NanitePrepareIndirectDispatchCS, 1, 1, 1);
```

**输出：**
- `dispatchArgs`: 用于下一步的间接派发参数

#### 第 3 步：生成间接绘制命令

```cpp
DispatchIndirect(NaniteGenerateIndirectDrawCS, dispatchArgsBuffer, 0);
```

**输出：**
- `indirectCommands`: DrawIndexedIndirect 命令数组
- `instanceIndices`: 每个绘制调用对应的 MeshInstance 索引

#### 第 4 步：执行间接绘制

```cpp
// 绑定顶点和索引缓冲区
BindVertexBuffer(vertexBuffer);
BindIndexBuffer(indexBuffer);

// 执行 MultiDrawIndexedIndirect
DrawIndexedIndirect(indirectCommandsBuffer, candidateCountBuffer[0]);
```

## 可选优化

### 可见性压缩

```cpp
// 对候选 Cluster 进行排序和批量处理
Dispatch(NaniteVisibilityCompactionCS, threadGroups, 1, 1);
Dispatch(NaniteBatchDrawCommandsCS, threadGroups, 1, 1);
```

### 遮挡剔除（未来）

```cpp
// 使用 Hi-Z 进行遮挡剔除
Dispatch(NaniteOcclusionCullingCS, threadGroups, 1, 1);
```

## 参数调优

### LOD 误差阈值

在 [NaniteCulling.slang](NaniteCulling.slang) 中调整：

```slang
public static const float NANITE_ERROR_THRESHOLD = 1.0; // 屏幕空间误差阈值（像素）
```

- 更小的值 (0.5-1.0): 更高的细节，更多的三角形
- 更大的值 (2.0-4.0): 更低的细节，更好的性能

### 遍历深度

```slang
public static const uint NANITE_MAX_TRAVERSAL_DEPTH = 16;
```

根据 LOD 树的深度调整，通常 8-16 足够。

## 缓冲区布局

### 所需的 GPU 缓冲区

1. **输入缓冲区：**
   - `meshInstances`: StructuredBuffer<MeshInstanceData>
   - `clusterGroups`: StructuredBuffer<ClusterGroupData>
   - `clusters`: StructuredBuffer<ClusterData>
   - `vertexBuffer`: Vertex buffer（与现有渲染管线兼容）
   - `indexBuffer`: Index buffer（与现有渲染管线兼容）

2. **中间缓冲区：**
   - `candidateClusters`: RWStructuredBuffer<CandidateCluster>（最大 = 总 cluster 数）
   - `candidateCount`: RWStructuredBuffer<uint>（1 个元素）
   - `dispatchArgs`: RWStructuredBuffer<uint3>（1 个元素）

3. **输出缓冲区：**
   - `indirectCommands`: RWStructuredBuffer<IndexedIndirectCommand>
   - `instanceIndices`: RWStructuredBuffer<uint>（用于查找对应的 MeshInstance）

## 性能考虑

1. **批量大小：** 每个计算着色器使用 64 个线程，根据实际情况调整
2. **缓冲区大小：** `candidateClusters` 应该足够大以容纳最坏情况（所有 cluster 可见）
3. **原子操作：** 使用 `InterlockedAdd` 进行计数，性能开销较小
4. **内存访问：** 尽量保持连续访问以提高缓存命中率

## 与现有系统集成

### 着色器端

在渲染着色器中，使用 `firstInstance` 参数查找对应的 MeshInstance：

```slang
[shader("vertex")]
VertexOut MainVS(
    VertexIn input,
    uint instanceId : SV_InstanceID)
{
    // 从 instanceIndices 查找真实的 MeshInstance 索引
    uint meshInstanceIndex = instanceIndices[instanceId];
    MeshInstanceData meshInstance = meshInstances[meshInstanceIndex];

    // 应用世界变换
    float4 worldPos = mul(meshInstance.matWorld, float4(input.pos, 1.0));
    // ...
}
```

### CPU 端数据准备

确保 ClusterData 中的 `meshInstanceIndex` 字段正确填充，以便在渲染时能够正确关联到 MeshInstance。

## 调试技巧

1. **可视化 LOD 层级：** 根据 cluster depth 着色
2. **可视化剔除结果：** 显示被剔除的 cluster 包围球
3. **性能分析：** 监控各阶段的候选数量和绘制调用次数

## 示例伪代码

```cpp
// CPU 端完整流程示例
class NaniteRenderer {
    void RenderFrame() {
        // 0. 初始化
        Dispatch(initCS, 1, 1, 1);

        // 1. 准备渲染数据
        NaniteRenderData renderData = PrepareRenderData();

        // 2. LOD 选择和剔除
        Dispatch(cullingCS, (instanceCount + 63) / 64, 1, 1);

        // 3. 准备间接派发
        Dispatch(prepareDispatchCS, 1, 1, 1);

        // 4. 生成间接绘制命令
        DispatchIndirect(generateDrawCS, dispatchArgsBuffer);

        // 5. 执行间接绘制
        BindPipeline(nanitePipeline);
        BindDescriptorSets(descriptorSets);
        DrawIndexedIndirect(indirectBuffer, candidateCountBuffer);
    }
};
```

## 未来扩展

1. **遮挡剔除：** 使用 Hierarchical Z-Buffer
2. **软件光栅化：** 小三角形使用计算着色器光栅化
3. **流式加载：** 动态加载/卸载 LOD 数据
4. **压缩：** 使用更紧凑的数据格式减少带宽

## 参考资料

- Unreal Engine 5 Nanite Documentation
- GPU-Driven Rendering Pipelines (Siggraph)
- Hierarchical LOD Systems
