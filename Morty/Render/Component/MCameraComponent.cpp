#include "MCameraComponent.h"

#include "MEngine.h"

MORTY_CLASS_IMPLEMENT(MCameraComponent, MComponent)

MCameraComponent::MCameraComponent()
	: MComponent()
	, m_eCameraType(MECameraType::EPerspective)
	, m_fFov(40.0f)
	, m_fZNear(0.1f)
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

void MCameraComponent::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);

	M_SERIALIZER_WRITE_BEGIN;

	M_SERIALIZER_WRITE_VALUE("CameraType", (int)GetCameraType);
	M_SERIALIZER_WRITE_VALUE("Fov", GetFov);
	M_SERIALIZER_WRITE_VALUE("ZNear", GetZNear);
	M_SERIALIZER_WRITE_VALUE("ZFar", GetZFar);
	M_SERIALIZER_WRITE_VALUE("Width", GetWidth);
	M_SERIALIZER_WRITE_VALUE("Height", GetHeight);

	M_SERIALIZER_END;
}

void MCameraComponent::ReadFromStruct(const MStruct& srt)
{
	Super::ReadFromStruct(srt);

	M_SERIALIZER_READ_BEGIN;

	M_SERIALIZER_READ_VALUE("CameraType", SetCameraType, Enum<MECameraType>);
	M_SERIALIZER_READ_VALUE("Fov", SetFov, Float);
	M_SERIALIZER_READ_VALUE("ZNear", SetZNear, Float);
	M_SERIALIZER_READ_VALUE("ZFar", SetZFar, Float);
	M_SERIALIZER_READ_VALUE("Width", SetWidth, Float);
	M_SERIALIZER_READ_VALUE("Height", SetHeight, Float);

	M_SERIALIZER_END;
}
