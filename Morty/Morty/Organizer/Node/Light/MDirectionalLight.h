/**
 * @File         MDirectionalLight
 * 
 * @Created      2019-09-19 17:45:15
 *
 * @Author       DoubleYe
**/

#ifndef _M_MDIRECTIONALLIGHT_H_
#define _M_MDIRECTIONALLIGHT_H_
#include "MGlobal.h"
#include "MILight.h"
#include "Type/MColor.h"

class MORTY_API MDirectionalLight : public MILight
{
public:
	M_OBJECT(MDirectionalLight);
    MDirectionalLight();
    virtual ~MDirectionalLight();

public:

	void SetDirection(const Vector3& v3Direction)
	{
		m_v3Direction = v3Direction;
	}

	Vector3 GetDirection() { return m_v3Direction; }

	Vector3 GetWorldDirection()
	{
		return GetWorldTransform().GetRotatePart() * m_v3Direction;
	}

	void SetDiffuseColor(const MColor& color) { m_f3Diffuse = color; }
	MColor GetDiffuseColor() const { return m_f3Diffuse; }

	// For Phong Lightning
	void SetSpecularColor(const MColor& color) { m_f3Specular = color; }
	MColor GetSpecularColor() const { return m_f3Specular; }

	// For PBR Lightning
	void SetLightIntensity(const float& fIntensity) { m_fIntensity = fIntensity; }
	float GetLightIntensity() const { return m_fIntensity; }

private:

	Vector3 m_v3Direction;
	MColor m_f3Diffuse;
	MColor m_f3Specular;
	float m_fIntensity;

};


#endif
