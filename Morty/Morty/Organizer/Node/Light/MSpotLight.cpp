#include "MSpotLight.h"

MTypeIdentifierImplement(MSpotLight, MILight)

MSpotLight::MSpotLight()
	: MILight()
	, m_f3Diffuse(1.0f, 1.0f, 1.0f)
	, m_f3Specular(1.0f, 1.0f, 1.0f)
	, m_fInnerCutOffAngle(90.0f)
	, m_fOuterCutOffAngle(90.0f)
{
	m_fInnerCutOffRadius = cos(m_fInnerCutOffAngle * 0.5f * M_PI / 180.0f);
	m_fOuterCutOffRadius = cos(m_fOuterCutOffAngle * 0.5f * M_PI / 180.0f);
}

MSpotLight::~MSpotLight()
{

}

void MSpotLight::SetInnerCutOff(const float& fCutOff)
{
	m_fInnerCutOffAngle = fCutOff;
	m_fInnerCutOffRadius = cos(m_fInnerCutOffAngle * 0.5f * M_PI / 180.0f);

	if (fCutOff - m_fOuterCutOffAngle > 1e-6f)
		SetOuterCutOff(fCutOff);
}

void MSpotLight::SetOuterCutOff(const float& fCutOff)
{
	m_fOuterCutOffAngle = fCutOff;
	m_fOuterCutOffRadius = cos(m_fOuterCutOffAngle * 0.5f * M_PI / 180.0f);

	if (fCutOff - m_fInnerCutOffAngle < -1e-6f)
		SetInnerCutOff(fCutOff);
}
