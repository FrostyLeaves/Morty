#include "Component/MPointLightComponent.h"

#include "Flatbuffer/MPointLightComponent_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MPointLightComponent, MComponent)

MPointLightComponent::MPointLightComponent()
    : MComponent()
    , m_color(1.0f, 1.0f, 1.0f)
    , m_intensity(1.0f)
    , m_constant(1.0f)
    , m_linear(0.022f)
    , m_quadratic(0.0019f)
{}

MPointLightComponent::~MPointLightComponent() {}

flatbuffers::Offset<void> MPointLightComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    auto                             super = Super::Serialize(fbb).o;

    fbs::MPointLightComponentBuilder builder(fbb);

    Vector4                          color = GetColorVector();
    builder.add_color(reinterpret_cast<fbs::Vector4*>(&color));
    builder.add_light_intensity(GetLightIntensity());
    builder.add_constant(GetConstant());
    builder.add_linear(GetLinear());
    builder.add_quadratic(GetQuadratic());

    builder.add_super(super);

    return builder.Finish().Union();
}

void MPointLightComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
    const fbs::MPointLightComponent* fbcomponent = fbs::GetMPointLightComponent(fbb.GetCurrentBufferPointer());
    Deserialize(fbcomponent);
}

void MPointLightComponent::Deserialize(const void* pBufferPointer)
{
    const fbs::MPointLightComponent* pComponent = reinterpret_cast<const fbs::MPointLightComponent*>(pBufferPointer);

    SetColorVector(*reinterpret_cast<const Vector4*>(pComponent->color()));
    SetLightIntensity(pComponent->light_intensity());
    SetConstant(pComponent->constant());
    SetLinear(pComponent->linear());
    SetQuadratic(pComponent->quadratic());

    Super::Deserialize(pComponent->super());
}
