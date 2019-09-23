#include "MCamera.h"
#include "MLogManager.h"

MCamera::MCamera()
	: m_fZNear(10.0f)
	, m_fZFar(500.0f)
{

}

MCamera::~MCamera()
{

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
