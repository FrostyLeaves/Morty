/**
 * @File         MPointLightComponent
 * 
 * @Created      2021-04-27 14:08:35
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

class MORTY_API MPointLightComponent : public MComponent
{
public:
    MORTY_CLASS(MPointLightComponent)
public:
    MPointLightComponent();

    virtual ~MPointLightComponent();

public:
    void    SetColor(const MColor& color) { m_color = color; }

    MColor  GetColor() const { return m_color; }

    void    SetColorVector(const Vector4& color) { m_color = color; }

    Vector4 GetColorVector() const { return m_color.ToVector4(); }

    // For PBR Lightning
    void    SetLightIntensity(const float& fIntensity) { m_intensity = fIntensity; }

    float   GetLightIntensity() const { return m_intensity; }

    void    SetConstant(const float& fValue) { m_constant = fValue; }

    float   GetConstant() const { return m_constant; }

    void    SetLinear(const float& fValue) { m_linear = fValue; }

    float   GetLinear() const { return m_linear; }

    void    SetQuadratic(const float& fValue) { m_quadratic = fValue; }

    float   GetQuadratic() const { return m_quadratic; }

public:
    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;

    virtual void                      Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;

    virtual void                      Deserialize(const void* pBufferPointer) override;

private:
    MColor m_color;

    float  m_intensity;
    float  m_constant;
    float  m_linear;
    float  m_quadratic;
};

}// namespace morty