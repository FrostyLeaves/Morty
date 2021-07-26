#include "MViewport.h"

#include "MEntity.h"
#include "MScene.h"
#include "MEngine.h"

#include "MSceneComponent.h"
#include "MCameraComponent.h"
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
	, m_m4CameraInvProj(Matrix4::IdentityMatrix)
	, m_bCameraInvProjMatrixLocked(false)
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
		return false;

	UpdateMatrix();

	Vector4 pos = m_m4CameraInvProj * Vector4(v3WorldPos, 1.0f);
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
		return;

	MSceneComponent* pSceneComponent = GetCamera()->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
		return;

	UpdateMatrix();

	Matrix4 mat = m_m4CameraInvProj.Inverse();

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
		return false;

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
		return false;

	UpdateMatrix();

	Vector4 v4Rst = m_m4CameraInvProj * Vector4(v3Pos, 1.0f);
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
void MViewport::LockMatrix()
{
	UpdateMatrix();
	m_cameraFrustum.UpdateFromCameraInvProj(this->GetCameraInverseProjection());
	m_bCameraInvProjMatrixLocked = true;
}

void MViewport::UnlockMatrix()
{
	m_bCameraInvProjMatrixLocked = false;
}

void MViewport::Input(MInputEvent* pEvent)
{
}

Matrix4 MViewport::GetLightInverseProjection(MPointLight* pLight)
{
	return Matrix4::IdentityMatrix;
}

Matrix4 MViewport::GetLightInverseProjection(MEntity* pLight, const MBoundsAABB& cMeshRenderAABB, const MBoundsAABB& cShadowRenderAABB)
{
	if (nullptr == pLight)
		return Matrix4::IdentityMatrix;

	MSceneComponent* pLightSceneComponent = pLight->GetComponent<MSceneComponent>();
	if (nullptr == pLightSceneComponent)
		return Matrix4::IdentityMatrix;
	
	MCameraComponent* pCameraComponent = GetCamera()->GetComponent<MCameraComponent>();
	if (nullptr == pCameraComponent)
		return Matrix4::IdentityMatrix;

	MSceneComponent* pCameraSceneComponent = GetCamera()->GetComponent<MSceneComponent>();
	if (nullptr == pCameraSceneComponent)
		return Matrix4::IdentityMatrix;

	Matrix4 matLight(pLightSceneComponent->GetTransform().GetRotation());
	Matrix4 matLightInv = matLight.Inverse();

	MEntity* pCamera = GetCamera();
	Matrix4 matCameraInv = pCameraSceneComponent->GetWorldTransform().Inverse();

	std::vector<Vector3> vSceneBoundsPoints(8);
	cMeshRenderAABB.GetPoints(vSceneBoundsPoints);

	//计算相机的有效ZNear和ZFar.
	float fSceneMinZNear = FLT_MAX, fSceneMaxZFar = -FLT_MAX;
	for (uint32_t i = 0; i < 8; ++i)
	{
		float z = (matCameraInv * vSceneBoundsPoints[i]).z;

		if (fSceneMinZNear > z)
			fSceneMinZNear = z;
		if (fSceneMaxZFar < z)
			fSceneMaxZFar = z;
	}
	float fZValidNear = fSceneMinZNear > pCameraComponent->GetZNear() ? fSceneMinZNear : pCameraComponent->GetZNear();
	float fZValidFar = fSceneMaxZFar < pCameraComponent->GetZFar() ? fSceneMaxZFar : pCameraComponent->GetZFar();

	//获取相机视椎体在方向光Camera内的最小和最大X、Y值
	std::vector<Vector3> vCameraBoundsPoints(8);
	GetCameraFrustum(GetCamera(), fZValidNear, MAX(fZValidNear, fZValidFar), vCameraBoundsPoints);

	MBoundsAABB aabbCameraFrustum(vCameraBoundsPoints);
	aabbCameraFrustum.GetPoints(vCameraBoundsPoints);

	Vector3 v3CameraMin(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 v3CameraMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (uint32_t i = 0; i < 8; ++i)
	{
		Vector3 pos = matLightInv * vCameraBoundsPoints[i];
		if (v3CameraMin.x > pos.x) v3CameraMin.x = pos.x;
		if (v3CameraMin.y > pos.y) v3CameraMin.y = pos.y;
		if (v3CameraMin.z > pos.z) v3CameraMin.z = pos.z;

		if (v3CameraMax.x < pos.x) v3CameraMax.x = pos.x;
		if (v3CameraMax.y < pos.y) v3CameraMax.y = pos.y;
		if (v3CameraMax.z < pos.z) v3CameraMax.z = pos.z;
	}

	std::vector<Vector3> vShadowModelBoundsPoints(8);
	cShadowRenderAABB.GetPoints(vShadowModelBoundsPoints);

	//计算Scene的AABB盒在方向光Camera内的最小和最大Z值
	Vector3 v3SceneMin(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 v3SceneMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (uint32_t i = 0; i < 8; ++i)
	{
		Vector3 pos = matLightInv * vShadowModelBoundsPoints[i];
		if (v3SceneMin.x > pos.x) v3SceneMin.x = pos.x;
		if (v3SceneMin.y > pos.y) v3SceneMin.y = pos.y;
		if (v3SceneMin.z > pos.z) v3SceneMin.z = pos.z;

		if (v3SceneMax.x < pos.x) v3SceneMax.x = pos.x;
		if (v3SceneMax.y < pos.y) v3SceneMax.y = pos.y;
		if (v3SceneMax.z < pos.z) v3SceneMax.z = pos.z;
	}
	
	//x和y取视椎体和SceneAABB的交集， zMin取SceneAABB的，因为相机后面的模型也会生成Shadow
	//zMax取交集，超过视椎体的Shadow不需要渲染。

	float fLeft = MAX(v3CameraMin.x, v3SceneMin.x);
	float fRight = MIN(v3CameraMax.x, v3SceneMax.x);
	float fBottom = MAX(v3CameraMin.y, v3SceneMin.y);
	float fTop = MIN(v3CameraMax.y, v3SceneMax.y);
	
	float width = fRight - fLeft;
	float height = fTop - fBottom;

	Matrix4 projMat = MatrixOrthoOffCenterLH(
		fLeft,
		fLeft + MAX(width, height),
		fBottom + MAX(width, height),
		fBottom,
		v3SceneMin.z,
		MIN(v3CameraMax.z, v3SceneMax.z)
	);

	return projMat * matLightInv;
}

void MViewport::GetCameraFrustum(MEntity* pCamera, const float& fZNear, const float& fZFar, Vector3& v3NearTopLeft, Vector3& v3NearTopRight, Vector3& v3NearBottomRight, Vector3& v3NearBottomLeft, Vector3& v3FarTopLeft, Vector3& v3FarTopRight, Vector3& v3FarBottomRight, Vector3& v3FarBottomLeft)
{
	MCameraComponent* pCameraComponent = GetCamera()->GetComponent<MCameraComponent>();
	if (nullptr == pCameraComponent)
		return;

	MSceneComponent* pSceneComponent = GetCamera()->GetComponent<MSceneComponent>();
	if (nullptr == pSceneComponent)
		return;

	UpdateMatrix();

	if (MCameraComponent::MECameraType::EPerspective == pCameraComponent->GetCameraType())
	{
		float fAspect = GetWidth() / GetHeight();
		float fHalfHeightDivideZ = (pCameraComponent->GetFov() * 0.5f * M_PI / 180.0f);
		float fHalfWidthDivideZ = fHalfHeightDivideZ * fAspect;

		Matrix4 localToWorld = pSceneComponent->GetWorldTransform();

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
		float fHalfWidth = pCameraComponent->GetWidth() * 0.5f;
		float fHalfHeight = pCameraComponent->GetHeight() * 0.5f;

		Matrix4 localToWorld = pSceneComponent->GetWorldTransform();

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
	MCameraComponent* pCameraComponent = GetCamera()->GetComponent<MCameraComponent>();
	if (nullptr == pCameraComponent)
		return;

	MEntity* pCamera = GetCamera();
	GetCameraFrustum(pCamera, pCameraComponent->GetZNear(), pCameraComponent->GetZFar(), v3NearTopLeft, v3NearTopRight, v3NearBottomRight, v3NearBottomLeft, v3FarTopLeft, v3FarTopRight, v3FarBottomRight,  v3FarBottomLeft);
}

void MViewport::GetCameraFrustum(MEntity* pCamera, const float& fZNear, const float& fZFar, std::vector<Vector3>& vPoints)
{
	GetCameraFrustum(pCamera, fZNear, fZFar, vPoints[0], vPoints[1], vPoints[2], vPoints[3], vPoints[4], vPoints[5], vPoints[6], vPoints[7]);
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

void MViewport::UpdateMatrix()
{
	if (m_bCameraInvProjMatrixLocked)
		return;

	MCameraComponent* pCameraComponent = GetCamera()->GetComponent<MCameraComponent>();
	if (nullptr == pCameraComponent)
		return;

	MSceneComponent* pSceneComponent = GetCamera()->GetComponent<MSceneComponent>();
	if (nullptr == pSceneComponent)
		return;

	//Update Camera and Projection Matrix.
	MEntity* pCamera = GetCamera();
	Matrix4 projMat = pCameraComponent->GetCameraType() == MCameraComponent::MECameraType::EPerspective
		? MatrixPerspectiveFovLH(pCameraComponent->GetFov() * 0.5f, m_v2Size.x / m_v2Size.y, pCameraComponent->GetZNear(), pCameraComponent->GetZFar())
		: MatrixOrthoOffCenterLH(-pCameraComponent->GetWidth() * 0.5f, pCameraComponent->GetWidth() * 0.5f, pCameraComponent->GetHeight() * 0.5f, -pCameraComponent->GetHeight() * 0.5f, pCameraComponent->GetZNear(), pCameraComponent->GetZFar());

	m_m4CameraInvProj = projMat * pSceneComponent->GetWorldTransform().Inverse();
}

MEntity* MViewport::GetCamera() const
{
	return m_pUserCamera;
}

Matrix4 MViewport::MatrixPerspectiveFovLH(const float& fFovYZAngle, const float& fScreenAspect, const float& fScreenNear, const float& fScreenFar)
{

	Matrix4 mProjMatrix;
	//清除为0
	memset(&mProjMatrix, 0, sizeof(mProjMatrix));


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
