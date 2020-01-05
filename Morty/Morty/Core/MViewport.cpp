#include "MViewport.h"
#include "MScene.h"
#include "MCamera.h"
#include "MEngine.h"
#include "MIRenderer.h"
#include "MInputManager.h"

#include "MBounds.h"
#include "MDirectionalLight.h"

MTypeIdentifierImplement(MViewport, MObject)

MViewport::MViewport()
	: MObject()
	, m_pScene(nullptr)
	, m_pUserCamera(nullptr)
	, m_pDefaultCamera(nullptr)
	, m_m4CameraInvProj(Matrix4::IdentityMatrix)
	, m_bCameraInvProjMatrixUpdated(false)
	, m_v2LeftTop(0,0)
	, m_v2Size(0, 0)
{

}

MViewport::~MViewport()
{
	if (m_pDefaultCamera)
	{
		m_pEngine->GetObjectManager()->RemoveObject(m_pDefaultCamera->GetObjectID());
		m_pDefaultCamera = nullptr;
	}

// 	if (m_pScene)
// 	{
// 		m_pScene->RemoveAttachedViewport(this);
// 		m_pScene = nullptr;
// 	}
}

bool MViewport::ConvertWorldPointToViewport(const Vector3& v3WorldPos, Vector3& v2Result)
{
	UpdateMatrix();

	Vector4 pos = m_m4CameraInvProj * Vector4(v3WorldPos, 1.0f);
	float z = pos.z;
	if (fabs(pos.w) > 1e-6)
	{
		pos /= pos.w;
	}

	v2Result = Vector3((pos.x + 1.0) * 0.5 * GetWidth(), (pos.y + 1.0) * 0.5 * GetHeight(), pos.z);

	return z >= GetCamera()->GetZNear();
}	

void MViewport::ConvertViewportPointToWorld(const Vector2& v2ViewportPos, const float& fDepth, Vector3& v3Result)
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

bool  MViewport::ConvertWorldLineToNormalizedDevice(const Vector3& v3Pos1, const Vector3& v3Pos2, Vector2& v3Rst1, Vector2& v3Rst2)
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

	v3Rst1 = v4Pos1;
	v3Rst2 = v4Pos2;

	if (v4Pos1.z < znear || v4Pos2.z < znear)
		return false;

	return true;
}

bool MViewport::ConvertWorldPointToNormalizedDevice(const Vector3& v3Pos, Vector2& v2Rst)
{
	UpdateMatrix();

	Vector4 v4Rst = m_m4CameraInvProj * Vector4(v3Pos, 1.0f);
	if (fabs(v4Rst.w) > 1e-6)
	{
		v4Rst.x /= v4Rst.w;
		v4Rst.y /= v4Rst.w;
	}

	v2Rst = v4Rst;
	if (v4Rst.z < GetCamera()->GetZNear())
		return false;

	return true;
}

void MViewport::OnCreated()
{
	MObject::OnCreated();

	//Init Default Camera.
	m_pDefaultCamera = m_pEngine->GetObjectManager()->CreateObject<MCamera>();
}

void MViewport::SetSize(const Vector2& v2Size)
{
	m_v2Size = v2Size;
}

void MViewport::Render(MIRenderer* pRenderer)
{
	if (nullptr == m_pScene)
		return;


	UpdateMatrix();
	m_bCameraInvProjMatrixUpdated = true;
	
	m_pScene->Render(pRenderer, this);
	
	
	m_bCameraInvProjMatrixUpdated = false;
}

void MViewport::Input(MInputEvent* pEvent)
{
	if (m_pScene)
	{
		m_pScene->Input(pEvent, this);
	}
}

Matrix4 MViewport::GetLightInverseProjection(MPointLight* pLight)
{
	return Matrix4::IdentityMatrix;
}

Matrix4 MViewport::GetLightInverseProjection(MDirectionalLight* pLight)
{
	Matrix4 matLightInv(pLight->GetTransform().GetRotation());
	matLightInv = matLightInv.Inverse();

	std::vector<Vector3> points(8);

	MBoundsAABB* pBounds = m_pScene->GetSceneAABB();
	MCamera* pCamera = GetCamera();

	Matrix4 matCameraInv = pCamera->GetWorldTransform().Inverse();
	pBounds->GetPoints(points);
	float fZValidNear = FLT_MAX, fZValidFar = 0;

	for (unsigned int i = 0; i < 8; ++i)
	{
		float z = (matCameraInv * points[i]).z;
		if (fZValidNear > z)
			fZValidNear = z;
		if (fZValidFar < z)
			fZValidFar = z;
	}
	if (fZValidNear > pCamera->GetZNear())
		fZValidNear = pCamera->GetZNear();

//	GetCameraFrustum(GetCamera(), fZValidNear, fZValidFar, points[0], points[1], points[2], points[3], points[4], points[5], points[6], points[7]);


	Vector3 v3Min(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 v3Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (unsigned int i = 0; i < 8; ++i)
	{
		Vector3 pos = matLightInv * points[i];
		if (v3Min.x > pos.x)
			v3Min.x = pos.x;
		if (v3Min.y > pos.y)
			v3Min.y = pos.y;
		if (v3Min.z > pos.z)
			v3Min.z = pos.z;
		if (v3Max.x < pos.x)
			v3Max.x = pos.x;
		if (v3Max.y < pos.y)
			v3Max.y = pos.y;
		if (v3Max.z < pos.z)
			v3Max.z = pos.z;
	}

	Matrix4 projMat = MatrixOrthoOffCenterLH(v3Min.x, v3Max.x, v3Max.y, v3Min.y, v3Min.z, v3Max.z);
	return projMat * matLightInv;
}

void MViewport::GetCameraFrustum(MCamera* pCamera, const float& fZNear, const float& fZFar, Vector3& v3NearTopLeft, Vector3& v3NearTopRight, Vector3& v3NearBottomRight, Vector3& v3NearBottomLeft, Vector3& v3FarTopLeft, Vector3& v3FarTopRight, Vector3& v3FarBottomRight, Vector3& v3FarBottomLeft)
{
	UpdateMatrix();

	if (MCamera::EPerspective == pCamera->GetCameraType())
	{
		float fAspect = GetWidth() / GetHeight();
		float fHalfHeightDivideZ = (pCamera->GetFov() * 0.5f * M_PI / 180.0f);
		float fHalfWidthDivideZ = fHalfHeightDivideZ * fAspect;

		Matrix4 localToWorld = pCamera->GetWorldTransform();

		std::vector<Vector3> points(8);
		v3NearTopLeft = localToWorld * (Vector3(-fHalfWidthDivideZ, +fHalfHeightDivideZ, 1) * fZNear);
		v3NearTopRight = localToWorld * (Vector3(+fHalfWidthDivideZ, +fHalfHeightDivideZ, 1) * fZNear);
		v3NearBottomLeft = localToWorld * (Vector3(-fHalfWidthDivideZ, -fHalfHeightDivideZ, 1) * fZNear);
		v3NearBottomRight = localToWorld * (Vector3(+fHalfWidthDivideZ, -fHalfHeightDivideZ, 1) * fZNear);

		v3FarTopLeft = localToWorld * (Vector3(-fHalfWidthDivideZ, +fHalfHeightDivideZ, 1) * fZFar);
		v3FarTopRight = localToWorld * (Vector3(+fHalfWidthDivideZ, +fHalfHeightDivideZ, 1) * fZFar);
		v3FarBottomLeft = localToWorld * (Vector3(-fHalfWidthDivideZ, -fHalfHeightDivideZ, 1) * fZFar);
		v3FarBottomRight = localToWorld * (Vector3(+fHalfWidthDivideZ, -fHalfHeightDivideZ, 1) * fZFar);
	}
	else
	{
		std::vector<Vector3> points(8);

		Vector3 v3CameraPosition = pCamera->GetWorldPosition();
		float fHalfWidth = pCamera->GetWidth() * 0.5f;
		float fHalfHeight = pCamera->GetHeight() * 0.5f;

		Matrix4 localToWorld = pCamera->GetWorldTransform();

		v3NearTopLeft = localToWorld * Vector3(-fHalfWidth, +fHalfHeight, fZNear);
		v3NearTopRight = localToWorld * Vector3(+fHalfWidth, +fHalfHeight, fZNear);
		v3NearBottomLeft = localToWorld * Vector3(-fHalfWidth, -fHalfHeight, fZNear);
		v3NearBottomRight = localToWorld * Vector3(+fHalfWidth, -fHalfHeight, fZNear);

		v3FarTopLeft = localToWorld * Vector3(-fHalfWidth, +fHalfHeight, fZFar);
		v3FarTopRight = localToWorld * Vector3(+fHalfWidth, +fHalfHeight, fZFar);
		v3FarBottomLeft = localToWorld * Vector3(-fHalfWidth, -fHalfHeight, fZFar);
		v3FarBottomRight = localToWorld * Vector3(+fHalfWidth, -fHalfHeight, fZFar);
	}
}

void MViewport::GetCameraFrustum(Vector3& v3NearTopLeft, Vector3& v3NearTopRight, Vector3& v3NearBottomRight, Vector3& v3NearBottomLeft, Vector3& v3FarTopLeft, Vector3& v3FarTopRight, Vector3& v3FarBottomRight, Vector3& v3FarBottomLeft)
{
	MCamera* pCamera = GetCamera();
	GetCameraFrustum(pCamera, pCamera->GetZNear(), pCamera->GetZFar(), v3NearTopLeft, v3NearTopRight, v3NearBottomRight, v3NearBottomLeft, v3FarTopLeft, v3FarTopRight, v3FarBottomRight,  v3FarBottomLeft);
}

MBoundsAABB* MViewport::GetFrustumAABB()
{
	std::vector<Vector3> points(8);
	GetCameraFrustum(points[0], points[1], points[2], points[3], points[4], points[5], points[6], points[7]);

	return new MBoundsAABB(points);
}

void MViewport::SetScene(MScene* pScene)
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

void MViewport::SetCamera(MCamera* pCamera)
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

void MViewport::SetValidCamera(MCamera* pCamera)
{
	m_pUserCamera = pCamera;
}

void MViewport::UpdateMatrix()
{
	if (m_bCameraInvProjMatrixUpdated)
		return;

	//Update Camera and Projection Matrix.
	MCamera* pCamera = GetCamera();
	Matrix4 projMat = pCamera->GetCameraType() == MCamera::EPerspective
		? MatrixPerspectiveFovLH(pCamera->GetFov() * 0.5f, m_v2Size.x / m_v2Size.y, pCamera->GetZNear(), pCamera->GetZFar())
		: MatrixOrthoOffCenterLH(-pCamera->GetWidth() * 0.5f, pCamera->GetWidth() * 0.5f, pCamera->GetHeight() * 0.5f, -pCamera->GetHeight() * 0.5f, pCamera->GetZNear(), pCamera->GetZFar());

	m_m4CameraInvProj = projMat * pCamera->GetWorldTransform().Inverse();
}

MCamera* MViewport::GetCamera()
{
	return m_pUserCamera ? m_pUserCamera : m_pDefaultCamera;
}

Matrix4 MViewport::MatrixPerspectiveFovLH(const float& fFovYZAngle, const float& fScreenAspect, const float& fScreenNear, const float& fScreenFar)
{

	Matrix4 mProjMatrix;
	//清除为0
	ZeroMemory(&mProjMatrix.m, sizeof(mProjMatrix.m));

	//度数转化为弧度
	float angle = fFovYZAngle * M_PI / 180.0f;

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

Matrix4 MViewport::MatrixOrthoOffCenterLH(const float& fLeft, const float& fRight, const float& fTop, const float& fBottom, const float& fNear, const float& fFar)
{
	//warning, (fLeft >> fRight) and (fTop >> fBottom) and (fFar >> fNear)
	return Matrix4(2 / (fRight - fLeft), 0, 0, (fLeft + fRight) / (fLeft - fRight),
		0, 2 / (fTop - fBottom), 0, (fTop + fBottom) / (fBottom - fTop),
		0, 0, 1 / (fFar - fNear), fNear / (fNear - fFar),
		0,  0,  0,  1
	);
}
