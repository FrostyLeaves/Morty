# Model Import Tools

模型导入工具集 - 从 Assimp 导入 3D 模型到 Morty 引擎。

## 组件

### MMeshImporter
专注于 Mesh 数据导入：
- 顶点处理（静态/骨骼）
- 索引处理
- 骨骼权重绑定
- 生成 MMeshResource

### MModelImporter
完整模型导入：
- 场景加载
- 材质/纹理处理
- 骨骼/动画处理
- 灯光/相机导入
- Entity 层级组织
- 资源保存

## 快速开始

### 导入完整模型

```cpp
#include "Tools/MModelImporter.h"

MModelImporter importer(engine);

MModelConvertInfo info;
info.strResourcePath = "model.fbx";
info.strOutputDir = "Resource/Models";
info.strOutputName = "Character";
info.eMaterialType = MModelConvertMaterialType::E_PBR_Deferred;
info.bImportCamera = false;
info.bImportLights = true;

importer.Import(info);
```

### 单独导入 Mesh

```cpp
#include "Tools/MMeshImporter.h"

MMeshImporter meshImporter(engine);

MString meshName;
auto pMeshResource = meshImporter.ImportMesh(
    pAiMesh,    // Assimp mesh
    pSkeleton,  // nullptr for static mesh
    meshName    // 输出名称
);
```

## 支持的格式

通过 Assimp 支持：
- FBX
- OBJ
- GLTF/GLB
- DAE (Collada)
- 3DS
- Blend
- 等等...

## 材质类型

### E_PBR_Deferred
PBR 延迟渲染材质：
- Albedo (BaseColor)
- Normal
- Metallic
- Roughness
- Ambient Occlusion
- Emission

### E_Default_Forward
传统前向渲染材质：
- Diffuse
- Specular
- Normal
- Ambient

## 输出结构

```
Resource/Models/Character/
├── Character.entity       # 场景层级
├── Character.ske          # 骨骼
├── mesh_0.mesh            # Mesh资源
├── mesh_1.mesh
├── material_0.mat         # 材质
├── texture_0.mtex         # 纹理
└── anim_0.anim            # 动画
```

## 高级功能

### 自定义纹理处理

```cpp
class MyTextureDelegate : public MITextureDelegate {
    std::shared_ptr<MTextureResource> GetTexture(
        const MString& strFullPath,
        MEModelTextureUsage eUsage
    ) override {
        // 自定义纹理加载逻辑
        // 例如：压缩、格式转换、LOD生成
    }
};

info.pTextureDelegate = std::make_shared<MyTextureDelegate>();
```

### 自定义材质后处理

```cpp
class MyMaterialDelegate : public MIMaterialDelegate {
    void PostProcess(MMaterial* pMaterial) override {
        // 材质后处理
        // 例如：调整参数、添加特殊属性
    }
};

info.pMaterialDelegate = std::make_shared<MyMaterialDelegate>();
```

## 从 MModelConverter 迁移

详见 [MIGRATION_GUIDE.md](./MIGRATION_GUIDE.md)

简要步骤：
1. `#include "Model/MModelConverter.h"` → `#include "Tools/MModelImporter.h"`
2. `MModelConverter` → `MModelImporter`
3. `Convert()` → `Import()`

## 文件

```
Tools/
├── MMeshImporter.h         # Mesh导入器
├── MMeshImporter.cpp
├── MModelImporter.h        # 模型导入器
├── MModelImporter.cpp
├── README.md               # 本文件
└── MIGRATION_GUIDE.md      # 迁移指南
```

## 架构

```
┌──────────────────┐
│ MModelImporter   │
│  (场景组织)       │
└────────┬─────────┘
         │ uses
         ▼
┌──────────────────┐
│ MMeshImporter    │
│  (Mesh导入)       │
└──────────────────┘
```

## 作者

DoubleYe - 2025-01-14
