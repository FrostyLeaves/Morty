/**
 * @File         MDirectionalLightComponent
 * 
 * @Created      2021-04-27 14:08:25
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Component/MComponent.h"

#include "Math/Vector.h"
#include "Utility/MColor.h"

namespace morty
{

class MORTY_API MDirectionalLightComponent : public MComponent
{
public:
    MORTY_CLASS(MDirectionalLightComponent)

public:
    MDirectionalLightComponent();

    virtual ~MDirectionalLightComponent();

    void    SetDirection(const Vector3& v3Direction);

    Vector3 GetDirection() { return m_direction; }

    Vector3 GetWorldDirection();

    void    SetColor(const MColor& color) { m_color = color; }

    MColor  GetColor() const { return m_color; }

    void    SetColorVector(const Vector4& color) { m_color = color; }

    Vector4 GetColorVector() const { return m_color.ToVector4(); }

    // For PBR Lightning
    void    SetLightIntensity(const float& fIntensity) { m_intensity = fIntensity; }

    float   GetLightIntensity() const { return m_intensity; }

    void    SetLightSize(const float& fLightSize) { m_lightSize = fLightSize; }

    float   GetLightSize() const { return m_lightSize; }

    void    SetLightEnable(const bool& bLightEnable) { m_enable = bLightEnable; }

    bool    GetLightEnable() const { return m_enable; }

public:
    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;

    virtual void                      Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;

    virtual void                      Deserialize(const void* pBufferPointer) override;

private:
    Vector3 m_direction;
    MColor  m_color;
    float   m_intensity;
    float   m_lightSize = 1.0f;
    bool    m_enable    = true;
};

}// namespace morty