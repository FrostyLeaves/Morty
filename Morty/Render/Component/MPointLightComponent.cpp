#include "Component/MPointLightComponent.h"

#include "Flatbuffer/MPointLightComponent_generated.h"

MORTY_CLASS_IMPLEMENT(MPointLightComponent, MComponent)

MPointLightComponent::MPointLightComponent()
	: MComponent()
	, m_f3Color(1.0f, 1.0f, 1.0f)
	, m_fIntensity(1.0f)
	, m_fConstant(1.0f)
	, m_fLinear(0.022f)
	, m_fQuadratic(0.0019f)
{

}

MPointLightComponent::~MPointLightComponent()
{

}

flatbuffers::Offset<void> MPointLightComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
	auto super = Super::Serialize(fbb).o;

	mfbs::MPointLightComponentBuilder builder(fbb);

	builder.add_color(reinterpret_cast<mfbs::Vector4*>(&GetColorVector()));
	builder.add_light_intensity(GetLightIntensity());
	builder.add_constant(GetConstant());
	builder.add_linear(GetLinear());
	builder.add_quadratic(GetQuadratic());

	builder.add_super(super);

	return builder.Finish().Union();
}

void MPointLightComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
	const mfbs::MPointLightComponent* fbcomponent = mfbs::GetMPointLightComponent(fbb.GetCurrentBufferPointer());
	Deserialize(fbcomponent);
}

void MPointLightComponent::Deserialize(const void* pBufferPointer)
{
	const mfbs::MPointLightComponent* pComponent = reinterpret_cast<const mfbs::MPointLightComponent*>(pBufferPointer);

	SetColorVector(*reinterpret_cast<const Vector4*>(pComponent->color()));
	SetLightIntensity(pComponent->light_intensity());
	SetConstant(pComponent->constant());
	SetLinear(pComponent->linear());
	SetQuadratic(pComponent->quadratic());

	Super::Deserialize(pComponent->super());
}
