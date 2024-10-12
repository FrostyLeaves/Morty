#include "Component/MSpotLightComponent.h"

#include "Component/MSceneComponent.h"
#include "Scene/MEntity.h"

#include "Flatbuffer/MSpotLightComponent_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSpotLightComponent, MComponent)

MSpotLightComponent::MSpotLightComponent()
    : MComponent()
    , m_color(1.0f, 1.0f, 1.0f)
    , m_intensity(1.0f)
    , m_innerCutOffAngle(90.0f)
    , m_outerCutOffAngle(90.0f)
{
    m_innerCutOffRadius = cosf(m_innerCutOffAngle * 0.5f * M_PI / 180.0f);
    m_outerCutOffRadius = cosf(m_outerCutOffAngle * 0.5f * M_PI / 180.0f);
}

MSpotLightComponent::~MSpotLightComponent() {}

void MSpotLightComponent::SetInnerCutOff(const float& fCutOff)
{
    m_innerCutOffAngle  = fCutOff;
    m_innerCutOffRadius = cos(m_innerCutOffAngle * 0.5f * M_PI / 180.0f);

    if (fCutOff - m_outerCutOffAngle > 1e-6f) SetOuterCutOff(fCutOff);
}

void MSpotLightComponent::SetOuterCutOff(const float& fCutOff)
{
    m_outerCutOffAngle  = fCutOff;
    m_outerCutOffRadius = cos(m_outerCutOffAngle * 0.5f * M_PI / 180.0f);

    if (fCutOff - m_innerCutOffAngle < -1e-6f) SetInnerCutOff(fCutOff);
}

Vector3 MSpotLightComponent::GetWorldDirection()
{
    MEntity* pEntity = GetEntity();
    if (!pEntity) return Vector3(0.0f, 0.0f, 1.0f);

    MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
    if (!pSceneComponent) return Vector3(0.0f, 0.0f, 1.0f);

    return pSceneComponent->GetWorldTransform().GetRotatePart() * Vector3(0.0f, 0.0f, 1.0f);
}

flatbuffers::Offset<void> MSpotLightComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    auto                            fbSuper = Super::Serialize(fbb).o;

    fbs::MSpotLightComponentBuilder builder(fbb);

    Vector4                         color = GetColorVector();
    builder.add_color(reinterpret_cast<fbs::Vector4*>(&color));
    builder.add_light_intensity(GetLightIntensity());
    builder.add_inner_cut_off_angle(GetInnerCutOff());
    builder.add_outer_cut_off_angle(GetOuterCutOff());

    builder.add_super(fbSuper);

    return builder.Finish().Union();
}

void MSpotLightComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
    const fbs::MSpotLightComponent* fbcomponent = fbs::GetMSpotLightComponent(fbb.GetCurrentBufferPointer());
    Deserialize(fbcomponent);
}

void MSpotLightComponent::Deserialize(const void* pBufferPointer)
{
    const fbs::MSpotLightComponent* pComponent = reinterpret_cast<const fbs::MSpotLightComponent*>(pBufferPointer);

    SetColorVector(*reinterpret_cast<const Vector4*>(pComponent->color()));
    SetLightIntensity(pComponent->light_intensity());
    SetInnerCutOff(pComponent->inner_cut_off_angle());
    SetOuterCutOff(pComponent->outer_cut_off_angle());

    Super::Deserialize(pComponent->super());
}
