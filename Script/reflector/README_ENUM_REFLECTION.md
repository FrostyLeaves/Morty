# Enum Reflection 枚举反射

## 概述

这个反射框架现在支持收集带有 `MORTY_ENUM` 属性的枚举类型，自动提取枚举名和枚举值，用于组件属性编辑器。

## 使用方法

### 1. 定义枚举类型

在C++代码中使用 `MORTY_ENUM` 宏来标注需要被收集的枚举：

```cpp
// 在 MGlobal.h 中已定义:
#define MORTY_ENUM [[clang::annotate("MORTY_ENUM")]] enum class

// 使用示例:
MORTY_ENUM MEShadowType {
    ENone            = 0,
    EOnlyDirectional = 1,
    EAllLights       = 2,
};
```

### 2. 在组件属性中使用枚举

使用 `PROPERTY_ENUM` 宏标注枚举类型的成员变量：

```cpp
class MRenderMeshComponent : public MComponent {
protected:
    PROPERTY_ENUM MEShadowType m_shadowType;
};
```

### 3. 自动生成的代码

反射系统会自动：

1. **收集枚举定义**：从带有 `MORTY_ENUM` 的枚举中提取所有枚举值及其数值
2. **关联属性**：将 `PROPERTY_ENUM` 标注的属性与对应的枚举类型关联
3. **生成编辑器代码**：自动生成属性编辑器代码，包含枚举值列表

生成的代码示例：

```cpp
PROPERTY_VALUE_GET_SET_EDIT(
    component,
    "ShadowType",           // 显示名称
    MEShadowType,           // 枚举类型
    GetShadowType,          // Getter方法
    SetShadowType,          // Setter方法
    {"None", "OnlyDirectional", "AllLights"}  // 枚举值列表（自动生成）
);
```

## 实现细节

### 文件结构

- **enum_collector.py**: 枚举收集器，遍历AST收集所有 `MORTY_ENUM` 标注的枚举
- **component_property_collector.py**: 组件属性收集器，使用枚举收集器获取枚举值
- **generate_reflector.py**: 主入口，协调各个收集器

### 工作流程

1. **枚举收集阶段**：
   - `enum_collector` 扫描所有带有 `MORTY_ENUM` 属性的枚举定义
   - 提取枚举名称和所有枚举值（名称+数值）
   - 存储到 `m_enum_table` 字典中

2. **属性收集阶段**：
   - `component_property_collector` 扫描所有带有 `ComponentPropertyEnum` 的成员变量
   - 通过枚举类型名查询 `enum_collector` 获取枚举值列表
   - 将枚举值填充到 `prop_info.enum_values`

3. **代码生成阶段**：
   - 将枚举值字典转换为C++初始化列表格式
   - 自动移除枚举值前缀（如 `ENone` -> `None`）
   - 按枚举值排序，保持正确顺序

### 枚举值格式化

`enum_to_array()` 方法会：
- 按枚举数值排序，保持定义顺序
- 自动移除常见的枚举前缀（如 `E` 开头）
- 转换为C++字符串数组格式：`{"Value1", "Value2", ...}`

## 输出文件

- **MComponentProperty.gen**: 生成的组件属性编辑器代码
- **MEnumReflection.gen**: 枚举反射信息（用于调试）

## 注意事项

1. **枚举必须用 `MORTY_ENUM` 标注**：只有带此属性的枚举才会被收集
2. **支持嵌套枚举**：支持类内枚举（如 `MRenderMeshComponent::MEShadowType`）
3. **自动名称处理**：
   - 成员变量：`m_shadowType` -> 显示名 `ShadowType`
   - Getter/Setter：自动生成 `GetShadowType` / `SetShadowType`
   - 枚举值：`ENone` -> 显示为 `None`

## 示例

完整示例请参考：
- 枚举定义：[MRenderMeshComponent.h:40-44](../../Morty/Render/Component/MRenderMeshComponent.h#L40-L44)
- 属性标注：[MRenderMeshComponent.h:90](../../Morty/Render/Component/MRenderMeshComponent.h#L90)
- 手写版本（对比）：[PropertyMRenderMeshComponent.h:85-90](../../Morty/Editor/Property/PropertyMRenderMeshComponent.h#L85-L90)
