#include "MCamera.h"
#include "MLogManager.h"

M_OBJECT_IMPLEMENT(MCamera, M3DNode)

MCamera::MCamera()
	: M3DNode()
	, m_fFov(40.0f)
	, m_fZNear(0.001f)
	, m_fZFar(500.0f)
	, m_fWidth(50)
	, m_fHeight(50)
	, m_eCameraType(MECameraType::EPerspective)
{

}

MCamera::~MCamera()
{

}

void MCamera::SetFov(const float& fFov)
{
	m_fFov = fFov;
}

void MCamera::SetZNear(const float& fZNear)
{
	if (fZNear >= m_fZFar)
		MLogManager::GetInstance()->Warning("MCamera::SetZNear:   fZNear >= m_fZFar");
// 	else
		m_fZNear = fZNear;
}

void MCamera::SetZFar(const float& fZFar)
{
	if (fZFar <= m_fZNear)
		MLogManager::GetInstance()->Warning("MCamera::SetZFar:   fZFar <= m_fZNear");
//	else
		m_fZFar = fZFar;
}

void MCamera::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);

	M_SERIALIZER_BEGIN(Write);

	M_SERIALIZER_WRITE_VALUE("CameraType", (int)GetCameraType);
	M_SERIALIZER_WRITE_VALUE("Fov", GetFov);
	M_SERIALIZER_WRITE_VALUE("ZNear", GetZNear);
	M_SERIALIZER_WRITE_VALUE("ZFar", GetZFar);
	M_SERIALIZER_WRITE_VALUE("Width", GetWidth);
	M_SERIALIZER_WRITE_VALUE("Height", GetHeight);

	M_SERIALIZER_END;
}

void MCamera::ReadFromStruct(MStruct& srt)
{
	Super::ReadFromStruct(srt);

	M_SERIALIZER_BEGIN(Read);

	M_SERIALIZER_READ_VALUE("CameraType", SetCameraType, Enum<MECameraType>);
	M_SERIALIZER_READ_VALUE("Fov", SetFov, Float);
	M_SERIALIZER_READ_VALUE("ZNear", SetZNear, Float);
	M_SERIALIZER_READ_VALUE("ZFar", SetZFar, Float);
	M_SERIALIZER_READ_VALUE("Width", SetWidth, Float);
	M_SERIALIZER_READ_VALUE("Height", SetHeight, Float);

	M_SERIALIZER_END;
}

