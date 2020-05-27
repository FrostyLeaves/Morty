#include "MPointLight.h"

M_OBJECT_IMPLEMENT(MPointLight, MILight)

MPointLight::MPointLight()
	: MILight()
	, m_f3Diffuse(1.0f, 1.0f, 1.0f)
	, m_f3Specular(1.0f, 1.0f, 1.0f)
{

}

MPointLight::~MPointLight()
{

}
