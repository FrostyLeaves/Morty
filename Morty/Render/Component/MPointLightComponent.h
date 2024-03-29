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

MORTY_SPACE_BEGIN

class MORTY_API MPointLightComponent : public MComponent
{
public:
    MORTY_CLASS(MPointLightComponent)
public:
    MPointLightComponent();
    virtual ~MPointLightComponent();

public:

	void SetColor(const MColor& color) { m_f3Color = color; }
	MColor GetColor() const { return m_f3Color; }

	void SetColorVector(const Vector4& color) { m_f3Color = color; }
	Vector4 GetColorVector() const { return m_f3Color.ToVector4(); }

	// For PBR Lightning
	void SetLightIntensity(const float& fIntensity) { m_fIntensity = fIntensity; }
	float GetLightIntensity() const { return m_fIntensity; }

	void SetConstant(const float& fValue) { m_fConstant = fValue; }
	float GetConstant() const { return m_fConstant; }

	void SetLinear(const float& fValue) { m_fLinear = fValue; }
	float GetLinear() const { return m_fLinear; }

	void SetQuadratic(const float& fValue) { m_fQuadratic = fValue; }
	float GetQuadratic() const { return m_fQuadratic; }

public:

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;
	virtual void Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;
	virtual void Deserialize(const void* pBufferPointer) override;

private:
	MColor m_f3Color;

	float m_fIntensity;
	float m_fConstant;
	float m_fLinear;
	float m_fQuadratic;
};

MORTY_SPACE_END