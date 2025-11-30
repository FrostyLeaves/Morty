# 组件属性反射系统使用说明

## 概述

这个反射系统可以自动生成组件属性编辑器代码，避免手动编写像 `PropertyMRenderMeshComponent.h` 这样的重复代码。

## 使用方法

### 1. 在组件头文件中添加反射宏定义

首先在组件头文件中定义反射宏：

```cpp
#define REFL_COMPONENT_PROPERTY [[clang::annotate("ComponentProperty")]]
```

### 2. 标注需要反射的属性

在组件类的成员变量前添加 `REFL_COMPONENT_PROPERTY` 标注：

```cpp
class MORTY_API MRenderMeshComponent : public MComponent
{
protected:
    // 不需要编辑器的属性不加标注
    MResourceRef m_mesh;

    // 需要在编辑器中显示的属性加上标注
    REFL_COMPONENT_PROPERTY MResourceRef m_material;
    REFL_COMPONENT_PROPERTY MEShadowType m_shadowType;
    REFL_COMPONENT_PROPERTY bool m_generateDirLightShadow = true;
};
```

### 3. 确保有对应的 Getter/Setter 方法

反射系统会根据成员变量名自动推导 Getter/Setter 方法名：

- 成员变量：`m_shadowType` → Getter: `GetShadowType()`, Setter: `SetShadowType()`
- 成员变量：`m_material` → Getter: `GetMaterial()`, Setter: `SetMaterial()`

```cpp
public:
    void SetShadowType(const MEShadowType& eType) { m_shadowType = eType; }
    MEShadowType GetShadowType() { return m_shadowType; }

    void SetMaterial(const std::shared_ptr<MMaterialResource>& material);
    std::shared_ptr<MMaterialResource> GetMaterial() const;
```

### 4. 运行反射代码生成器

运行 `generate_reflector.py` 脚本：

```bash
python Script/reflector/generate_reflector.py <source_path> <build_dir>
```

生成的代码会输出到 `Editor/Reflection/MComponentProperty.gen`

## 支持的属性类型

### Bool 类型
```cpp
REFL_COMPONENT_PROPERTY bool m_generateDirLightShadow;
```

生成的代码：
```cpp
PROPERTY_VALUE_GET_SET_EDIT(
    component,
    "GenerateDirLightShadow",
    bool,
    GetGenerateDirLightShadow,
    SetGenerateDirLightShadow
);
```

### 枚举类型
```cpp
enum class MEShadowType
{
    ENone = 0,
    EOnlyDirectional = 1,
    EAllLights = 2,
};

REFL_COMPONENT_PROPERTY MEShadowType m_shadowType;
```

生成的代码：
```cpp
m_editProperty.ShowValueBegin("ShadowType");
MRenderMeshComponent::MEShadowType eValue = component->GetShadowType();
auto nSelected = (size_t)eValue;
if (m_editProperty.EditEnum({"None", "OnlyDirectional", "AllLights"}, nSelected))
{
    component->SetShadowType((MRenderMeshComponent::MEShadowType)nSelected);
}
m_editProperty.ShowValueEnd();
```

注意：目前枚举值需要在 collector 中手动配置，未来可以扩展为自动解析。

### 资源类型
```cpp
REFL_COMPONENT_PROPERTY MResourceRef m_material;
```

生成的代码：
```cpp
m_editProperty.ShowValueBegin("Material");
auto pResource = component->GetMaterial();
if (m_editProperty.EditMResource(
    "Material_file_dlg",
    MMaterialResourceLoader::GetResourceTypeName(),
    MMaterialResourceLoader::GetSuffixList(),
    pResource
))
{
    if (pResource) { component->SetMaterial(pResource); }
}
m_editProperty.ShowValueEnd();
```

## 生成的代码结构

生成的文件包含两部分：

1. **属性编辑器类**：为每个组件生成一个 `Property{ComponentName}` 类
2. **工厂映射**：自动注册到 `MComponentPropertyList::PropertyFactory`

```cpp
#include "Component/MRenderMeshComponent.h"
class PropertyMRenderMeshComponent : public MComponentProperty
{
public:
    void EditEntity(MainEditor* editor, MEntity* pEntity) override
    {
        // ... 自动生成的编辑器代码
    }
};

const std::unordered_map<MStringId, MComponentPropertyList::PropertyCreateFunc>
MComponentPropertyList::PropertyFactory = {
    { MStringId("MRenderMeshComponent"), []() {
        return static_cast<MComponentProperty*>(new PropertyMRenderMeshComponent());
    }},
};
```

## 与现有系统的对比

### 手动方式（旧）
- 需要手写整个 `PropertyMRenderMeshComponent.h` 文件
- 添加新属性需要手动更新编辑器代码
- 容易出错，代码重复

### 反射方式（新）
- 只需要在属性上添加 `REFL_COMPONENT_PROPERTY` 标注
- 运行脚本自动生成编辑器代码
- 减少重复代码，降低出错概率

## 示例

参考 [MRenderMeshComponent.h](../../Render/Component/MRenderMeshComponent.h) 的实现：

```cpp
#define REFL_COMPONENT_PROPERTY [[clang::annotate("ComponentProperty")]]

class MORTY_API MRenderMeshComponent : public MComponent
{
public:
    enum class MEShadowType
    {
        ENone = 0,
        EOnlyDirectional = 1,
        EAllLights = 2,
    };

    void SetShadowType(const MEShadowType& eType);
    MEShadowType GetShadowType();

    void SetMaterial(const std::shared_ptr<MMaterialResource>& material);
    std::shared_ptr<MMaterialResource> GetMaterial() const;

    void SetGenerateDirLightShadow(const bool& bGenerate);
    bool GetGenerateDirLightShadow() const;

protected:
    REFL_COMPONENT_PROPERTY MResourceRef m_material;
    REFL_COMPONENT_PROPERTY MEShadowType m_shadowType;
    REFL_COMPONENT_PROPERTY bool m_generateDirLightShadow = true;
};
```

## 扩展

如果需要支持更多类型，可以修改 [component_property_collector.py](component_property_collector.py)：

1. 在 `parse_property_type()` 中添加新类型的识别逻辑
2. 在 `generate_property_code()` 中添加新类型的代码生成模板
3. 添加对应的模板字符串

## 注意事项

1. 确保成员变量命名遵循 `m_propertyName` 格式
2. Getter/Setter 方法必须遵循 `Get{PropertyName}()`、`Set{PropertyName}()` 命名规则
3. 枚举类型目前需要手动配置枚举值（可以扩展为自动解析）
4. 生成的代码会覆盖之前的 `.gen` 文件
