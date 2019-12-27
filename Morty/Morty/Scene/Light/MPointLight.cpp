#include "MPointLight.h"

MTypeIdentifierImplement(MPointLight, MILight)

MPointLight::MPointLight()
	: MILight()
	, m_f3Ambient(1.0f, 1.0f, 1.0f)
	, m_f3Diffuse(1.0f, 1.0f, 1.0f)
	, m_f3Specular(1.0f, 1.0f, 1.0f)
{

}

MPointLight::~MPointLight()
{

}
