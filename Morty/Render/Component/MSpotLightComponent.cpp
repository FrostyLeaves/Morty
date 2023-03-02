#include "Component/MSpotLightComponent.h"

MORTY_CLASS_IMPLEMENT(MSpotLightComponent, MComponent)

#include "Scene/MEntity.h"
#include "Component/MSceneComponent.h"

#include "Flatbuffer/MSpotLightComponent_generated.h"

MSpotLightComponent::MSpotLightComponent()
	: MComponent()
	, m_f3Color(1.0f, 1.0f, 1.0f)
	, m_fIntensity(1.0f)
	, m_fInnerCutOffAngle(90.0f)
	, m_fOuterCutOffAngle(90.0f)
{
	m_fInnerCutOffRadius = cosf(m_fInnerCutOffAngle * 0.5f * M_PI / 180.0f);
	m_fOuterCutOffRadius = cosf(m_fOuterCutOffAngle * 0.5f * M_PI / 180.0f);
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

flatbuffers::Offset<void> MSpotLightComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
	auto fbSuper = Super::Serialize(fbb).o;

	mfbs::MSpotLightComponentBuilder builder(fbb);

	builder.add_color(reinterpret_cast<mfbs::Vector4*>(&GetColorVector()));
	builder.add_light_intensity(GetLightIntensity());
	builder.add_inner_cut_off_angle(GetInnerCutOff());
	builder.add_outer_cut_off_angle(GetOuterCutOff());

	builder.add_super(fbSuper);

	return builder.Finish().Union();
}

void MSpotLightComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
	const mfbs::MSpotLightComponent* fbcomponent = mfbs::GetMSpotLightComponent(fbb.GetCurrentBufferPointer());
	Deserialize(fbcomponent);
}

void MSpotLightComponent::Deserialize(const void* pBufferPointer)
{
	const mfbs::MSpotLightComponent* pComponent = reinterpret_cast<const mfbs::MSpotLightComponent*>(pBufferPointer);

	SetColorVector(*reinterpret_cast<const Vector4*>(pComponent->color()));
	SetLightIntensity(pComponent->light_intensity());
	SetInnerCutOff(pComponent->inner_cut_off_angle());
	SetOuterCutOff(pComponent->outer_cut_off_angle());

	Super::Deserialize(pComponent->super());
}
