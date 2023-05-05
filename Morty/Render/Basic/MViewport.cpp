#include "Basic/MViewport.h"

#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MRenderSystem.h"

#include "Component/MInputComponent.h"
#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include <float.h>

MORTY_CLASS_IMPLEMENT(MViewport, MObject)

#define Vector3Intersection(a, b, p) {\
	if ((a).x > (p).x)			\
		(a).x = (p).x;			\
	if ((a).y > (p).y)			\
		(a).y = (p).y;			\
	if ((a).z > (p).z)			\
		(a).z = (p).z;			\
	if ((b).x < (p).x)			\
		(b).x = (p).x;			\
	if ((b).y < (p).y)			\
		(b).y = (p).y;			\
	if ((b).z < (p).z)			\
		(b).z = (p).z;			\
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

MViewport::MViewport()
	: MObject()
	, m_pScene(nullptr)
	, m_pUserCamera(nullptr)
	, m_v2LeftTop(0,0)
	, m_v2Size(0, 0)
	, m_v2ScreenPosition(0, 0)
	, m_v2ScreenScale(1, 1)
{

}

MViewport::~MViewport()
{
}

bool MViewport::ConvertWorldPointToViewport(const Vector3& v3WorldPos, Vector3& v2Result)
{
	MCameraComponent* pCameraComponent = GetCamera()->GetComponent<MCameraComponent>();
	if (!pCameraComponent)
	{
		return false;
	}

	MSceneComponent* pSceneComponent = GetCamera()->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
	{
		return false;
	}

	Matrix4 m4CameraInvProj = MRenderSystem::GetCameraInverseProjection(this, pCameraComponent, pSceneComponent);


	Vector4 pos = m4CameraInvProj * Vector4(v3WorldPos, 1.0f);
	float z = pos.z;
	if (fabs(pos.w) > 1e-6)
	{
		pos /= pos.w;
	}

	v2Result = Vector3((pos.x + 1.0) * 0.5 * GetWidth(), (pos.y + 1.0) * 0.5 * GetHeight(), pos.z);

	return z >= pCameraComponent->GetZNear();
}	

void MViewport::ConvertViewportPointToWorld(const Vector2& v2ViewportPos, const float& fDepth, Vector3& v3Result)
{
	MCameraComponent* pCameraComponent = GetCamera()->GetComponent<MCameraComponent>();
	if (!pCameraComponent)
	{
		return;
	}

	MSceneComponent* pSceneComponent = GetCamera()->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
	{
		return;
	}

	Matrix4 m4CameraInvProj = MRenderSystem::GetCameraInverseProjection(this, pCameraComponent, pSceneComponent);

	Matrix4 mat = m4CameraInvProj.Inverse();

	float x = (v2ViewportPos.x / GetWidth()) * 2.0f - 1.0f;
	float y = (v2ViewportPos.y / GetHeight()) * 2.0f - 1.0f;

	Vector4 pos4 = mat * Vector4(x, y, pCameraComponent->GetZNear(), 1.0f);
	Vector3 pos = pos4 / pos4.w;
	Vector3 dir = pos - pSceneComponent->GetWorldPosition();
	dir.Normalize();

	v3Result = pos + dir * fDepth;
}

bool  MViewport::ConvertWorldLineToNormalizedDevice(const Vector3& v3Pos1, const Vector3& v3Pos2, Vector2& v3Rst1, Vector2& v3Rst2)
{
	MCameraComponent* pCameraComponent = GetCamera()->GetComponent<MCameraComponent>();
	if (!pCameraComponent)
	{
		return false;
	}

	MSceneComponent* pSceneComponent = GetCamera()->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
	{
		return false;
	}

	Matrix4 m4CameraInvProj = MRenderSystem::GetCameraInverseProjection(this, pCameraComponent, pSceneComponent);


	Vector4 v4Pos1 = m4CameraInvProj * Vector4(v3Pos1, 1.0f);
	Vector4 v4Pos2 = m4CameraInvProj * Vector4(v3Pos2, 1.0f);

	if (fabs(v4Pos1.w) > 1e-6)
	{
		v4Pos1.x /= v4Pos1.w;
		v4Pos1.y /= v4Pos1.w;
	}

	if (fabs(v4Pos2.w) > 1e-6)
	{
		v4Pos2.x /= v4Pos2.w;
		v4Pos2.y /= v4Pos2.w;
	}

	float znear = pCameraComponent->GetZNear();

	v3Rst1 = v4Pos1;
	v3Rst2 = v4Pos2;

	if (v4Pos1.z < znear || v4Pos2.z < znear)
		return false;

	return true;
}

bool MViewport::ConvertWorldPointToNormalizedDevice(const Vector3& v3Pos, Vector2& v2Rst)
{
	MCameraComponent* pCameraComponent = GetCamera()->GetComponent<MCameraComponent>();
	if (!pCameraComponent)
	{
		return false;
	}

	MSceneComponent* pSceneComponent = GetCamera()->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
	{
		return false;
	}

	Matrix4 m4CameraInvProj = MRenderSystem::GetCameraInverseProjection(this, pCameraComponent, pSceneComponent);

	Vector4 v4Rst = m4CameraInvProj * Vector4(v3Pos, 1.0f);
	if (fabs(v4Rst.w) > 1e-6)
	{
		v4Rst.x /= v4Rst.w;
		v4Rst.y /= v4Rst.w;
	}

	v2Rst = v4Rst;
	if (v4Rst.z < pCameraComponent->GetZNear())
		return false;

	return true;
}

bool MViewport::ConvertScreenPointToViewport(const Vector2& v2Point, Vector2& v2Result)
{
	v2Result.x = (v2Point.x - m_v2ScreenPosition.x) * m_v2ScreenScale.x;
	v2Result.y = m_v2Size.y - (v2Point.y - m_v2ScreenPosition.y)* m_v2ScreenScale.y;

	return v2Result.x >= 0.0f && v2Result.y >= 0.0f && v2Result.x <= m_v2Size.x && v2Result.y <= m_v2Size.y;
}

void MViewport::OnCreated()
{
	MObject::OnCreated();
}

void MViewport::OnDelete()
{
	if (m_pScene)
	{
		m_pScene = nullptr;
	}

	Super::OnDelete();
}

void MViewport::SetSize(const Vector2& v2Size)
{
	m_v2Size = v2Size;
}

void MViewport::Input(MInputEvent* pEvent)
{
	if (!m_pScene)
		return;

	MComponentGroup<MInputComponent>* pComponents = m_pScene->FindComponents<MInputComponent>();
	if (!pComponents)
		return;

	for (MInputComponent& inputComponent : pComponents->m_vComponents)
	{
		if (inputComponent.IsValid())
		{
			inputComponent.Input(pEvent, this);
		}
	}
}

void MViewport::SetScene(MScene* pScene)
{
	if (m_pScene == pScene)
		return;

	m_pScene = pScene;
}

void MViewport::SetCamera(MEntity* pCamera)
{
	if (m_pScene && pCamera && pCamera->GetScene() == m_pScene)
	{
		SetValidCamera(pCamera);
	}
	else
	{
		m_pUserCamera = nullptr;
	}
}

void MViewport::SetValidCamera(MEntity* pCamera)
{
	m_pUserCamera = pCamera;
}

MEntity* MViewport::GetCamera() const
{
	return m_pUserCamera;
}
