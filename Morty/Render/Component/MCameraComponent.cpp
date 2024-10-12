#include "Component/MCameraComponent.h"

#include "Engine/MEngine.h"

#include "Flatbuffer/MCameraComponent_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MCameraComponent, MComponent)

MCameraComponent::MCameraComponent()
    : MComponent()
    , m_cameraType(MECameraType::EPerspective)
    , m_fov(60.0f)
    , m_zNear(1.0f)
    , m_zFar(500.0f)
    , m_width(50)
    , m_height(50)
{}

MCameraComponent::~MCameraComponent() {}

void MCameraComponent::SetFov(const float& fFov) { m_fov = fFov; }

void MCameraComponent::SetZNear(const float& fZNear)
{
    if (fZNear >= m_zFar) { GetEngine()->GetLogger()->Warning("MCamera::SetZNear:   fZNear >= m_zFar"); }
    // 	else
    m_zNear = fZNear;
}

void MCameraComponent::SetZFar(const float& fZFar)
{
    if (fZFar <= m_zNear) GetEngine()->GetLogger()->Warning("MCamera::SetZFar:   fZFar <= m_zNear");
    //	else
    m_zFar = fZFar;
}

flatbuffers::Offset<void> MCameraComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    auto                         fbSuper = Super::Serialize(fbb).o;

    fbs::MCameraComponentBuilder builder(fbb);

    builder.add_camera_type((int) GetCameraType());
    builder.add_fov(GetFov());
    builder.add_znear(GetZNear());
    builder.add_zfar(GetZFar());
    builder.add_width(GetWidth());
    builder.add_height(GetHeight());

    builder.add_super(fbSuper);

    return builder.Finish().Union();
}

void MCameraComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
    const fbs::MCameraComponent* fbcomponent = fbs::GetMCameraComponent(fbb.GetCurrentBufferPointer());
    Deserialize(fbcomponent);
}

void MCameraComponent::Deserialize(const void* pBufferPointer)
{
    const fbs::MCameraComponent* pComponent = reinterpret_cast<const fbs::MCameraComponent*>(pBufferPointer);

    Super::Deserialize(pComponent->super());

    SetCameraType((MECameraType) pComponent->camera_type());
    SetFov(pComponent->fov());
    SetZNear(pComponent->znear());
    SetZFar(pComponent->zfar());
    SetWidth(pComponent->width());
    SetHeight(pComponent->height());
}
