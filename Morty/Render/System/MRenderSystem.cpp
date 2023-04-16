#include "System/MRenderSystem.h"


#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "Render/Vulkan/MVulkanDevice.h"
#include "Render/Vulkan/MVulkanRenderCommand.h"

#include "Component/MRenderableMeshComponent.h"

#include "Basic/MViewport.h"
#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "System/MObjectSystem.h"

MORTY_CLASS_IMPLEMENT(MRenderSystem, MISystem)

MRenderSystem::MRenderSystem()
	: MISystem()
	, m_pDevice(nullptr)
{

}

MRenderSystem::~MRenderSystem()
{

}

void MRenderSystem::Update(MTaskNode* pNode)
{
	m_pDevice->Update();

}

MIDevice* MRenderSystem::GetDevice() const
{
	return m_pDevice;
}

void MRenderSystem::Initialize()
{
	if (!m_pDevice)
	{
		m_pDevice = new MVulkanDevice();
		m_pDevice->SetEngine(GetEngine());
		m_pDevice->Initialize();
	}
}

void MRenderSystem::Release()
{
	if (m_pDevice)
	{
		m_pDevice->Release();
		delete m_pDevice;
		m_pDevice = nullptr;
	}
}

void MRenderSystem::ResizeFrameBuffer(MRenderPass& renderpass, const Vector2& v2Size)
{
	for (MRenderTarget& tex : renderpass.m_vBackTextures)
	{
		if (tex.pTexture->GetSize() != v2Size)
		{
			tex.pTexture->SetSize(v2Size);
			tex.pTexture->DestroyBuffer(GetDevice());
			tex.pTexture->GenerateBuffer(GetDevice());
		}
	}

	if (std::shared_ptr<MTexture> pDepthTexture = renderpass.GetDepthTexture())
	{
		if (pDepthTexture->GetSize() != v2Size)
		{
			pDepthTexture->SetSize(v2Size);
			pDepthTexture->DestroyBuffer(GetDevice());
			pDepthTexture->GenerateBuffer(GetDevice());
		}
	}

	if (renderpass.GetFrameBufferSize() != v2Size)
	{
		renderpass.Resize(GetDevice());
	}
}

void MRenderSystem::ReleaseRenderpass(MRenderPass& renderpass, bool bClearTexture)
{
	if (bClearTexture)
	{
		for (MRenderTarget& tex : renderpass.m_vBackTextures)
		{
			tex.pTexture->DestroyBuffer(GetDevice());
			tex.pTexture = nullptr;
		}

		if (std::shared_ptr<MTexture> pDepthTexture = renderpass.GetDepthTexture())
		{
			pDepthTexture->DestroyBuffer(GetDevice());
			pDepthTexture = nullptr;
		}
	}
	
	renderpass.m_vBackTextures.clear();
	renderpass.SetDepthTexture(nullptr, {});

	renderpass.DestroyBuffer(GetDevice());
}

Matrix4 MRenderSystem::GetCameraInverseProjection(const MViewport* pViewport, const MCameraComponent* pCameraComponent, MSceneComponent* pSceneComponent)
{
	return GetCameraInverseProjection(pViewport, pCameraComponent, pSceneComponent, pCameraComponent->GetZNear(), pCameraComponent->GetZFar());

}

Matrix4 MRenderSystem::GetCameraInverseProjection(const MViewport* pViewport, const MCameraComponent* pCameraComponent, MSceneComponent* pSceneComponent, float fZNear, float fZFar)
{
	//Update Camera and Projection Matrix.
	Matrix4 m4Projection = pCameraComponent->GetCameraType() == MCameraComponent::MECameraType::EPerspective
		? MRenderSystem::MatrixPerspectiveFovLH(pCameraComponent->GetFov() * 0.5f, pViewport->GetSize().x / pViewport->GetSize().y, fZNear, fZFar)
		: MRenderSystem::MatrixOrthoOffCenterLH(-pCameraComponent->GetWidth() * 0.5f, pCameraComponent->GetWidth() * 0.5f, pCameraComponent->GetHeight() * 0.5f, -pCameraComponent->GetHeight() * 0.5f, fZNear, fZFar);

	m4Projection = m4Projection * pSceneComponent->GetWorldTransform().Inverse();

	return m4Projection;

}

void MRenderSystem::GetCameraFrustumPoints(MEntity* pCamera, const Vector2& v2ViewportSize, const float& fZNear, const float& fZFar, std::vector<Vector3>& vPoints)
{
	GetCameraFrustumPoints(pCamera, v2ViewportSize, fZNear, fZFar, vPoints[0], vPoints[1], vPoints[2], vPoints[3], vPoints[4], vPoints[5], vPoints[6], vPoints[7]);
}

void MRenderSystem::GetCameraFrustumPoints(MEntity* pCamera
	, const Vector2& v2ViewportSize
	, const float& fZNear, const float& fZFar
	, Vector3& v3NearTopLeft, Vector3& v3NearTopRight
	, Vector3& v3NearBottomRight, Vector3& v3NearBottomLeft
	, Vector3& v3FarTopLeft, Vector3& v3FarTopRight
	, Vector3& v3FarBottomRight, Vector3& v3FarBottomLeft
) {

	MCameraComponent* pCameraComponent = pCamera->GetComponent<MCameraComponent>();
	if (nullptr == pCameraComponent)
		return;

	MSceneComponent* pSceneComponent = pCamera->GetComponent<MSceneComponent>();
	if (nullptr == pSceneComponent)
		return;

	if (MCameraComponent::MECameraType::EPerspective == pCameraComponent->GetCameraType())
	{
		float fAspect = v2ViewportSize.x / v2ViewportSize.y;
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

Matrix4 MRenderSystem::MatrixPerspectiveFovLH(const float& fFovYZAngle, const float& fScreenAspect, const float& fScreenNear, const float& fScreenFar)
{

	Matrix4 mProjMatrix;
	//���Ϊ0
	memset(&mProjMatrix, 0, sizeof(mProjMatrix));


	//����ת��Ϊ����
	float angle = fFovYZAngle * M_PI / 180.0f;

	//����������
	float s1 = 1 / (fScreenAspect * (float)tan(angle));
	float s2 = 1 / tan(angle);
	float a = fScreenFar / (fScreenFar - fScreenNear);
	float b = -(fScreenNear * fScreenFar) / (fScreenFar - fScreenNear);

	mProjMatrix.m[0][0] = s1;
	mProjMatrix.m[1][1] = s2;
	mProjMatrix.m[2][2] = a;
	mProjMatrix.m[2][3] = b;
	mProjMatrix.m[3][2] = 1.0f;

	return mProjMatrix;
}

Matrix4 MRenderSystem::MatrixOrthoOffCenterLH(const float& fLeft, const float& fRight, const float& fTop, const float& fBottom, const float& fNear, const float& fFar)
{
	//warning, (fLeft >> fRight) and (fTop >> fBottom) and (fFar >> fNear)
	return Matrix4(2 / (fRight - fLeft), 0, 0, (fLeft + fRight) / (fLeft - fRight),
		0, 2 / (fTop - fBottom), 0, (fTop + fBottom) / (fBottom - fTop),
		0, 0, 1 / (fFar - fNear), fNear / (fNear - fFar),
		0, 0, 0, 1
	);
}
