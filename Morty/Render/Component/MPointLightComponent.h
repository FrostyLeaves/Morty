/**
 * @File         MPointLightComponent
 * 
 * @Created      2021-04-27 14:08:35
 *
 * @Author       Pobrecito
**/

#ifndef _M_MPOINTLIGHTCOMPONENT_H_
#define _M_MPOINTLIGHTCOMPONENT_H_
#include "MGlobal.h"
#include "MComponent.h"

#include "Vector.h"
#include "MColor.h"

class MORTY_API MPointLightComponent : public MComponent
{
public:
    MORTY_CLASS(MPointLightComponent)
public:
    MPointLightComponent();
    virtual ~MPointLightComponent();

public:

	void SetColor(const MColor& color) { m_f3Color = color; }
	MColor GetColor() { return m_f3Color; }

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

	virtual void WriteToStruct(MStruct& srt, MComponentRefTable& refTable) override;
	virtual void ReadFromStruct(const MStruct& srt, MComponentRefTable& refTable) override;

private:
	MColor m_f3Color;

	float m_fIntensity;
	float m_fConstant;
	float m_fLinear;
	float m_fQuadratic;
};


#endif
