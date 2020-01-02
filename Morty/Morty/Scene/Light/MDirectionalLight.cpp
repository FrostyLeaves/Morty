#include "MDirectionalLight.h"

MTypeIdentifierImplement(MDirectionalLight, MILight)

MDirectionalLight::MDirectionalLight()
	: MILight()
	, m_v3Direction(0.0f, 0.0f, 1.0f)
	, m_f3Ambient(0.1f, 0.1f, 0.1f)
	, m_f3Diffuse(1.0f, 1.0f, 1.0f)
	, m_f3Specular(1.0f, 1.0f, 1.0f)
{

}

MDirectionalLight::~MDirectionalLight()
{

}
