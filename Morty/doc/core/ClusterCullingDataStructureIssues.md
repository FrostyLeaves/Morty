# Cluster Culling 数据结构不一致问题记录

> 创建时间: 2026-01-04
> 状态: 待修复

---

## 问题概览

| 编号 | 严重程度 | 模块 | 问题描述 | 状态 |
|------|----------|------|----------|------|
| #1 | 🔴 严重 | MMeshInstanceRenderProxy | 缺少 Shader 必需字段 | ✅ 方案已确定 |
| #2 | 🟡 中等 | MClusterRenderData | C++ 多余字段 + Shader 冗余字段 | ✅ 方案已确定 |
| #3 | 🟢 正常 | MClusterGroupRenderData | 与 Shader 一致 | ✅ 无需修改 |
| #4 | 🟡 中等 | MMeshManager | 第96-97行语法错误 | 待修复 |

---

## 问题 #1：MMeshInstanceRenderProxy 缺少关键字段

### 文件位置
- C++ 端: `Render/Batch/Mesh/MMaterialBatchGroup.h:23`
- Shader 端: `Resource/ShaderSlang/Model/CommonModel.slang:33`

### 当前 C++ 定义

```cpp
struct MMeshInstanceRenderProxy {
    Matrix4              worldTransform = Matrix4::IdentityMatrix;
    MMeshInstanceKey     proxyId        = MGlobal::M_INVALID_INDEX;
    MMaterialInstanceKey materialInstanceId     = MGlobal::M_INVALID_INDEX;
    bool                 visible        = false;
};
```

### Shader 端期望

```glsl
public struct MeshInstanceData
{
    public float4x4 matWorld;
    public float3x3 matNormal;
    public int rootClusterGroupBeginIndex;
    public int rootClusterGroupCount;
};
```

### 原缺失字段（已确定处理方案）

| 字段 | 类型 | 用途 | 处理方案 |
|------|------|------|----------|
| ~~`matNormal`~~ | float3x3 | 法线变换矩阵 | **Shader 动态计算** (见附录) |
| `rootClusterGroupBeginIndex` | int | LOD 树根节点起始索引 | **移至 MeshResourceData** |
| `rootClusterGroupCount` | int | LOD 树根节点数量 | **移至 MeshResourceData** |

### 影响
- 无法执行 GPU 端的 LOD 树遍历 → 通过 `meshResourceId` 间接查找解决
- ~~无法正确计算光照~~ → Shader 动态计算 `matNormal` 解决

---

## 问题 #2：MClusterRenderData 结构不匹配

### 文件位置
- C++ 端: `Render/Mesh/MCluster.h:70`
- Shader 端: `Resource/ShaderSlang/Model/CommonModel.slang:5`

### 当前 C++ 定义

```cpp
struct MClusterRenderData {
    uint32_t indexOffset = 0;
    uint32_t indexCount  = 0;
    uint32_t parent      = MGlobal::M_INVALID_UINDEX;   // 不应该在这里
    uint32_t firstChild  = MGlobal::M_INVALID_UINDEX;   // 不应该在这里
    uint32_t childCount  = 0;                           // 不应该在这里
    uint32_t valid       = 0;

    Vector3  position;
    float    radius;
    float    error;
};
```

### 当前 Shader 端定义

```glsl
public struct ClusterData
{
    uint indexOffset;
    uint indexCount;
    uint valid;

    float3 position;
    float radius;
    float error;

    uint meshInstanceIndex;  // 冗余字段，实际未使用
};
```

### 修正后的目标结构

```glsl
// Shader 端 (CommonModel.slang)
public struct ClusterData
{
    uint indexOffset;
    uint indexCount;
    uint valid;

    float3 position;
    float radius;
    float error;
    // 移除 meshInstanceIndex - 该信息从 Instance 遍历时传递
};
```

```cpp
// C++ 端 (MCluster.h)
struct MClusterRenderData {
    uint32_t indexOffset = 0;
    uint32_t indexCount  = 0;
    uint32_t valid       = 0;

    Vector3  position;
    float    radius;
    float    error;
    // 移除 parent/firstChild/childCount - 这些属于 ClusterGroup
};
```

### 问题详情

#### 2.1 C++ 端多余字段（应移除）

| 字段 | 原因 |
|------|------|
| `parent` | 这是 ClusterGroup 的属性，不属于 Cluster |
| `firstChild` | 这是 ClusterGroup 的属性，不属于 Cluster |
| `childCount` | 这是 ClusterGroup 的属性，不属于 Cluster |

#### 2.2 Shader 端冗余字段（应移除）

| 字段 | 原因 |
|------|------|
| `meshInstanceIndex` | 实际代码中未使用，meshInstanceIndex 是从 Instance 遍历时作为参数传递的 |

> **分析依据**: `NaniteCulling.slang:255` 中 `candidate.meshInstanceIndex = meshInstanceIndex` 使用的是函数参数，不是 `cluster.meshInstanceIndex`

### 影响
- C++ 端内存浪费（多余的 parent/firstChild/childCount 字段）
- 数据布局与 Shader 不匹配
- Shader 端 ClusterData 有冗余字段

---

## 问题 #3：MClusterGroupRenderData（已正确实现）

### 文件位置
- C++ 端: `Render/Mesh/MCluster.h:94`
- Shader 端: `Resource/ShaderSlang/Model/CommonModel.slang:18`

### 对比结果

| 字段 | C++ | Shader | 状态 |
|------|-----|--------|------|
| valid | uint32_t | uint | ✓ |
| parentGroupId | uint32_t | uint | ✓ |
| firstGroupId | uint32_t | uint | ✓ |
| childGroupCount | uint32_t | uint | ✓ |
| clusterBeginIndex | uint32_t | uint | ✓ |
| clusterCount | uint32_t | uint | ✓ |
| position | Vector3 | float3 | ✓ |
| radius | float | float | ✓ |
| error | float | float | ✓ |

**结论**: 完全一致，无需修改

---

## 问题 #4：MMeshManager.h 语法错误

### 文件位置
- `Render/Mesh/MMeshManager.h:96-97`

### 问题代码

```cpp
    std::vector <
};
```

### 影响
- 编译错误（如果该行被启用）
- 代码不完整

---

## 修复方案（已确定）

### 问题 #1 修复：分离 Mesh 资源数据与实例数据

**核心思路**: 将 Mesh 资源级别的数据（ClusterGroup 信息）与实例级别的数据（变换矩阵）分离

#### 1.1 新增 MeshResourceData 结构

**Shader 端** (`CommonModel.slang`):
```glsl
public struct MeshResourceData
{
    int rootClusterGroupBeginIndex;
    int rootClusterGroupCount;
};
```

**C++ 端** (`MCluster.h`):
```cpp
struct MMeshResourceData {
    int32_t rootClusterGroupBeginIndex = MGlobal::M_INVALID_INDEX;
    int32_t rootClusterGroupCount = 0;
};
```

#### 1.2 修改 MeshInstanceData / MMeshInstanceRenderProxy

> **matNormal 决策**: 选择方案 A - Shader 动态计算法线矩阵
> - 不在数据结构中存储 matNormal / normalTransform
> - Shader 端动态计算: `float3x3 matNormal = transpose(inverse(matWorld))`
> - 优点: 减少内存占用 (每 Instance 节省 36-48 字节)，避免 float3x3 对齐问题
> - 缺点: 增加 Shader 计算开销，但 inverse(4x4) 仅需一次

**Shader 端** (`CommonModel.slang`):
```glsl
public struct MeshInstanceData
{
    float4x4 matWorld;
    int meshResourceId;    // 索引到 MeshResourceData 缓冲
    int materialInstanceId;
    // matNormal 移除 - 在 Shader 中动态计算
};
```

**C++ 端** (`MMaterialBatchGroup.h`):
```cpp
struct MMeshInstanceRenderProxy {
    Matrix4              worldTransform = Matrix4::IdentityMatrix;
    int32_t              meshResourceId = MGlobal::M_INVALID_INDEX;  // 新增
    int32_t              materialInstanceId = MGlobal::M_INVALID_INDEX;      // 改为 int32_t
    // normalTransform 移除 - Shader 端动态计算

    // CPU 管理字段（不上传 GPU）
    MMeshInstanceKey     proxyId = MGlobal::M_INVALID_INDEX;
    bool                 visible = false;
};
```

#### 1.3 MMeshManager 添加 MeshResourceBuffer 管理

**修改文件**: `Render/Mesh/MMeshManager.h/.cpp`

```cpp
class MMeshManager : public IManager
{
    // ... 现有代码 ...

    // 新增：Mesh 资源数据缓冲
    std::vector<MMeshResourceData>  m_meshResourceDatas;
    MBuffer                         m_meshResourceBuffer;

public:
    // 新增接口
    [[nodiscard]] const MBuffer* GetMeshResourceBuffer() const { return &m_meshResourceBuffer; }

    // 注册 Mesh 时填充 MMeshResourceData
    bool RegisterMesh(MIMesh* mesh);  // 内部维护 meshResourceId

private:
    void UpdateMeshResourceBuffer();
};
```

#### 1.4 数据流向

```
┌─────────────────────────────────────────────────────────────────────┐
│                        数据组织架构                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  MeshResourceBuffer (per Mesh)          InstanceBuffer (per Instance)
│  ┌─────────────────────────┐            ┌──────────────────────────┐
│  │ MeshResourceData[0]     │            │ MeshInstanceData[0]      │
│  │  - rootClusterGroupBegin │◀──────────│  - matWorld              │
│  │  - rootClusterGroupCount │   ref     │  - meshResourceId = 0    │
│  ├─────────────────────────┤            │  - materialInstanceId            │
│  │ MeshResourceData[1]     │            ├──────────────────────────┤
│  │  - rootClusterGroupBegin │◀──┐       │ MeshInstanceData[1]      │
│  │  - rootClusterGroupCount │   │       │  - meshResourceId = 1    │
│  └─────────────────────────┘   │       ├──────────────────────────┤
│                                 └───────│ MeshInstanceData[2]      │
│                                         │  - meshResourceId = 1    │
│                                         └──────────────────────────┘
│                                                                     │
│  优势: 多个 Instance 共享同一 Mesh 资源时，不重复存储 ClusterGroup 信息
│  注意: matNormal 在 Shader 中动态计算 = transpose(inverse(matWorld))
└─────────────────────────────────────────────────────────────────────┘
```

#### 1.5 Shader 端使用方式

```glsl
// NaniteCulling.slang 中的遍历逻辑修改
[shader("compute")]
public void NaniteInstanceCullingCS(
    StructuredBuffer<MeshInstanceData> meshInstances,
    StructuredBuffer<MeshResourceData> meshResources,  // 新增
    StructuredBuffer<ClusterGroupData> clusterGroups,
    ...
)
{
    MeshInstanceData instance = meshInstances[instanceIndex];
    MeshResourceData resource = meshResources[instance.meshResourceId];  // 间接查找

    // 使用 resource.rootClusterGroupBeginIndex 和 rootClusterGroupCount
    for (int i = 0; i < resource.rootClusterGroupCount; ++i)
    {
        uint rootGroupIndex = resource.rootClusterGroupBeginIndex + i;
        // ...
    }
}
```

---

## 待讨论事项

1. [x] 问题 #1 的修复方案选择 → **已确定：分离 Mesh 资源数据与实例数据**
2. [x] 问题 #2 meshInstanceIndex 分析 → **已确定：Shader 未使用，应移除**
3. [x] 问题 #2 parent/firstChild/childCount 分析 → **已确定：Shader 中不存在这些字段，C++ 应移除**
4. [x] 命名规范 → **已确定：保持现状，各自风格**
5. [x] matNormal 处理策略 → **已确定：方案 A - Shader 动态计算，移除存储**

---

### 问题 #2 修复：统一 ClusterData 结构

#### Shader 中 ClusterData 实际使用的字段

| 字段 | 使用位置 | 用途 |
|------|----------|------|
| `position` | NaniteCulling.slang:115,258 | 包围球中心 |
| `radius` | NaniteCulling.slang:122 | 包围球半径 |
| `valid` | NaniteCulling.slang:240 | 有效性标记 |
| `indexCount` | MeshInstanceCullingModule.slang:72 | 间接绘制 |
| `indexOffset` | MeshInstanceCullingModule.slang:74 | 间接绘制 |
| `error` | 未使用 | 保留，LOD 选择预留 |

#### 目标结构

**C++ 端** (`Render/Mesh/MCluster.h`):
```cpp
struct MClusterRenderData {
    uint32_t indexOffset = 0;
    uint32_t indexCount  = 0;
    uint32_t valid       = 0;

    Vector3  position;
    float    radius;
    float    error;
    // 移除: parent, firstChild, childCount
};
```

**Shader 端** (`Resource/ShaderSlang/Model/CommonModel.slang`):
```glsl
public struct ClusterData
{
    uint indexOffset;
    uint indexCount;
    uint valid;

    float3 position;
    float radius;
    float error;
    // 移除: meshInstanceIndex
};
```

---

## 相关文件清单

### 问题 #1 相关文件

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `Render/Batch/Mesh/MMaterialBatchGroup.h` | C++ Header | 修改 MMeshInstanceRenderProxy，添加 meshResourceId, materialInstanceId (无需 normalTransform) |
| `Render/Mesh/MCluster.h` | C++ Header | 新增 MMeshResourceData 结构 |
| `Render/Mesh/MMeshManager.h` | C++ Header | 添加 m_meshResourceDatas, m_meshResourceBuffer, GetMeshResourceBuffer() |
| `Render/Mesh/MMeshManager.cpp` | C++ Source | 实现 MeshResource 缓冲管理和上传 |
| `Render/Batch/Mesh/MMeshInstanceManager.cpp` | C++ Source | 更新 CreateProxyFromComponent，填充 meshResourceId |
| `Resource/ShaderSlang/Model/CommonModel.slang` | Shader | 修改 MeshInstanceData (移除 matNormal)，新增 MeshResourceData |
| `Resource/ShaderSlang/Model/ModelUtil.slang` | Shader | 添加 matNormal 动态计算: `transpose(inverse(matWorld))` |
| `Resource/ShaderSlang/Culling/NaniteCulling.slang` | Shader | 更新 TraverseLODTree，使用 MeshResourceData |
| `Resource/ShaderSlang/Culling/MeshInstanceCullingModule.slang` | Shader | 更新 CS 参数，添加 meshResources 缓冲 |

### 问题 #2 相关文件

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `Render/Mesh/MCluster.h` | C++ Header | MClusterRenderData 移除 parent/firstChild/childCount |
| `Resource/ShaderSlang/Model/CommonModel.slang` | Shader | ClusterData 移除 meshInstanceIndex |

---

## 附录：matNormal 处理决策

### 背景

`matNormal` (法线变换矩阵) 用于在光照计算中正确变换法线向量。当模型存在非均匀缩放时，直接使用 `matWorld` 变换法线会导致错误结果。

**正确公式**: `matNormal = transpose(inverse(matWorld))`

### 方案对比

| 方案 | 描述 | 优点 | 缺点 |
|------|------|------|------|
| **A** | Shader 动态计算 | 减少内存，避免对齐问题 | 增加计算开销 |
| B | CPU 预计算存储 | 避免重复计算 | 增加内存，需处理对齐 |

### 选择结果：方案 A

**理由**:
1. **内存节省**: 每 Instance 节省 36-48 字节 (float3x3 对齐后)
2. **避免对齐问题**: float3x3 在 GPU 缓冲中需要 48 字节 (3x float4 对齐)，容易出错
3. **计算开销可接受**: 4x4 矩阵求逆仅需一次，现代 GPU 处理能力充足
4. **简化数据流**: C++ 端无需维护额外矩阵

### Shader 实现

**修改文件**: `Resource/ShaderSlang/Model/ModelUtil.slang`

```glsl
// 原代码
output.normal = normalize(mul(input.normal, data.matNormal));
output.tangent = normalize(mul(input.tangent, data.matNormal));

// 修改后
float3x3 matNormal = transpose(inverse((float3x3)data.matWorld));
output.normal = normalize(mul(input.normal, matNormal));
output.tangent = normalize(mul(input.tangent, matNormal));
```

### 优化提示

如果未来性能分析显示 `inverse()` 成为瓶颈，可考虑:
1. 假设 `matWorld` 为正交矩阵 (无非均匀缩放): `matNormal = (float3x3)matWorld`
2. 仅在 Instance 包含非均匀缩放时才计算 inverse (需添加标志位)
