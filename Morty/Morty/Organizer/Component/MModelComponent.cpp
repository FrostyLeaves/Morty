#include "MModelComponent.h"

M_OBJECT_IMPLEMENT(MModelComponent, MComponent)

#include "MNode.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "MSkeletonResource.h"
#include "MSkeletalAnimationResource.h"

#include "MRenderableMeshComponent.h"

MModelComponent::MModelComponent()
	: MComponent()
	, m_SkeletonResource()
	, m_pSkeleton(nullptr)
	, m_pCurrentAnimationController(nullptr)
	, m_bBoundingBoxVisiable(false)
{

}

MModelComponent::~MModelComponent()
{

}

void MModelComponent::SetSkeletonResource(MSkeleton* pSkeleton)
{
	if (m_pSkeleton)
	{
		delete m_pSkeleton;
		m_pSkeleton = nullptr;
	}

	if (pSkeleton)
	{
		m_pSkeleton = new MSkeletonInstance(pSkeleton);
	}


	m_SkeletonResource.SetResource(pSkeleton);
}

void MModelComponent::SetSkeletonResourcePath(const MString& strSkeletonPath)
{
	if (MResource* pResource = GetEngine()->GetResourceManager()->LoadResource(strSkeletonPath))
	{
		SetSkeletonResource(pResource->DynamicCast<MSkeletonResource>());
	}
}

MString MModelComponent::GetSkeletonResourcePath() const
{
	return m_SkeletonResource.GetResourcePath();
}

bool MModelComponent::PlayAnimation(const MString& strAnimationName)
{
	if (MResource* pAnimResource = GetEngine()->GetResourceManager()->LoadResource(strAnimationName))
	{
		return PlayAnimation(pAnimResource);
	}

	return false;
}

bool MModelComponent::PlayAnimation(MResource* pAnimation)
{
	RemoveAnimation();

	if (MSkeletalAnimationResource* pAnimRes = pAnimation->DynamicCast<MSkeletalAnimationResource>())
	{
		MSkeletalAnimController* pController = new MSkeletalAnimController();
		if (pController->Initialize(m_pSkeleton, pAnimRes))
		{
			m_pCurrentAnimationController = pController;

			return true;
		}
	}

	return false;
}

void MModelComponent::RemoveAnimation()
{
	if (m_pCurrentAnimationController)
	{
		delete m_pCurrentAnimationController;
		m_pCurrentAnimationController = nullptr;
	}
}

MSkeletalAnimController* MModelComponent::GetSkeletalAnimationController()
{
	return m_pCurrentAnimationController;
}

MBoundsAABB MModelComponent::GetBoundsAABB()
{
	MBoundsAABB aabb;

	MNode* pNode = GetOwnerNode();
	if (!pNode)
		return aabb;

	Vector3 min_pos(FLT_MAX, FLT_MAX, FLT_MAX), max_pos(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	pNode->CallRecursivelyFunction([&](MNode* pChild){

		MBoundsAABB* p2 = &aabb;

		if (MRenderableMeshComponent* pMeshComponent = pChild->GetComponent<MRenderableMeshComponent>())
		{
			if (MBoundsAABB* pBounds = pMeshComponent->GetBoundsAABB())
			{
				pBounds->UnionMinMax(min_pos, max_pos);
			}
		}

	});

	aabb.SetMinMax(min_pos, max_pos);
	return aabb;
}

void MModelComponent::Tick(const float& fDelta)
{
	Super::Tick(fDelta);

	bool bVisible = false;

	if (MNode* pOwnerNode = GetOwnerNode())
	{
		bVisible = pOwnerNode->GetVisibleRecursively();
	}

	if (m_pCurrentAnimationController && m_pCurrentAnimationController->GetState() == MIAnimController::EPlay)
	{
		m_pCurrentAnimationController->Update(fDelta, bVisible);
	}
}

void MModelComponent::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);

	M_SERIALIZER_WRITE_BEGIN;
	M_SERIALIZER_WRITE_VALUE("SkeletonResource", GetSkeletonResourcePath)

	M_SERIALIZER_END;
}

void MModelComponent::ReadFromStruct(const MStruct& srt)
{
	Super::ReadFromStruct(srt);

	M_SERIALIZER_READ_BEGIN;
	M_SERIALIZER_READ_VALUE("SkeletonResource", SetSkeletonResourcePath, String);

	M_SERIALIZER_END;
}
