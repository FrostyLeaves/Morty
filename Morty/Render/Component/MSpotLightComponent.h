/**
 * @File         MSpotLightComponent
 * 
 * @Created      2021-04-27 14:08:51
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSPOTLIGHTCOMPONENT_H_
#define _M_MSPOTLIGHTCOMPONENT_H_
#include "MGlobal.h"
#include "MComponent.h"

#include "Vector.h"
#include "MColor.h"

class MORTY_API MSpotLightComponent : public MComponent
{
public:
    MORTY_CLASS(MSpotLightComponent)

public:
    MSpotLightComponent();
    virtual ~MSpotLightComponent();

	void SetColor(const MColor& color) { m_f3Color = color; }
	MColor GetColor() const { return m_f3Color; }

	void SetColorVector(const Vector4& color) { m_f3Color = color; }
	Vector4 GetColorVector() const { return m_f3Color.ToVector4(); }

	// For PBR Lightning
	void SetLightIntensity(const float& fIntensity) { m_fIntensity = fIntensity; }
	float GetLightIntensity() const { return m_fIntensity; }

	void SetInnerCutOff(const float& fCutOff);
	float GetInnerCutOff() const { return m_fInnerCutOffAngle; }
	float GetInnerCutOffRadius() const { return m_fInnerCutOffRadius; }

	void SetOuterCutOff(const float& fCutOff);
	float GetOuterCutOff() const { return m_fOuterCutOffAngle; }
	float GetOuterCutOffRadius() const { return m_fOuterCutOffRadius; }

	Vector3 GetWorldDirection();

public:

	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(const MStruct& srt) override;

private:
	MColor m_f3Color;

	float m_fIntensity;
	float m_fInnerCutOffAngle;
	float m_fInnerCutOffRadius;
	float m_fOuterCutOffAngle;
	float m_fOuterCutOffRadius;
};


#endif
