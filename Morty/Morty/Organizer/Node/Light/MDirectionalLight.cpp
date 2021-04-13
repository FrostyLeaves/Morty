#include "MDirectionalLight.h"

M_OBJECT_IMPLEMENT(MDirectionalLight, MILight)

MDirectionalLight::MDirectionalLight()
	: MILight()
	, m_v3Direction(0.0f, 0.0f, 1.0f)
	, m_f3Diffuse(1.0f, 1.0f, 1.0f)
	, m_f3Specular(1.0f, 1.0f, 1.0f)
	, m_fIntensity(1.0f)
{
	//SetRotation(Quaternion(Vector3(1, 0, 0), 90.0f));
}

MDirectionalLight::~MDirectionalLight()
{

}
