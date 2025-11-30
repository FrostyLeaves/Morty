# 枚举反射使用示例

## 示例 1：简单枚举

```cpp
// 定义枚举
MORTY_ENUM MERenderMode {
    EForward  = 0,
    EDeferred = 1,
    ERayTracing = 2,
};

// 在组件中使用
class MRenderComponent : public MComponent {
protected:
    PROPERTY_ENUM MERenderMode m_renderMode;

public:
    void SetRenderMode(const MERenderMode& mode) { m_renderMode = mode; }
    MERenderMode GetRenderMode() const { return m_renderMode; }
};
```

**生成的编辑器代码**：
```cpp
PROPERTY_VALUE_GET_SET_EDIT(
    component,
    "RenderMode",
    MERenderMode,
    GetRenderMode,
    SetRenderMode,
    {"Forward", "Deferred", "RayTracing"}
);
```

## 示例 2：类内嵌套枚举（当前使用的方式）

```cpp
class MRenderMeshComponent : public MComponent {
public:
    // 定义嵌套枚举
    MORTY_ENUM MEShadowType {
        ENone            = 0,
        EOnlyDirectional = 1,
        EAllLights       = 2,
    };

protected:
    // 使用嵌套枚举
    PROPERTY_ENUM MEShadowType m_shadowType;

public:
    void SetShadowType(const MEShadowType& type) { m_shadowType = type; }
    MEShadowType GetShadowType() const { return m_shadowType; }
};
```

**生成的编辑器代码**：
```cpp
PROPERTY_VALUE_GET_SET_EDIT(
    component,
    "ShadowType",
    MRenderMeshComponent::MEShadowType,
    GetShadowType,
    SetShadowType,
    {"None", "OnlyDirectional", "AllLights"}
);
```

## 示例 3：多个枚举属性

```cpp
class MLightComponent : public MComponent {
public:
    MORTY_ENUM MELightType {
        EDirectional = 0,
        EPoint       = 1,
        ESpot        = 2,
    };

    MORTY_ENUM MEShadowQuality {
        ELow    = 0,
        EMedium = 1,
        EHigh   = 2,
        EUltra  = 3,
    };

protected:
    PROPERTY_ENUM MELightType m_lightType;
    PROPERTY_ENUM MEShadowQuality m_shadowQuality;

public:
    void SetLightType(const MELightType& type) { m_lightType = type; }
    MELightType GetLightType() const { return m_lightType; }

    void SetShadowQuality(const MEShadowQuality& quality) { m_shadowQuality = quality; }
    MEShadowQuality GetShadowQuality() const { return m_shadowQuality; }
};
```

**生成的编辑器代码**：
```cpp
PROPERTY_VALUE_GET_SET_EDIT(component, "LightType", MELightType,
    GetLightType, SetLightType, {"Directional", "Point", "Spot"});

PROPERTY_VALUE_GET_SET_EDIT(component, "ShadowQuality", MEShadowQuality,
    GetShadowQuality, SetShadowQuality, {"Low", "Medium", "High", "Ultra"});
```

## 命名约定

### 成员变量命名
- 格式：`m_variableName`
- 示例：`m_shadowType`, `m_renderMode`
- 自动转换为显示名：`ShadowType`, `RenderMode`

### 枚举值命名
- 推荐格式：`EValueName`（E前缀）
- 示例：`ENone`, `EOnlyDirectional`, `EAllLights`
- 自动移除前缀显示为：`None`, `OnlyDirectional`, `AllLights`
- 也支持无前缀的枚举值，如：`Low`, `Medium`, `High`

### Getter/Setter命名
- 自动生成格式：`Get{PropertyName}()` / `Set{PropertyName}()`
- 示例：
  - `m_shadowType` -> `GetShadowType()` / `SetShadowType()`
  - `m_renderMode` -> `GetRenderMode()` / `SetRenderMode()`

## 与手写代码对比

### 手写版本（旧方式）
```cpp
m_editProperty.ShowValueBegin("ShadowType");
MRenderMeshComponent::MEShadowType eType = meshComponent->GetShadowType();
auto nSelected = (size_t)eType;
if (m_editProperty.EditEnum({"None", "OnlyDirection", "AllLights"}, nSelected)) {
    meshComponent->SetShadowType((MRenderMeshComponent::MEShadowType)nSelected);
}
m_editProperty.ShowValueEnd();
```

### 自动生成版本（新方式）
```cpp
PROPERTY_VALUE_GET_SET_EDIT(
    component, "ShadowType", MEShadowType,
    GetShadowType, SetShadowType,
    {"None", "OnlyDirectional", "AllLights"}  // 自动从枚举定义中提取
);
```

## 运行反射生成器

```bash
# 在项目根目录运行
python Script/reflector/generate_reflector.py <source_path> <build_dir>

# 生成的文件位置：
# - Morty/Editor/Reflection/MComponentProperty.gen
# - Morty/Editor/Reflection/MEnumReflection.gen
```
