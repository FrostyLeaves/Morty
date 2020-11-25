#include "MPointLight.h"

M_OBJECT_IMPLEMENT(MPointLight, MILight)

MPointLight::MPointLight()
	: MILight()
	, m_f3Diffuse(1.0f, 1.0f, 1.0f)
	, m_f3Specular(1.0f, 1.0f, 1.0f)
	, m_fConstant(1.0f)
	, m_fLinear(0.022f)
	, m_fQuadratic(0.0019f)
{

}

MPointLight::~MPointLight()
{

}
