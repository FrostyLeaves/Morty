#include "MIViewport.h"
#include "MIScene.h"
#include "MCamera.h"
#include "MEngine.h"

MIViewport::MIViewport()
	: m_pScene(nullptr)
	, m_pUserCamera(nullptr)
	, m_pDefaultCamera(nullptr)
	, m_m4CameraInvProj(Matrix4::IdentityMatrix)
	, m_v2LeftTop(0,0)
	, m_v2Size(0, 0)
{

}

MIViewport::~MIViewport()
{
// 	if (m_pDefaultCamera)
// 	{
// 		m_pEngine->GetObjectManager()->RemoveObject(m_pDefaultCamera->GetObjectID());
// 		m_pDefaultCamera = nullptr;
// 	}
// 
// 	if (m_pScene)
// 	{
// 		m_pScene->RemoveAttachedViewport(this);
// 		m_pScene = nullptr;
// 	}
}

bool MIViewport::ConvertWorldPositionToViewport(const Vector3& v3WorldPos, Vector2& v2Result)
{
	UpdateMatrix();

	Vector4 pos = m_m4CameraInvProj * Vector4(v3WorldPos, 1.0f);

	if (fabs(pos.w) > 1e-6)
	{
		pos.x /= pos.w;
		pos.y /= pos.w;
	}

	v2Result = Vector2((pos.x + 1.0) * 0.5 * GetWidth(), (pos.y + 1.0) * 0.5 * GetHeight());

	return pos.z >= GetCamera()->GetZNear();
}	

void MIViewport::ConvertViewportPositionToWorld(const Vector2& v2ViewportPos, const float& fDepth, Vector3& v3Result)
{
	UpdateMatrix();

	Matrix4 mat = m_m4CameraInvProj.Inverse();

	float x = (v2ViewportPos.x / GetWidth()) * 2.0f - 1.0f;
	float y = (v2ViewportPos.y / GetHeight()) * 2.0f - 1.0f;

	Vector3 pos = mat * Vector4(x, y, GetCamera()->GetZNear(), 1);
	Vector3 dir = pos - GetCamera()->GetWorldPosition();
	dir.Normalize();

	v3Result = pos + dir * fDepth;
}

bool  MIViewport::ConvertWorldLineToNormalizedDevice(const Vector3& v3Pos1, const Vector3& v3Pos2, Vector3& v3Rst1, Vector3& v3Rst2)
{
	UpdateMatrix();

	Vector4 v4Pos1 = m_m4CameraInvProj * Vector4(v3Pos1, 1.0f);
	Vector4 v4Pos2 = m_m4CameraInvProj * Vector4(v3Pos2, 1.0f);

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

	float znear = GetCamera()->GetZNear();

	if (v4Pos1.z < znear && v4Pos2.z < znear)
	{
		return false;
	}
	else if (v4Pos1.z >= znear&& v4Pos2.z >= znear)
	{
		v3Rst1 = v4Pos1;
		v3Rst2 = v4Pos2;
	}
	else if (v4Pos1.z < znear && v4Pos2.z >= znear)
	{
		Vector3 dir = (v4Pos1 - v4Pos2);
		dir.Normalize();
		v3Rst2 = v4Pos2;
		v3Rst1 = v3Rst2 + dir * (znear - v4Pos2.z) / dir.z;
	}
	else
	{
		Vector3 dir = (v4Pos2 - v4Pos1);
		dir.Normalize();
		v3Rst1 = v4Pos1;
		v3Rst2 = v3Rst1 + dir * (znear - v4Pos1.z) / dir.z;
	}

	return true;
}

void MIViewport::OnCreated()
{
	MObject::OnCreated();

	//Init Default Camera.
	m_pDefaultCamera = m_pEngine->GetObjectManager()->CreateObject<MCamera>();
}

void MIViewport::SetSize(const Vector2& v2Size)
{
	m_v2Size = v2Size;
}

void MIViewport::Render(MIRenderer* pRenderer)
{
	if (nullptr == m_pScene)
		return;

	UpdateMatrix();
	m_pScene->Render(pRenderer, this);
}

void MIViewport::Input(MInputEvent* pEvent)
{
	if(m_pScene)
	{
	}
}

void MIViewport::SetScene(MIScene* pScene)
{
	if (m_pScene == pScene)
		return;

	if (m_pScene)
		m_pScene->RemoveAttachedViewport(this);

	m_pScene = pScene;

	if (pScene)
	{		
		m_pScene->AddAttachedViewport(this);
		if (MNode* pRootNode = m_pScene->GetRootNode())
		{
			MCamera* pCamera = pRootNode->FindFirstChildByType<MCamera>();
			SetValidCamera(pCamera);
		}
	}
}

void MIViewport::SetCamera(MCamera* pCamera)
{
	if (m_pScene && pCamera->GetScene() == m_pScene)
	{
		SetValidCamera(pCamera);
	}
	else
	{
		m_pUserCamera = nullptr;
	}
}

void MIViewport::SetValidCamera(MCamera* pCamera)
{
	m_pUserCamera = pCamera;
}

void MIViewport::UpdateMatrix()
{
	//Update Camera and Projection Matrix.
	MCamera* pCamera = GetCamera();
	Matrix4 projMat = MatrixPerspectiveFovLH(20, m_v2Size.x / m_v2Size.y, pCamera->GetZNear(), pCamera->GetZFar());
	m_m4CameraInvProj = projMat * pCamera->GetWorldTransform().Inverse();
}

MCamera* MIViewport::GetCamera()
{
	return m_pUserCamera ? m_pUserCamera : m_pDefaultCamera;
}

Matrix4 MIViewport::MatrixPerspectiveFovLH(const float& fFovYZAngle, const float& fScreenAspect, const float& fScreenNear, const float& fScreenFar)
{

	Matrix4 mProjMatrix;

	//清除为0
	ZeroMemory(&mProjMatrix.m, sizeof(mProjMatrix.m));

	//度数转化为弧度
	float angle = fFovYZAngle / 180.0f * M_PI;

	//半角
	angle /= 2.0f;

	//求出各类参数
	float s1 = 1 / (fScreenAspect*(float)tan(angle));
	float s2 = 1 / tan(angle);
	float a = fScreenFar / (fScreenFar - fScreenNear);
	float b = -(fScreenNear*fScreenFar) / (fScreenFar - fScreenNear);

	mProjMatrix.m[0][0] = s1;
	mProjMatrix.m[1][1] = s2;
	mProjMatrix.m[2][2] = a;
	mProjMatrix.m[2][3] = b;
	mProjMatrix.m[3][2] = 1.0f;

	return mProjMatrix;
}
