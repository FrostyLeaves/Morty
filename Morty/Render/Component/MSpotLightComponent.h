/**
 * @File         MSpotLightComponent
 * 
 * @Created      2021-04-27 14:08:51
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

class MORTY_API MSpotLightComponent : public MComponent
{
public:
    MORTY_CLASS(MSpotLightComponent)

public:
    MSpotLightComponent();

    virtual ~MSpotLightComponent();

    void    SetColor(const MColor& color) { m_color = color; }

    MColor  GetColor() const { return m_color; }

    void    SetColorVector(const Vector4& color) { m_color = color; }

    Vector4 GetColorVector() const { return m_color.ToVector4(); }

    // For PBR Lightning
    void    SetLightIntensity(const float& fIntensity) { m_intensity = fIntensity; }

    float   GetLightIntensity() const { return m_intensity; }

    void    SetInnerCutOff(const float& fCutOff);

    float   GetInnerCutOff() const { return m_innerCutOffAngle; }

    float   GetInnerCutOffRadius() const { return m_innerCutOffRadius; }

    void    SetOuterCutOff(const float& fCutOff);

    float   GetOuterCutOff() const { return m_outerCutOffAngle; }

    float   GetOuterCutOffRadius() const { return m_outerCutOffRadius; }

    Vector3 GetWorldDirection();

public:
    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;

    virtual void                      Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;

    virtual void                      Deserialize(const void* pBufferPointer) override;

private:
    MColor m_color;

    float  m_intensity;
    float  m_innerCutOffAngle;
    float  m_innerCutOffRadius;
    float  m_outerCutOffAngle;
    float  m_outerCutOffRadius;
};

}// namespace morty