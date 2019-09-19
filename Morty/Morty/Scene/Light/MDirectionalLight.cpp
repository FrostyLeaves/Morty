#include "MDirectionalLight.h"

MDirectionalLight::MDirectionalLight()
	: MILight()
{

}

MDirectionalLight::~MDirectionalLight()
{

}

void MDirectionalLight::SetDirection(const Vector3& v3Direction)
{
	m_v3Direction = v3Direction;
}
