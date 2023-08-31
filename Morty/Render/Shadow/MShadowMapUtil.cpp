#include "MShadowMapUtil.h"

#include "Scene/MScene.h"
#include "Render/MBuffer.h"
#include "Basic/MViewport.h"

#include "System/MRenderSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderMeshComponent.h"
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
	MComponentGroup<MRenderMeshComponent>* pMeshComponentGroup = pScene->FindComponents<MRenderMeshComponent>();
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
		//const float fSplit = float(nCascadedIdx + 1) / (MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
		const float fCascadedTransitionRange = 0.25f;
		float fTransitionRange = (fSplit - fLastSplit) * fCascadedTransitionRange;

		vCascadedData[nCascadedIdx].fCascadeSplit = fSplit;
		vCascadedData[nCascadedIdx].fTransitionRange = fTransitionRange;
		vCascadedData[nCascadedIdx].fNearZ = fNearZ + fRange * fLastSplit;
		vCascadedData[nCascadedIdx].fFarZ = fNearZ + fRange * fSplit;
		vCascadedData[nCascadedIdx].fOverFarZ = fNearZ + fRange * (fSplit + fTransitionRange);
		fLastSplit = fSplit;

		Matrix4 m4CameraInvProj = MRenderSystem::GetCameraInverseProjection(pViewport, pCameraComponent, pCameraSceneComponent, vCascadedData[nCascadedIdx].fNearZ, vCascadedData[nCascadedIdx].fOverFarZ);
		vCascadedData[nCascadedIdx].cCameraFrustum.UpdateFromCameraInvProj(m4CameraInvProj);
	}

	return vCascadedData;
}


MBoundsSphere ConvertSphere(const MBoundsSphere& sphere, Matrix4 mat)
{
	MBoundsSphere result = sphere;

	result.m_v3CenterPoint = mat * result.m_v3CenterPoint;

	return result;
}

MBoundsAABB ConvertAABB(const MBoundsAABB& aabb, Matrix4 mat)
{
	std::vector<Vector3> points(8);
	std::vector<Vector3> convertPoints(8);
	aabb.GetPoints(points);
	for (size_t i = 0; i < 8; ++i)
	{
		convertPoints[i] = mat * points[i];
	}

	MBoundsAABB result;
	result.SetPoints(convertPoints);
	return result;
}

std::array<MCascadedShadowRenderData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>
MShadowMapUtil::CalculateRenderData(MViewport* pViewport, MEntity* pCameraEntity,
	const std::array<MCascadedShadowSceneData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>& vCascadedData,
	const std::array<MBoundsAABB, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>& vCascadedPscBounds)
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

	Matrix4 matLight(pLightSceneComponent->GetTransform().GetRotation());
	Matrix4 matLightInv = matLight.Inverse();

	std::array<Matrix4, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vCascadeProjectionMatrix;
	std::array<float, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vCascadeFrustumWidth;
	std::array<float, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vPscBoundsInLightSpaceMinZ;
	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		float fLastSplitDist = nCascadedIdx ? vCascadedData[nCascadedIdx - 1].fCascadeSplit : 0.0f;
		float fCurrSplitDist = vCascadedData[nCascadedIdx].fCascadeSplit;

		float fCascadedNearZ = fNearZ + (fFarZ - fNearZ) * fLastSplitDist;
		float fCascadedFarZ = fNearZ + (fFarZ - fNearZ) * fCurrSplitDist;

		std::vector<Vector3> vCascadedFrustumPointsInWorldSpace(8);
		MRenderSystem::GetCameraFrustumPoints(pCameraEntity, pViewport->GetSize(), fCascadedNearZ, fCascadedFarZ, vCascadedFrustumPointsInWorldSpace);
		MBoundsSphere cCascadedFrustumSphereInWorldSpace;
		cCascadedFrustumSphereInWorldSpace.SetPoints(vCascadedFrustumPointsInWorldSpace);

		MBoundsSphere cascadedFrustumSphereInLightSpace = cCascadedFrustumSphereInWorldSpace;
		cascadedFrustumSphereInLightSpace.m_v3CenterPoint = matLightInv * cascadedFrustumSphereInLightSpace.m_v3CenterPoint;

		MBoundsAABB cascadedAABBBoundsInLightSpace;
		cascadedAABBBoundsInLightSpace.SetMinMax(
			cascadedFrustumSphereInLightSpace.m_v3CenterPoint - Vector3::Fill(cascadedFrustumSphereInLightSpace.m_fRadius),
			cascadedFrustumSphereInLightSpace.m_v3CenterPoint + Vector3::Fill(cascadedFrustumSphereInLightSpace.m_fRadius)
		);

		MBoundsAABB lightFrustumInLightSpace = cascadedAABBBoundsInLightSpace;

		if (true)
		{
			if (vCascadedPscBounds[nCascadedIdx].m_v3HalfLength.Length() > 1e-3)
			{
				MBoundsAABB pscInLightSpace = ConvertAABB(vCascadedPscBounds[nCascadedIdx], matLightInv);

				Vector3 min = lightFrustumInLightSpace.m_v3MinPoint;
				Vector3 max = lightFrustumInLightSpace.m_v3MaxPoint;
				for (size_t nIdx = 0; nIdx < 3; ++nIdx)
				{
					min.m[nIdx] = (std::max)(min.m[nIdx], pscInLightSpace.m_v3MinPoint.m[nIdx]);
					max.m[nIdx] = (std::min)(max.m[nIdx], pscInLightSpace.m_v3MaxPoint.m[nIdx]);

					min.m[nIdx] = (std::min)(min.m[nIdx], max.m[nIdx]);
				}
				min.z = pscInLightSpace.m_v3MinPoint.z;
				max.z = std::min(pscInLightSpace.m_v3MaxPoint.z, max.z);
				lightFrustumInLightSpace.SetMinMax(min, max);

				lightFrustumInLightSpace.m_v3HalfLength.x = lightFrustumInLightSpace.m_v3HalfLength.y = std::max(lightFrustumInLightSpace.m_v3HalfLength.x, lightFrustumInLightSpace.m_v3HalfLength.y);
				lightFrustumInLightSpace.SetMinMax(
					lightFrustumInLightSpace.m_v3CenterPoint - lightFrustumInLightSpace.m_v3HalfLength,
					lightFrustumInLightSpace.m_v3CenterPoint + lightFrustumInLightSpace.m_v3HalfLength
				);
			}
		}

		if (true)
		{	//https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps#moving-the-light-in-texel-sized-increments

			float fCascadedFrustumSphereLength = cCascadedFrustumSphereInWorldSpace.m_fRadius * 2.0f;
			Vector3 cameraFrustumLength = Vector3::Fill(fCascadedFrustumSphereLength);
			Vector2 worldUnitsPerTexel = {
				cameraFrustumLength.x / MRenderGlobal::SHADOW_TEXTURE_SIZE,
				cameraFrustumLength.y / MRenderGlobal::SHADOW_TEXTURE_SIZE,
			};

			/*
			Vector2 worldUnitsPerTexel = {
				16,
				16,
			};
			*/

			Vector3 min = lightFrustumInLightSpace.m_v3MinPoint;
			Vector3 max = lightFrustumInLightSpace.m_v3MaxPoint;
			Vector3 length = max - min;
			min.x = std::floor(min.x / worldUnitsPerTexel.x) * worldUnitsPerTexel.x;
			min.y = std::floor(min.y / worldUnitsPerTexel.y) * worldUnitsPerTexel.y;
			length.x = std::ceil(length.x / worldUnitsPerTexel.x) * worldUnitsPerTexel.x;
			length.y = std::ceil(length.y / worldUnitsPerTexel.y) * worldUnitsPerTexel.y;
			max.x = min.x + length.x;
			max.y = min.y + length.y;
			lightFrustumInLightSpace.SetMinMax(min, max);
		}

		//max bounds
		float fLightProjectionLeft = lightFrustumInLightSpace.m_v3MinPoint.x;
		float fLightProjectionRight = lightFrustumInLightSpace.m_v3MaxPoint.x;
		float fLightProjectionTop = lightFrustumInLightSpace.m_v3MaxPoint.y;
		float fLightProjectionBottom = lightFrustumInLightSpace.m_v3MinPoint.y;
		float fLightProjectionBack = lightFrustumInLightSpace.m_v3MinPoint.z;
		float fLightProjectionFront = lightFrustumInLightSpace.m_v3MaxPoint.z;

		vCascadeProjectionMatrix[nCascadedIdx] = MRenderSystem::MatrixOrthoOffCenterLH(
			fLightProjectionLeft,
			fLightProjectionRight,
			fLightProjectionTop,
			fLightProjectionBottom,
			fLightProjectionBack,
			fLightProjectionFront
		);

		vCascadeFrustumWidth[nCascadedIdx] = fLightProjectionRight - fLightProjectionLeft;
		vPscBoundsInLightSpaceMinZ[nCascadedIdx] = fLightProjectionBack;
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
		cRenderData[nCascadedIdx].fSplitRange.y = fNearZ + (fFarZ - fNearZ) * (vCascadedData[nCascadedIdx].fCascadeSplit + vCascadedData[nCascadedIdx].fTransitionRange);
		cRenderData[nCascadedIdx].fSplitRange.z = vCascadeFrustumWidth[nCascadedIdx];
		cRenderData[nCascadedIdx].fSplitRange.w = v4NearZPositionNDCSpace.z / v4NearZPositionNDCSpace.w;
		cRenderData[nCascadedIdx].m4DirLightInvProj = vCascadeProjectionMatrix[nCascadedIdx] * matLightInv;
	}

	return cRenderData;
}