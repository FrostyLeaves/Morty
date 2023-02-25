#include "MShadowMapRenderWork.h"

#include "MGPUCullingRenderWork.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Material/MMaterial.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"


MORTY_CLASS_IMPLEMENT(MShadowMapRenderWork, MObject)

MShadowMapRenderWork::MShadowMapRenderWork()
	: MObject()
{

}

MShadowMapRenderWork::~MShadowMapRenderWork()
{

}

void MShadowMapRenderWork::OnCreated()
{
	Super::OnCreated();

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	m_pShadowStaticMaterial = pResourceSystem->CreateResource<MMaterial>("Shadow Material");

	std::shared_ptr<MResource> pVertexShader = pResourceSystem->LoadResource("Shader/shadowmap.mvs");
	std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("Shader/shadowmap.mps");
	m_pShadowStaticMaterial->LoadVertexShader(pVertexShader);
	m_pShadowStaticMaterial->LoadPixelShader(pPixelShader);
	m_pShadowStaticMaterial->SetRasterizerType(MERasterizerType::ECullNone);

	m_pShadowSkeletonMaterial = pResourceSystem->CreateResource<MMaterial>("Shadow Material With Skeleton");
	m_pShadowSkeletonMaterial->GetShaderMacro().SetInnerMacro(MRenderGlobal::SHADER_SKELETON_ENABLE, "1");
	m_pShadowSkeletonMaterial->LoadVertexShader(pVertexShader);
	m_pShadowSkeletonMaterial->LoadPixelShader(pPixelShader);
	m_pShadowSkeletonMaterial->SetRasterizerType(MERasterizerType::ECullNone);

	m_pShadowGPUDrivenMaterial = pResourceSystem->CreateResource<MMaterial>("Shadow Material GPU Driven");
	m_pShadowGPUDrivenMaterial->LoadVertexShader(pVertexShader);
	m_pShadowGPUDrivenMaterial->LoadPixelShader(pPixelShader);
	m_pShadowGPUDrivenMaterial->SetRasterizerType(MERasterizerType::ECullNone);
	m_pShadowGPUDrivenMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::DRAW_MESH_MERGE_INSTANCING);

	std::shared_ptr<MTexture> pShadowMapTexture = MTexture::CreateShadowMapArray(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
	pShadowMapTexture->SetSize(Vector2(MRenderGlobal::SHADOW_TEXTURE_SIZE, MRenderGlobal::SHADOW_TEXTURE_SIZE));
	pShadowMapTexture->GenerateBuffer(pRenderSystem->GetDevice());


	m_renderPass.SetViewportNum(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
	m_renderPass.SetDepthTexture(pShadowMapTexture, { true, MColor::White });
	m_renderPass.GenerateBuffer(pRenderSystem->GetDevice());


	m_shadowPropertyBlock.BindMaterial(m_pShadowStaticMaterial);
}

void MShadowMapRenderWork::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	Super::OnDelete();

	m_shadowPropertyBlock.ReleaseShaderParamSet(GetEngine());

	m_pShadowStaticMaterial = nullptr;
	m_pShadowSkeletonMaterial = nullptr;
	m_pShadowGPUDrivenMaterial = nullptr;

	if (std::shared_ptr<MTexture> pDepthTexture = m_renderPass.GetDepthTexture())
	{
		pDepthTexture->DestroyBuffer(pRenderSystem->GetDevice());
		pDepthTexture = nullptr;
	}

	m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

void MShadowMapRenderWork::RenderShadow(MRenderInfo& info, MGPUCullingRenderWork* pGPUCullingRenderWork)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	if (!pCommand)
		return;

	std::shared_ptr<MTexture> pShadowmap = GetShadowMap();

	size_t nCascadedSize = info.cCascadedShadow.size();
	if (nCascadedSize == 0)
	{
		return;
	}

	Vector2 v2LeftTop = Vector2(0.0f, 0.0f);
	Vector2 v2Size = pShadowmap->GetSize();

	float x = v2LeftTop.x;
	float y = v2LeftTop.y;
	float w = v2Size.x;
	float h = v2Size.y;


	pCommand->BeginRenderPass(&m_renderPass);

	pCommand->SetViewport(MViewportInfo(x, y, w, h));
	pCommand->SetScissor(MScissorInfo(x, y, w, h));

	DrawShadowMesh(info, pCommand);
	DrawShadowMesh(pGPUCullingRenderWork, pCommand);

	pCommand->EndRenderPass();
}

void MShadowMapRenderWork::DrawShadowMesh(MRenderInfo& info, MIRenderCommand* pCommand)
{
	
	auto& materialGroup = info.m_tShadowGroupMesh;
	for (auto& pr : materialGroup)
	{
		if (std::shared_ptr<MSkeletonInstance> pSkeletonIns = pr.first)
		{
			pCommand->SetUseMaterial(m_pShadowSkeletonMaterial);
			pCommand->SetShaderParamSet(pSkeletonIns->GetShaderParamSet());
		}
		else
		{
			pCommand->SetUseMaterial(m_pShadowStaticMaterial);
		}

		pCommand->SetShaderParamSet(m_shadowPropertyBlock.GetShaderPropertyBlock());

		std::vector<MRenderableMeshComponent*>& vMesh = pr.second;

		for (MRenderableMeshComponent* pMeshComponent : vMesh)
		{
			pCommand->SetShaderParamSet(pMeshComponent->GetShaderMeshParamSet());
			pCommand->DrawMesh(pMeshComponent->GetMesh());
		}
	}
}

void MShadowMapRenderWork::DrawShadowMesh(MGPUCullingRenderWork* pGPUCullingRenderWork, MIRenderCommand* pCommand)
{
	if (pGPUCullingRenderWork)
	{
		const MBuffer* pDrawIndirectBuffer = pGPUCullingRenderWork->GetDrawIndirectBuffer();
		const std::vector<MMaterialCullingGroup>& vCullingInstanceGroup = pGPUCullingRenderWork->GetCullingInstanceGroup();
		for (const MMaterialCullingGroup& group : vCullingInstanceGroup)
		{
			pCommand->SetUseMaterial(m_pShadowGPUDrivenMaterial);
			pCommand->SetShaderParamSet(m_shadowPropertyBlock.GetShaderPropertyBlock());
			pCommand->SetShaderParamSet(group.pMeshTransformProperty);

			pCommand->DrawIndexedIndirect(group.pVertexBuffer, group.pIndexBuffer, pDrawIndirectBuffer, group.nClusterBeginIdx * sizeof(MDrawIndexedIndirectData), group.nClusterCount);
		}
	}
}

Matrix4 MShadowMapRenderWork::GetLightInverseProjection_MinBoundsAABB(MRenderInfo& info, const MBoundsAABB& cGenerateShadowAABB, float fZNear, float fZFar)
{
	if (nullptr == info.pDirectionalLightEntity)
		return Matrix4::IdentityMatrix;

	MSceneComponent* pLightSceneComponent = info.pDirectionalLightEntity->GetComponent<MSceneComponent>();
	if (nullptr == pLightSceneComponent)
		return Matrix4::IdentityMatrix;

	MCameraComponent* pCameraComponent = info.pCameraEntity->GetComponent<MCameraComponent>();
	if (nullptr == pCameraComponent)
		return Matrix4::IdentityMatrix;

	MSceneComponent* pCameraSceneComponent = info.pCameraEntity->GetComponent<MSceneComponent>();
	if (nullptr == pCameraSceneComponent)
		return Matrix4::IdentityMatrix;

	Matrix4 matLight(pLightSceneComponent->GetTransform().GetRotation());
	Matrix4 matLightInv = matLight.Inverse();

	Matrix4 matCameraInv = pCameraSceneComponent->GetWorldTransform().Inverse();

	std::vector<Vector3> vSceneBoundsPoints(8);
	info.cCaclSceneRenderAABB.GetPoints(vSceneBoundsPoints);

	//�����������ЧZNear��ZFar.
	float fSceneMinZNear = FLT_MAX, fSceneMaxZFar = -FLT_MAX;
	for (uint32_t i = 0; i < 8; ++i)
	{
		float z = (matCameraInv * vSceneBoundsPoints[i]).z;

		if (fSceneMinZNear > z)
			fSceneMinZNear = z;
		if (fSceneMaxZFar < z)
			fSceneMaxZFar = z;
	}
	float fZValidNear = fSceneMinZNear > fZNear ? fSceneMinZNear : fZNear;
	float fZValidFar = fSceneMaxZFar < fZFar ? fSceneMaxZFar : fZFar;

	//��ȡ�����׵���ڷ����Camera�ڵ���С�����X��Yֵ
	std::vector<Vector3> vCameraBoundsPoints(8);
	MRenderSystem::GetCameraFrustumPoints(info.pCameraEntity, info.pViewport->GetSize(), fZValidNear, (std::max)(fZValidNear, fZValidFar), vCameraBoundsPoints);

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
	cGenerateShadowAABB.GetPoints(vShadowModelBoundsPoints);

	//����Scene��AABB���ڷ����Camera�ڵ���С�����Zֵ
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

	//x��yȡ��׵���SceneAABB�Ľ����� zMinȡSceneAABB�ģ���Ϊ��������ģ��Ҳ������Shadow
	//zMaxȡ������������׵���Shadow����Ҫ��Ⱦ��

	float fLeft = (std::max)(v3CameraMin.x, v3SceneMin.x);
	float fRight = (std::min)(v3CameraMax.x, v3SceneMax.x);
	float fBottom = (std::max)(v3CameraMin.y, v3SceneMin.y);
	float fTop = (std::min)(v3CameraMax.y, v3SceneMax.y);

	float width = fRight - fLeft;
	float height = fTop - fBottom;

	float size = (std::max)(width, height);
	float centerX = (fLeft + fRight) * 0.5f;
	float centerY = (fBottom + fTop) * 0.5f;

	/*
	Matrix4 projMat = MRenderSystem::MatrixOrthoOffCenterLH(
		fLeft,
		fLeft + (std::max)(width, height),
		fBottom + (std::max)(width, height),
		fBottom,
		v3SceneMin.z,
		(std::min)(v3CameraMax.z, v3SceneMax.z)
	);
	*/

	Matrix4 projMat = MRenderSystem::MatrixOrthoOffCenterLH(
		centerX - size * 0.5f,
		centerX + size * 0.5f,
		centerY + size * 0.5f,
		centerY - size * 0.5f,
		v3SceneMin.z,
		(std::min)(v3CameraMax.z, v3SceneMax.z)
	);

	return projMat * matLightInv;
}

Matrix4 MShadowMapRenderWork::GetLightInverseProjection_MaxBoundsSphere(MRenderInfo& info, const MBoundsAABB& cGenerateShadowAABB, float fZNear, float fZFar)
{
	if (nullptr == info.pDirectionalLightEntity)
		return Matrix4::IdentityMatrix;

	MSceneComponent* pLightSceneComponent = info.pDirectionalLightEntity->GetComponent<MSceneComponent>();
	if (nullptr == pLightSceneComponent)
		return Matrix4::IdentityMatrix;

	MCameraComponent* pCameraComponent = info.pCameraEntity->GetComponent<MCameraComponent>();
	if (nullptr == pCameraComponent)
		return Matrix4::IdentityMatrix;

	MSceneComponent* pCameraSceneComponent = info.pCameraEntity->GetComponent<MSceneComponent>();
	if (nullptr == pCameraSceneComponent)
		return Matrix4::IdentityMatrix;

	Matrix4 matLight(pLightSceneComponent->GetTransform().GetRotation());
	Matrix4 matLightInv = matLight.Inverse();

	Matrix4 matCameraInv = pCameraSceneComponent->GetWorldTransform().Inverse();

	std::vector<Vector3> vSceneBoundsPoints(8);
	info.cCaclSceneRenderAABB.GetPoints(vSceneBoundsPoints);

	//�����������ЧZNear��ZFar.
	float fSceneMinZNear = FLT_MAX, fSceneMaxZFar = -FLT_MAX;
	for (uint32_t i = 0; i < 8; ++i)
	{
		float z = (matCameraInv * vSceneBoundsPoints[i]).z;

		if (fSceneMinZNear > z)
			fSceneMinZNear = z;
		if (fSceneMaxZFar < z)
			fSceneMaxZFar = z;
	}

	//��ȡ�����׵���ڷ����Camera�ڵ���С�����X��Yֵ
	std::vector<Vector3> vCameraBoundsPoints(8);
	MRenderSystem::GetCameraFrustumPoints(info.pCameraEntity, info.pViewport->GetSize(), fZNear, fZFar, vCameraBoundsPoints);

	MBoundsSphere cameraFrustumSphere;

	float fCameraBoundsLightSpaceMaxZ = -FLT_MAX;
	for (uint32_t i = 0; i < 8; ++i)
	{
		vCameraBoundsPoints[i] = matLightInv * vCameraBoundsPoints[i];

		if (fCameraBoundsLightSpaceMaxZ < vCameraBoundsPoints[i].z)
			fCameraBoundsLightSpaceMaxZ = vCameraBoundsPoints[i].z;

		vCameraBoundsPoints[i].z = 0.0f;
	}

	cameraFrustumSphere.SetPoints(vCameraBoundsPoints.data(), vCameraBoundsPoints.size(), 0, sizeof(Vector3));

	std::vector<Vector3> vShadowModelBoundsPoints(8);
	cGenerateShadowAABB.GetPoints(vShadowModelBoundsPoints);

	//����Scene��AABB���ڷ����Camera�ڵ���С�����Zֵ
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

	//x��yȡ��׵���SceneAABB�Ľ����� zMinȡSceneAABB�ģ���Ϊ��������ģ��Ҳ������Shadow
	//zMaxȡ������������׵���Shadow����Ҫ��Ⱦ��

	float radius = cameraFrustumSphere.m_fRadius * 0.5f;
	float centerX = cameraFrustumSphere.m_v3CenterPoint.x;
	float centerY = cameraFrustumSphere.m_v3CenterPoint.y;
	float centerZ = cameraFrustumSphere.m_v3CenterPoint.z;

	Matrix4 projMat = MRenderSystem::MatrixOrthoOffCenterLH(
		centerX - radius,
		centerX + radius,
		centerY + radius,
		centerY - radius,
		v3SceneMin.z,
		(std::min)(fCameraBoundsLightSpaceMaxZ, v3SceneMax.z)
	);

	return projMat * matLightInv;
}

void MShadowMapRenderWork::CalculateFrustumForCascadesShadowMap(MRenderInfo& info, const std::array<CascadedData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>& vCascadedData)
{
	if (!info.pCameraEntity)
	{
		return;
	}

	MSceneComponent* pLightSceneComponent = info.pDirectionalLightEntity->GetComponent<MSceneComponent>();
	if (!pLightSceneComponent)
		return;

	MCameraComponent* pCameraComponent = info.pCameraEntity->GetComponent<MCameraComponent>();
	if (!pCameraComponent)
		return;

	const float fNearZ = pCameraComponent->GetZNear();
	const float fFarZ = pCameraComponent->GetZFar();

	std::vector<Vector3> vCameraFrustumPoints(8);
	MRenderSystem::GetCameraFrustumPoints(info.pCameraEntity, info.pViewport->GetSize(), fNearZ, fFarZ, vCameraFrustumPoints);

	Matrix4 matLight(pLightSceneComponent->GetTransform().GetRotation());
	Matrix4 matLightInv = matLight.Inverse();

	for (size_t i = 0; i < 8; ++i)
	{
		vCameraFrustumPoints[i] = matLightInv * vCameraFrustumPoints[i];
	}
	
	const float fCascadedTransitionRange = 0.25f;

	std::array<Matrix4, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vCascadeProjectionMatrix;
	std::array<Vector3, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vCascadeFrustumCenter;
	std::array<float, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vCascadeFrustumRadius;
	std::array<float, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vCascadeTransitionRange;
	std::array<float, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vPscBoundsInLightSpaceMinZ;


	float fLastSplitDist = 0.0f;
	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		std::vector<Vector3> vCascadedFrustumPoints = vCameraFrustumPoints;

		float fTransitionRange = (vCascadedData[nCascadedIdx].fCascadeSplit - fLastSplitDist) * fCascadedTransitionRange;

		Vector3 v3FrustumCenter = Vector3(0.0f, 0.0f, 0.0f);
		for (size_t nPointIdx = 0; nPointIdx < 4; ++nPointIdx)
		{
			Vector3 dist = vCascadedFrustumPoints[nPointIdx + 4] - vCascadedFrustumPoints[nPointIdx];

			vCascadedFrustumPoints[nPointIdx + 4] = vCascadedFrustumPoints[nPointIdx] + dist * std::min(vCascadedData[nCascadedIdx].fCascadeSplit + fTransitionRange, 1.0f);
			vCascadedFrustumPoints[nPointIdx] = vCascadedFrustumPoints[nPointIdx] + (dist * fLastSplitDist);
			fLastSplitDist = vCascadedData[nCascadedIdx].fCascadeSplit;

			v3FrustumCenter += vCascadedFrustumPoints[nPointIdx];
			v3FrustumCenter += vCascadedFrustumPoints[nPointIdx + 4];

		}

		v3FrustumCenter = v3FrustumCenter / 8.0f;

		float fBoundsSphereRadius = 0.0f;
		for (uint32_t nPointIdx = 0; nPointIdx < 8; ++nPointIdx)
		{
			float fDistance = (vCascadedFrustumPoints[nPointIdx] - v3FrustumCenter).Length();
			fBoundsSphereRadius = (std::max)(fBoundsSphereRadius, fDistance);
		}
		fBoundsSphereRadius = std::ceil(fBoundsSphereRadius * 16.0f) / 16.0f;


		float fPscBoundsInLightSpaceMinZ = FLT_MAX;
		if (vCascadedData[nCascadedIdx].bGenerateShadow)
		{
			std::vector<Vector3> vPscBoundsPoints(8);
			vCascadedData[nCascadedIdx].cPcsBounds.GetPoints(vPscBoundsPoints);
			for (uint32_t i = 0; i < 8; ++i)
			{
				const float z = (matLightInv * vPscBoundsPoints[i]).z;
				if (fPscBoundsInLightSpaceMinZ > z)
				{
					fPscBoundsInLightSpaceMinZ = z;
				}
			}
		}
        else
        {
			fPscBoundsInLightSpaceMinZ = v3FrustumCenter.z - fBoundsSphereRadius;
        }


		vCascadeProjectionMatrix[nCascadedIdx] = MRenderSystem::MatrixOrthoOffCenterLH(
			v3FrustumCenter.x - fBoundsSphereRadius,
			v3FrustumCenter.x + fBoundsSphereRadius,
			v3FrustumCenter.y + fBoundsSphereRadius,
			v3FrustumCenter.y - fBoundsSphereRadius,
			//(std::min)(v3FrustumCenter.z - fBoundsSphereRadius, fPscBoundsInLightSpaceMinZ),
			fPscBoundsInLightSpaceMinZ,
			v3FrustumCenter.z + fBoundsSphereRadius
		);

		vCascadeFrustumCenter[nCascadedIdx] = v3FrustumCenter;
		vCascadeFrustumRadius[nCascadedIdx] = fBoundsSphereRadius;
		vCascadeTransitionRange[nCascadedIdx] = fTransitionRange;
		//vPscBoundsInLightSpaceMinZ[nCascadedIdx] = v3FrustumCenter.z - fBoundsSphereRadius;
		vPscBoundsInLightSpaceMinZ[nCascadedIdx] = fPscBoundsInLightSpaceMinZ;
	}

	float fMinLightSpaceZValue = FLT_MAX;
	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		if (fMinLightSpaceZValue > vPscBoundsInLightSpaceMinZ[nCascadedIdx])
		{
			fMinLightSpaceZValue = vPscBoundsInLightSpaceMinZ[nCascadedIdx];
		}
	}

    for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		const Vector4 v4NearZPositionNDCSpace = vCascadeProjectionMatrix[nCascadedIdx] * Vector4(0.0f, 0.0f, fMinLightSpaceZValue, 1.0f);

		info.cCascadedShadow[nCascadedIdx].fSplitRange.x = fNearZ + (fFarZ - fNearZ) * vCascadedData[nCascadedIdx].fCascadeSplit;
		info.cCascadedShadow[nCascadedIdx].fSplitRange.y = fNearZ + (fFarZ - fNearZ) * (vCascadedData[nCascadedIdx].fCascadeSplit + vCascadeTransitionRange[nCascadedIdx]);
		info.cCascadedShadow[nCascadedIdx].fSplitRange.z = vCascadeFrustumRadius[nCascadedIdx] * 2.0f;
		info.cCascadedShadow[nCascadedIdx].fSplitRange.w = v4NearZPositionNDCSpace.z / v4NearZPositionNDCSpace.w;
		info.cCascadedShadow[nCascadedIdx].m4DirLightInvProj = vCascadeProjectionMatrix[nCascadedIdx] * matLightInv;
	}
}

void MShadowMapRenderWork::UpdateShadowParams(MRenderInfo& info)
{
	m_shadowPropertyBlock.UpdateShaderSharedParams(info);
}

std::shared_ptr<MTexture> MShadowMapRenderWork::GetShadowMap()
{
	return m_renderPass.GetDepthTexture();
}

void MShadowMapRenderWork::CollectShadowMesh(MRenderInfo& info)
{
	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	const MViewport* pViewport = info.pViewport;

	if (!pViewport)
	{
		MORTY_ASSERT(pViewport);
		return;
	}
	MScene* pScene = pViewport->GetScene();
	if (!pScene)
		return;

	if (!info.pDirectionalLightEntity)
	{
		info.pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
		if (!info.pDirectionalLightEntity)
		{
			MORTY_ASSERT(info.pDirectionalLightEntity);
			return;
		}
	}

	const MSceneComponent* pLightSceneComponent = info.pDirectionalLightEntity->GetComponent<MSceneComponent>();
	if (!pLightSceneComponent)
	{
		MORTY_ASSERT(pLightSceneComponent);
		return;
	}
	const MCameraComponent* pCameraComponent = info.pCameraEntity->GetComponent<MCameraComponent>();
	if (!pCameraComponent)
	{
		MORTY_ASSERT(pCameraComponent);
		return;
	}
	MSceneComponent* pCameraSceneComponent = info.pCameraEntity->GetComponent<MSceneComponent>();
	if (!pCameraSceneComponent)
	{
		MORTY_ASSERT(pCameraSceneComponent);
		return;
	}
	MComponentGroup<MRenderableMeshComponent>* pMeshComponentGroup = pScene->FindComponents<MRenderableMeshComponent>();
	if (!pMeshComponentGroup)
	{
		MORTY_ASSERT(pMeshComponentGroup);
		return;
	}
	const Vector3 v3LightDir = pLightSceneComponent->GetForward();

	std::array<CascadedData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vCascadedData;

	const float fCascadeSplitLambda = 0.95f;

	const float fNearZ = pCameraComponent->GetZNear();
	const float fFarZ = pCameraComponent->GetZFar();

	const float fRange = fFarZ - fNearZ;
	const float fRatio = fFarZ / fNearZ;

	float fLastSplit = 0.0f;
	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		const float p = (nCascadedIdx + 1) / static_cast<float>(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
		const float log = fNearZ * std::pow(fRatio, p);
		const float uniform = fNearZ + fRange * p;
		const float d = fCascadeSplitLambda * (log - uniform) + uniform;

		const float fSplit = (d - fNearZ) / fRange;
		vCascadedData[nCascadedIdx].fCascadeSplit = fSplit;
		vCascadedData[nCascadedIdx].fNearZ = fNearZ + fRange * fLastSplit;
		vCascadedData[nCascadedIdx].fFarZ = fNearZ + fRange * fSplit;
		fLastSplit = fSplit;

		Matrix4 m4CameraInvProj = pRenderSystem->GetCameraInverseProjection(pViewport, pCameraComponent, pCameraSceneComponent, vCascadedData[nCascadedIdx].fNearZ, vCascadedData[nCascadedIdx].fFarZ);
		vCascadedData[nCascadedIdx].cCameraFrustum.UpdateFromCameraInvProj(m4CameraInvProj);
	}

	for (MRenderableMeshComponent& component : pMeshComponentGroup->m_vComponents)
	{
		if (!component.IsValid())
			continue;

		if (std::shared_ptr<MMaterial> pMaterial = component.GetMaterial())
		{
			if (pMaterial->GetMaterialType() != MEMaterialType::EDefault && pMaterial->GetMaterialType() != MEMaterialType::EDeferred)
				continue;
		}

		if (component.GetBatchInstanceEnable())
		{
			continue;
		}

		MEntity* pEntity = component.GetEntity();

		MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();

		if (pSceneComponent->GetVisibleRecursively() && component.GetGenerateDirLightShadow())
		{
			std::shared_ptr<MSkeletonInstance> pSkeletonInstance = component.GetSkeletonInstance();

			if (const MBoundsAABB* pBounds = component.GetBoundsAABB())
			{
				bool bRenderEnable = false;

				for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
				{
					CascadedData& cd = vCascadedData[nCascadedIdx];

					if (cd.cCameraFrustum.ContainTest(*pBounds, v3LightDir) != MCameraFrustum::EOUTSIDE)
					{
						pBounds->UnionMinMax(cd.v3ShadowMin, cd.v3ShadowMax);
						cd.bGenerateShadow = true;
						bRenderEnable = true;
					}
				}

				if (bRenderEnable)
				{
					auto& vMeshes = info.m_tShadowGroupMesh[pSkeletonInstance];
					vMeshes.push_back(&component);
				}
			}
		}
	}

	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		CascadedData& cd = vCascadedData[nCascadedIdx];

		if (cd.bGenerateShadow)
		{
			cd.cPcsBounds.SetMinMax(cd.v3ShadowMin, cd.v3ShadowMax);
		}
		else
		{
			cd.cPcsBounds = info.cCaclSceneRenderAABB;
		}

	}

	CalculateFrustumForCascadesShadowMap(info, vCascadedData);
}

