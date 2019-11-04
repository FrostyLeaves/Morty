/**
 * @File         MPointLight
 * 
 * @Created      2019-09-19 12:37:04
 *
 * @Author       Morty
**/

#ifndef _M_MPOINTLIGHT_H_
#define _M_MPOINTLIGHT_H_
#include "MGlobal.h"
#include "MILight.h"

#include "Vector.h"

class MORTY_CLASS MPointLight : public MILight
{
public:
	M_OBJECT(MPointLight);
    MPointLight();
    virtual ~MPointLight();

public:

	void SetAmbientColor(const MColor& color) { m_f3Ambient = color; }
	MColor GetAmbientColor() { return m_f3Ambient; }

	void SetDiffuseColor(const MColor& color) { m_f3Diffuse = color; }
	MColor GetDiffuseColor() { return m_f3Diffuse; }

	void SetSpecularColor(const MColor& color) { m_f3Specular = color; }
	MColor GetSpecularColor() { return m_f3Specular; }

private:
	MColor m_f3Ambient;
	MColor m_f3Diffuse;
	MColor m_f3Specular;
};


#endif
