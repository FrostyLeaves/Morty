/**
 * @File         MSpotLight
 * 
 * @Created      2019-09-19 17:48:28
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSPOTLIGHT_H_
#define _M_MSPOTLIGHT_H_
#include "MGlobal.h"
#include "MILight.h"
#include "Type/MType.h"
#include "Vector.h"

class MORTY_API MSpotLight : public MILight
{
public:
	M_OBJECT(MSpotLight);
    MSpotLight();
    virtual ~MSpotLight();


public:
	void SetDiffuseColor(const MColor& color) { m_f3Diffuse = color; }
	MColor GetDiffuseColor() const { return m_f3Diffuse; }

	void SetSpecularColor(const MColor& color) { m_f3Specular = color; }
	MColor GetSpecularColor() const { return m_f3Specular; }

	void SetInnerCutOff(const float& fCutOff);
	float GetInnerCutOff() const { return m_fInnerCutOffAngle; }
	float GetInnerCutOffRadius() const { return m_fInnerCutOffRadius; }

	void SetOuterCutOff(const float& fCutOff);
	float GetOuterCutOff() const { return m_fOuterCutOffAngle; }
	float GetOuterCutOffRadius() const { return m_fOuterCutOffRadius; }

	Vector3 GetWorldDirection()
	{
		return GetWorldTransform().GetRotatePart() * Vector3(0.0f, 0.0f, 1.0f);
	}

private:
	MColor m_f3Diffuse;
	MColor m_f3Specular;

	float m_fInnerCutOffAngle;
	float m_fInnerCutOffRadius;
	float m_fOuterCutOffAngle;
	float m_fOuterCutOffRadius;

};


#endif
