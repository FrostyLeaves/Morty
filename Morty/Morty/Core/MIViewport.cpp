#include "MIViewport.h"
#include "MIScene.h"
#include "MCamera.h"
#include "MEngine.h"

MIViewport::MIViewport()
	: m_pScene(nullptr)
	, m_pUserCamera(nullptr)
	, m_pDefaultCamera(nullptr)
	, m_m4CameraInvProj(Matrix4::IdentityMatrix)
{

}

void MIViewport::OnCreated()
{
	MObject::OnCreated();

	//Init Default Camera.
	m_pDefaultCamera = m_pEngine->GetObjectManager()->CreateObject<MCamera>();
}

void MIViewport::Render(MIRenderer* pRenderer)
{
	if (nullptr == m_pScene)
		return;

	//Update Camera and Projection Matrix.
	MCamera* pCamera = GetCamera();
	Matrix4 projMat = MatrixPerspectiveFovLH(45, m_v2Size.x / m_v2Size.y, pCamera->GetZNear(), pCamera->GetZFar());
	m_m4CameraInvProj = pCamera->GetWorldTransform().Inverse() * projMat;

	m_pScene->Render(pRenderer, this);
}

MIViewport::~MIViewport()
{

}

void MIViewport::SetScene(MIScene* pScene)
{
	m_pScene = pScene;
	if (m_pScene)
	{
		m_pScene->SetAttachedViewport(this);
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

MCamera* MIViewport::GetCamera()
{
	return m_pUserCamera ? m_pUserCamera : m_pDefaultCamera;
}

void MIViewport::SetSize(const Vector2& v2Size)
{
	m_v2Size = v2Size;
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
	mProjMatrix.m[3][2] = b;
	mProjMatrix.m[2][3] = 1.0f;

	return mProjMatrix;
}
