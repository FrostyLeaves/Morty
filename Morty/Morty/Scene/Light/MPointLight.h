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

	void SetAmbientColor(const ColorRGB& color) { m_f3Ambient = color; }
	ColorRGB GetAmbientColor() { return m_f3Ambient; }

	void SetDiffuseColor(const ColorRGB& color) { m_f3Diffuse = color; }
	ColorRGB GetDiffuseColor() { return m_f3Diffuse; }

	void SetSpecularColor(const ColorRGB& color) { m_f3Specular = color; }
	ColorRGB GetSpecularColor() { return m_f3Specular; }

private:
	ColorRGB m_f3Ambient;
	ColorRGB m_f3Diffuse;
	ColorRGB m_f3Specular;
};


#endif
