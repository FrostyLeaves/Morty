# MModelConverter 重构迁移指南

## 概述

`MModelConverter` 已重构为两个专注的类：
- **`MMeshImporter`**: 负责 Mesh 数据导入
- **`MModelImporter`**: 负责模型导入和场景组织

## 重构动机

### 问题
- **单一类过于庞大**: 826 行代码，职责不清
- **难以测试**: Mesh 导入逻辑与场景组织耦合
- **代码复用困难**: 无法单独使用 Mesh 导入功能

### 解决方案
- **职责分离**: Mesh 导入 vs 模型导入
- **降低耦合**: 清晰的接口边界
- **提高复用性**: MMeshImporter 可独立使用

---

## 架构对比

### 旧架构 (MModelConverter)

```
MModelConverter (826行)
├─ ProcessMeshVertices()      // Mesh相关
├─ ProcessMeshIndices()        // Mesh相关
├─ BindVertexAndBones()        // Mesh相关
├─ ProcessNode()               // 场景组织
├─ ProcessMaterial()           // 材质处理
├─ ProcessTexture()            // 纹理处理
├─ ProcessBones()              // 骨骼处理
├─ ProcessAnimation()          // 动画处理
├─ ProcessLights()             // 灯光导入
└─ ProcessCameras()            // 相机导入
```

### 新架构 (拆分后)

```
MMeshImporter (~200行)
├─ ImportMesh()                // 主入口
├─ ProcessMeshVertices()       // 静态mesh顶点
├─ ProcessMeshVertices()       // 骨骼mesh顶点
├─ ProcessMeshIndices()        // 索引处理
└─ BindVertexAndBones()        // 骨骼权重绑定

MModelImporter (~600行)
├─ Import()                    // 主入口
├─ Load()                      // 加载Assimp场景
├─ ProcessNode()               // 场景节点 (使用MMeshImporter)
├─ ProcessMaterial()           // 材质处理
├─ ProcessTexture()            // 纹理处理
├─ ProcessBones()              // 骨骼处理
├─ ProcessAnimation()          // 动画处理
├─ ProcessLights()             // 灯光导入
├─ ProcessCameras()            // 相机导入
└─ SaveResources()             // 保存资源
```

---

## API 变更

### 旧 API (MModelConverter)

```cpp
MModelConverter converter(pEngine);

MModelConvertInfo info;
info.strResourcePath = "model.fbx";
info.strOutputDir = "output";
info.strOutputName = "model";
info.eMaterialType = MModelConvertMaterialType::E_PBR_Deferred;

converter.Convert(info);
```

### 新 API (MModelImporter)

```cpp
// 完全相同的API！
MModelImporter importer(pEngine);

MModelConvertInfo info;
info.strResourcePath = "model.fbx";
info.strOutputDir = "output";
info.strOutputName = "model";
info.eMaterialType = MModelConvertMaterialType::E_PBR_Deferred;

importer.Import(info);  // 只是方法名从 Convert 改为 Import
```

**关键变化**:
- 类名: `MModelConverter` → `MModelImporter`
- 方法名: `Convert()` → `Import()`
- 其他完全兼容！

---

## 迁移步骤

### 1. 更新包含头文件

**旧代码**:
```cpp
#include "Model/MModelConverter.h"
```

**新代码**:
```cpp
#include "Tools/MModelImporter.h"
```

### 2. 更新类名和方法名

**旧代码**:
```cpp
MModelConverter converter(pEngine);
converter.Convert(convertInfo);
```

**新代码**:
```cpp
MModelImporter importer(pEngine);
importer.Import(convertInfo);
```

### 3. 完成！

就这么简单！`MModelConvertInfo` 结构体和所有配置选项保持不变。

---

## 新功能：独立使用 MMeshImporter

### 单独导入 Mesh

现在您可以单独使用 `MMeshImporter` 导入 mesh，无需加载整个模型：

```cpp
#include "Tools/MMeshImporter.h"

MMeshImporter meshImporter(pEngine);

// 从 Assimp mesh 导入
MString meshName;
auto pMeshResource = meshImporter.ImportMesh(
    pAiMesh,         // aiMesh*
    pSkeleton,       // MSkeleton* (nullptr for static mesh)
    meshName         // 输出mesh名称
);

if (pMeshResource) {
    // 使用 mesh resource
}
```

### 使用场景

1. **自定义导入流程**: 需要特殊处理某些 mesh
2. **测试**: 单独测试 mesh 导入逻辑
3. **工具开发**: 构建自定义的 mesh 转换工具

---

## 代码组织

### 文件位置

```
Morty/Render/
├── Model/
│   └── MModelConverter.h/cpp  (旧，可标记为废弃)
└── Tools/
    ├── MMeshImporter.h         (新)
    ├── MMeshImporter.cpp       (新)
    ├── MModelImporter.h        (新)
    └── MModelImporter.cpp      (新)
```

### 建议

- **逐步迁移**: 新代码使用 `MModelImporter`
- **标记废弃**: 在 `MModelConverter` 上添加 `[[deprecated]]`
- **最终移除**: 确认所有代码迁移后，删除 `MModelConverter`

---

## 测试清单

### 兼容性测试

- [ ] 静态模型导入
- [ ] 骨骼模型导入
- [ ] PBR 材质导入
- [ ] Forward 材质导入
- [ ] 纹理导入（文件纹理）
- [ ] 纹理导入（嵌入纹理）
- [ ] 骨骼动画导入
- [ ] 灯光导入
- [ ] 相机导入
- [ ] 节点层级正确性

### 新功能测试

- [ ] MMeshImporter 单独使用
- [ ] 静态 mesh 导入
- [ ] 骨骼 mesh 导入

---

## 示例：完整迁移

### 旧代码 (Editor/Tools/ModelConverter.cpp)

```cpp
#include "Model/MModelConverter.h"

void ConvertModel(MEngine* pEngine, const MString& modelPath)
{
    MModelConverter converter(pEngine);

    MModelConvertInfo info;
    info.strResourcePath = modelPath;
    info.strOutputDir = "Resource/Models";
    info.strOutputName = "Character";
    info.bImportCamera = false;
    info.bImportLights = true;
    info.eMaterialType = MModelConvertMaterialType::E_PBR_Deferred;

    if (!converter.Convert(info)) {
        // 错误处理
    }
}
```

### 新代码 (Editor/Tools/ModelConverter.cpp)

```cpp
#include "Tools/MModelImporter.h"  // 只需要改这一行

void ConvertModel(MEngine* pEngine, const MString& modelPath)
{
    MModelImporter importer(pEngine);  // MModelConverter → MModelImporter

    MModelConvertInfo info;
    info.strResourcePath = modelPath;
    info.strOutputDir = "Resource/Models";
    info.strOutputName = "Character";
    info.bImportCamera = false;
    info.bImportLights = true;
    info.eMaterialType = MModelConvertMaterialType::E_PBR_Deferred;

    if (!importer.Import(info)) {  // Convert → Import
        // 错误处理
    }
}
```

---

## 优势总结

### 代码质量

| 指标 | 旧架构 | 新架构 |
|------|--------|--------|
| 单一职责 | ❌ 职责混杂 | ✅ 职责清晰 |
| 可测试性 | ⚠️ 困难 | ✅ 简单 |
| 代码复用 | ❌ 难以复用 | ✅ 高度复用 |
| 维护性 | ⚠️ 代码过长 | ✅ 模块化 |

### 性能

- ✅ **无性能损失**: 相同的实现逻辑
- ✅ **无额外开销**: 编译器内联优化

### 兼容性

- ✅ **API 兼容**: 只需更改类名和方法名
- ✅ **配置兼容**: `MModelConvertInfo` 不变
- ✅ **输出兼容**: 生成相同的资源文件

---

## FAQ

### Q: 是否需要修改现有的模型资源？
**A**: 不需要。重构只影响导入工具，不影响已导入的资源。

### Q: 性能会有影响吗？
**A**: 没有。重构是代码组织优化，不改变算法逻辑。

### Q: 可以同时使用新旧代码吗？
**A**: 可以。但建议统一迁移到新架构。

### Q: 如果只需要导入 mesh，使用哪个类？
**A**: 使用 `MMeshImporter`。它是独立的，不需要加载整个模型。

### Q: 迁移需要多久？
**A**: 通常 5-10 分钟。主要是查找替换类名和方法名。

---

## 支持

如有问题，请参考：
- 新实现: `Morty/Render/Tools/MModelImporter.cpp`
- 旧实现: `Morty/Render/Model/MModelConverter.cpp`

重构完成后，考虑在 `MModelConverter.h` 中添加：

```cpp
// Deprecated: Use MModelImporter instead
[[deprecated("Use MModelImporter from Tools/MModelImporter.h")]]
class MORTY_API MModelConverter
{
    // ...
};
```
