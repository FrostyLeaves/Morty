#include "MCamera.h"
#include "MLogManager.h"

MTypeIdentifierImplement(MCamera, M3DNode)

MCamera::MCamera()
	: M3DNode()
	, m_fFov(40.0f)
	, m_fZNear(0.1f)
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
	{
		MLogManager::GetInstance()->Error("MCamera::SetZNear:   fZNear >= m_fZFar");
	}
	else
	{
		m_fZNear = fZNear;
	}
}

void MCamera::SetZFar(const float& fZFar)
{
	if (fZFar <= m_fZNear)
	{
		MLogManager::GetInstance()->Error("MCamera::SetZFar:   fZFar <= m_fZNear");
	}
	else
	{
		m_fZFar = fZFar;
	}
}
