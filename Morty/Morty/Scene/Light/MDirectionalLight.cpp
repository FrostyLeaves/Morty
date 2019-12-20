#include "MDirectionalLight.h"

MDirectionalLight::MDirectionalLight()
	: MILight()
	, m_v3Direction(0.0f, 0.0f, 1.0f)
	, m_f3Ambient(0.1f, 0.f, 0.1f)
	, m_f3Diffuse(0.1f, 0.f, 0.1f)
	, m_f3Specular(0.1f, 0.f, 0.1f)
{

}

MDirectionalLight::~MDirectionalLight()
{

}

void MDirectionalLight::SetDirection(const Vector3& v3Direction)
{
	m_v3Direction = v3Direction;
}
