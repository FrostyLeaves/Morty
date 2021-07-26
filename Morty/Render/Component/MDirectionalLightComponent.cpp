#include "MDirectionalLightComponent.h"

MORTY_CLASS_IMPLEMENT(MDirectionalLightComponent, MComponent)

#include "MScene.h"
#include "MSceneComponent.h"

MDirectionalLightComponent::MDirectionalLightComponent()
	: MComponent()
	, m_v3Direction(0.0f, 0.0f, 1.0f)
	, m_f3Color(1.0f, 1.0f, 1.0f)
	, m_fIntensity(1.0f)
{

}

MDirectionalLightComponent::~MDirectionalLightComponent()
{

}

void MDirectionalLightComponent::SetDirection(const Vector3& v3Direction)
{
	m_v3Direction = v3Direction;
}

Vector3 MDirectionalLightComponent::GetWorldDirection()
{
	MSceneComponent* pSceneComponent = GetEntity()->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
		return Vector3(0.0f, 0.0f, 1.0f);

	return pSceneComponent->GetWorldTransform().GetRotatePart() * m_v3Direction;
}

void MDirectionalLightComponent::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);

	M_SERIALIZER_WRITE_BEGIN;

	M_SERIALIZER_WRITE_VALUE("Color", GetColorVector);
	M_SERIALIZER_WRITE_VALUE("LightIntensity", GetLightIntensity);

	M_SERIALIZER_END;
}

void MDirectionalLightComponent::ReadFromStruct(const MStruct& srt)
{
	Super::ReadFromStruct(srt);

	M_SERIALIZER_READ_BEGIN;

	M_SERIALIZER_READ_VALUE("Color", SetColorVector, Vector4);
	M_SERIALIZER_READ_VALUE("LightIntensity", SetLightIntensity, Float);

	M_SERIALIZER_END;
}
