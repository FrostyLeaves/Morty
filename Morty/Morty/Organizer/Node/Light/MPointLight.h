/**
 * @File         MPointLight
 * 
 * @Created      2019-09-19 12:37:04
 *
 * @Author       DoubleYe
**/

#ifndef _M_MPOINTLIGHT_H_
#define _M_MPOINTLIGHT_H_
#include "MGlobal.h"
#include "MILight.h"
#include "Type/MType.h"
#include "Vector.h"

class MORTY_API MPointLight : public MILight
{
public:
	M_OBJECT(MPointLight);
    MPointLight();
    virtual ~MPointLight();

public:

	void SetDiffuseColor(const MColor& color) { m_f3Diffuse = color; }
	MColor GetDiffuseColor() { return m_f3Diffuse; }

	void SetSpecularColor(const MColor& color) { m_f3Specular = color; }
	MColor GetSpecularColor() { return m_f3Specular; }


	void SetConstant(const float& fValue) { m_fConstant = fValue; }

	float GetConstant() const { return m_fConstant; }

	void SetLinear(const float& fValue) { m_fLinear = fValue; }
	float GetLinear() const { return m_fLinear; }

	void SetQuadratic(const float& fValue) { m_fQuadratic = fValue; }
	float GetQuadratic() const { return m_fQuadratic; }

private:
	MColor m_f3Diffuse;
	MColor m_f3Specular;

	float m_fConstant;
	float m_fLinear;
	float m_fQuadratic;
};


#endif
