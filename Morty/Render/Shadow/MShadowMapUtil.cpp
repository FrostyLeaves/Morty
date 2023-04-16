#include "MShadowMapUtil.h"

#include "Scene/MScene.h"
#include "Render/MBuffer.h"
#include "Basic/MViewport.h"

#include "System/MRenderSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"

std::array<MCascadedShadowSceneData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> MShadowMapUtil::CascadedSplitCameraFrustum(MViewport* pViewport)
{
	MScene* pScene = pViewport->GetScene();
	MEntity* pCameraEntity = pViewport->GetCamera();

	MEntity* pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
	if (!pDirectionalLightEntity)
	{
		MORTY_ASSERT(pDirectionalLightEntity);
		return {};
	}

	const MSceneComponent* pLightSceneComponent = pDirectionalLightEntity->GetComponent<MSceneComponent>();
	if (!pLightSceneComponent)
	{
		MORTY_ASSERT(pLightSceneComponent);
		return {};
	}

	const MCameraComponent* pCameraComponent = pCameraEntity->GetComponent<MCameraComponent>();
	if (!pCameraComponent)
	{
		MORTY_ASSERT(pCameraComponent);
		return {};
	}
	MSceneComponent* pCameraSceneComponent = pCameraEntity->GetComponent<MSceneComponent>();
	if (!pCameraSceneComponent)
	{
		MORTY_ASSERT(pCameraSceneComponent);
		return {};
	}
	MComponentGroup<MRenderableMeshComponent>* pMeshComponentGroup = pScene->FindComponents<MRenderableMeshComponent>();
	if (!pMeshComponentGroup)
	{
		MORTY_ASSERT(pMeshComponentGroup);
		return {};
	}
	const Vector3 v3LightDir = pLightSceneComponent->GetForward();

	std::array<MCascadedShadowSceneData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vCascadedData;

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

		Matrix4 m4CameraInvProj = MRenderSystem::GetCameraInverseProjection(pViewport, pCameraComponent, pCameraSceneComponent, vCascadedData[nCascadedIdx].fNearZ, vCascadedData[nCascadedIdx].fFarZ);
		vCascadedData[nCascadedIdx].cCameraFrustum.UpdateFromCameraInvProj(m4CameraInvProj);
	}

	for (MRenderableMeshComponent& component : pMeshComponentGroup->m_vComponents)
	{
		if (!component.IsValid())
			continue;

		MEntity* pEntity = component.GetEntity();
		MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();

		if (pSceneComponent->GetVisibleRecursively() && component.GetGenerateDirLightShadow())
		{
			std::shared_ptr<MSkeletonInstance> pSkeletonInstance = component.GetSkeletonInstance();

			if (const MBoundsAABB* pBounds = component.GetBoundsAABB())
			{
				for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
				{
					MCascadedShadowSceneData& cd = vCascadedData[nCascadedIdx];

					if (cd.cCameraFrustum.ContainTest(*pBounds, v3LightDir) != MCameraFrustum::EOUTSIDE)
					{
						pBounds->UnionMinMax(cd.v3ShadowMin, cd.v3ShadowMax);
						cd.bGenerateShadow = true;
					}
				}
			}
		}
	}

	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		MCascadedShadowSceneData& cd = vCascadedData[nCascadedIdx];

		if (cd.bGenerateShadow)
		{
			cd.cPcsBounds.SetMinMax(cd.v3ShadowMin, cd.v3ShadowMax);
		}

	}

	return vCascadedData;
}

std::array<MCascadedShadowRenderData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>
	MShadowMapUtil::CalculateRenderData(MViewport* pViewport, MEntity* pCameraEntity,
	const std::array<MCascadedShadowSceneData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>& vCascadedData)
{

	MScene* pScene = pViewport->GetScene();

	MEntity* pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
	if (!pDirectionalLightEntity)
	{
		return {};
	}

	MSceneComponent* pLightSceneComponent = pDirectionalLightEntity->GetComponent<MSceneComponent>();
	if (!pLightSceneComponent)
	{
		MORTY_ASSERT(pLightSceneComponent);
		return {};
	}

	MCameraComponent* pCameraComponent = pCameraEntity->GetComponent<MCameraComponent>();
	if (!pCameraComponent)
	{
		return {};
	}

	const float fNearZ = pCameraComponent->GetZNear();
	const float fFarZ = pCameraComponent->GetZFar();

	std::vector<Vector3> vCameraFrustumPoints(8);
	MRenderSystem::GetCameraFrustumPoints(pCameraEntity, pViewport->GetSize(), fNearZ, fFarZ, vCameraFrustumPoints);

	Matrix4 matLight(pLightSceneComponent->GetTransform().GetRotation());
	Matrix4 matLightInv = matLight.Inverse();

	for (size_t i = 0; i < 8; ++i)
	{
		vCameraFrustumPoints[i] = matLightInv * vCameraFrustumPoints[i];
	}

	const float fCascadedTransitionRange = 0.25f;

	std::array<Matrix4, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vCascadeProjectionMatrix;
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
			fPscBoundsInLightSpaceMinZ,
			v3FrustumCenter.z + fBoundsSphereRadius
		);

		vCascadeFrustumRadius[nCascadedIdx] = fBoundsSphereRadius;
		vCascadeTransitionRange[nCascadedIdx] = fTransitionRange;
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


	std::array<MCascadedShadowRenderData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> cRenderData;
	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		const Vector4 v4NearZPositionNDCSpace = vCascadeProjectionMatrix[nCascadedIdx] * Vector4(0.0f, 0.0f, fMinLightSpaceZValue, 1.0f);

		cRenderData[nCascadedIdx].fSplitRange.x = fNearZ + (fFarZ - fNearZ) * vCascadedData[nCascadedIdx].fCascadeSplit;
		cRenderData[nCascadedIdx].fSplitRange.y = fNearZ + (fFarZ - fNearZ) * (vCascadedData[nCascadedIdx].fCascadeSplit + vCascadeTransitionRange[nCascadedIdx]);
		cRenderData[nCascadedIdx].fSplitRange.z = vCascadeFrustumRadius[nCascadedIdx] * 2.0f;
		cRenderData[nCascadedIdx].fSplitRange.w = v4NearZPositionNDCSpace.z / v4NearZPositionNDCSpace.w;
		cRenderData[nCascadedIdx].m4DirLightInvProj = vCascadeProjectionMatrix[nCascadedIdx] * matLightInv;
	}

	return cRenderData;
}

/*
Matrix4 MShadowMapUtil::GetLightInverseProjection_MaxBoundsSphere(MRenderInfo& info, const MBoundsAABB& cGenerateShadowAABB, float fZNear, float fZFar)
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

Matrix4 MShadowMapUtil::GetLightInverseProjection_MinBoundsAABB(MRenderInfo& info, const MBoundsAABB& cGenerateShadowAABB, float fZNear, float fZFar)
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
*/