#include "Component/MDirectionalLightComponent.h"

#include "Flatbuffer/MDirectionalLightComponent_generated.h"

#include "Scene/MScene.h"
#include "Component/MSceneComponent.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MDirectionalLightComponent, MComponent)

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

flatbuffers::Offset<void> MDirectionalLightComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
	auto fbSuper = Super::Serialize(fbb).o;

	fbs::MDirectionalLightComponentBuilder builder(fbb);

	Vector4 color = GetColorVector();
	builder.add_color(reinterpret_cast<fbs::Vector4*>(&color));
	builder.add_light_intensity(GetLightIntensity());

	builder.add_super(fbSuper);

	return builder.Finish().Union();
}

void MDirectionalLightComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
	const fbs::MDirectionalLightComponent* fbcomponent = fbs::GetMDirectionalLightComponent(fbb.GetCurrentBufferPointer());
	Deserialize(fbcomponent);
}

void MDirectionalLightComponent::Deserialize(const void* pBufferPointer)
{
	const fbs::MDirectionalLightComponent* pComponent = reinterpret_cast<const fbs::MDirectionalLightComponent*>(pBufferPointer);

	SetColorVector(*reinterpret_cast<const Vector4*>(pComponent->color()));
	SetLightIntensity(pComponent->light_intensity());

	Super::Deserialize(pComponent->super());
}
