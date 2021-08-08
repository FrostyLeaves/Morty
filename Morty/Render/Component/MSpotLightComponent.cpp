#include "MSpotLightComponent.h"

MORTY_CLASS_IMPLEMENT(MSpotLightComponent, MComponent)

#include "MEntity.h"
#include "MSceneComponent.h"

MSpotLightComponent::MSpotLightComponent()
	: MComponent()
	, m_f3Color(1.0f, 1.0f, 1.0f)
	, m_fIntensity(1.0f)
	, m_fInnerCutOffAngle(90.0f)
	, m_fOuterCutOffAngle(90.0f)
{
	m_fInnerCutOffRadius = cos(m_fInnerCutOffAngle * 0.5f * M_PI / 180.0f);
	m_fOuterCutOffRadius = cos(m_fOuterCutOffAngle * 0.5f * M_PI / 180.0f);
}

MSpotLightComponent::~MSpotLightComponent()
{

}

void MSpotLightComponent::SetInnerCutOff(const float& fCutOff)
{
	m_fInnerCutOffAngle = fCutOff;
	m_fInnerCutOffRadius = cos(m_fInnerCutOffAngle * 0.5f * M_PI / 180.0f);

	if (fCutOff - m_fOuterCutOffAngle > 1e-6f)
		SetOuterCutOff(fCutOff);
}

void MSpotLightComponent::SetOuterCutOff(const float& fCutOff)
{
	m_fOuterCutOffAngle = fCutOff;
	m_fOuterCutOffRadius = cos(m_fOuterCutOffAngle * 0.5f * M_PI / 180.0f);

	if (fCutOff - m_fInnerCutOffAngle < -1e-6f)
		SetInnerCutOff(fCutOff);
}

Vector3 MSpotLightComponent::GetWorldDirection()
{
	MEntity* pEntity = GetEntity();
	if (!pEntity)
		return Vector3(0.0f, 0.0f, 1.0f);

	MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
		return Vector3(0.0f, 0.0f, 1.0f);

	return pSceneComponent->GetWorldTransform().GetRotatePart() * Vector3(0.0f, 0.0f, 1.0f);
}

void MSpotLightComponent::WriteToStruct(MStruct& srt, MComponentRefTable& refTable)
{
	Super::WriteToStruct(srt, refTable);

	M_SERIALIZER_WRITE_BEGIN;

	M_SERIALIZER_WRITE_VALUE("Color", GetColorVector);
	M_SERIALIZER_WRITE_VALUE("LightIntensity", GetLightIntensity);
	M_SERIALIZER_WRITE_VALUE("InnerCutOffAngle", GetInnerCutOff);
	M_SERIALIZER_WRITE_VALUE("OuterCutOffAngle", GetOuterCutOff);

	M_SERIALIZER_END;
}

void MSpotLightComponent::ReadFromStruct(const MStruct& srt, MComponentRefTable& refTable)
{
	Super::ReadFromStruct(srt, refTable);

	M_SERIALIZER_READ_BEGIN;

	M_SERIALIZER_READ_VALUE("Color", SetColorVector, Vector4);
	M_SERIALIZER_READ_VALUE("LightIntensity", SetLightIntensity, Float);
	M_SERIALIZER_READ_VALUE("InnerCutOffAngle", SetInnerCutOff, Float);
	M_SERIALIZER_READ_VALUE("OuterCutOffAngle", SetOuterCutOff, Float);

	M_SERIALIZER_END;
}
