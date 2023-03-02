#include "Component/MCameraComponent.h"

#include "Engine/MEngine.h"

#include "Flatbuffer/MCameraComponent_generated.h"

MORTY_CLASS_IMPLEMENT(MCameraComponent, MComponent)

MCameraComponent::MCameraComponent()
	: MComponent()
	, m_eCameraType(MECameraType::EPerspective)
	, m_fFov(40.0f)
	, m_fZNear(1.0f)
	, m_fZFar(500.0f)
	, m_fWidth(50)
	, m_fHeight(50)
{

}

MCameraComponent::~MCameraComponent()
{

}

void MCameraComponent::SetFov(const float& fFov)
{
	m_fFov = fFov;
}

void MCameraComponent::SetZNear(const float& fZNear)
{
	if (fZNear >= m_fZFar)
	{
		GetEngine()->GetLogger()->Warning("MCamera::SetZNear:   fZNear >= m_fZFar");
	}
	// 	else
	m_fZNear = fZNear;
}

void MCameraComponent::SetZFar(const float& fZFar)
{
	if (fZFar <= m_fZNear)
		GetEngine()->GetLogger()->Warning("MCamera::SetZFar:   fZFar <= m_fZNear");
	//	else
	m_fZFar = fZFar;
}

flatbuffers::Offset<void> MCameraComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
	auto fbSuper = Super::Serialize(fbb).o;

	mfbs::MCameraComponentBuilder builder(fbb);

	builder.add_camera_type((int)GetCameraType());
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
	const mfbs::MCameraComponent* fbcomponent = mfbs::GetMCameraComponent(fbb.GetCurrentBufferPointer());
	Deserialize(fbcomponent);
}

void MCameraComponent::Deserialize(const void* pBufferPointer)
{
	const mfbs::MCameraComponent* pComponent = reinterpret_cast<const mfbs::MCameraComponent*>(pBufferPointer);

	Super::Deserialize(pComponent->super());

	SetCameraType((MECameraType)pComponent->camera_type());
	SetFov(pComponent->fov());
	SetZNear(pComponent->znear());
	SetZFar(pComponent->zfar());
	SetWidth(pComponent->width());
	SetHeight(pComponent->height());
}
