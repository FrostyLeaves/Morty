#include "MModelComponent.h"

MORTY_CLASS_IMPLEMENT(MModelComponent, MComponent)

#include "MEntity.h"
#include "MEngine.h"
#include "MResourceSystem.h"
#include "MSkeletonResource.h"
#include "MSkeletalAnimationResource.h"

#include "MSceneComponent.h"
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
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (MResource* pResource = pResourceSystem->LoadResource(strSkeletonPath))
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
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (MResource* pAnimResource = pResourceSystem->LoadResource(strAnimationName))
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

	MEntity* pEntity = GetEntity();
	if (!pEntity)
		return aabb;

	Vector3 min_pos(FLT_MAX, FLT_MAX, FLT_MAX), max_pos(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	MSceneComponent::CallRecursivelyFunction(pEntity, [&](MEntity* pEntity){

		MBoundsAABB* p2 = &aabb;

		if (MRenderableMeshComponent* pMeshComponent = pEntity->GetComponent<MRenderableMeshComponent>())
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

void MModelComponent::WriteToStruct(MStruct& srt, MComponentRefTable& refTable)
{
	Super::WriteToStruct(srt, refTable);

	M_SERIALIZER_WRITE_BEGIN;
	M_SERIALIZER_WRITE_VALUE("SkeletonResource", GetSkeletonResourcePath)

	M_SERIALIZER_END;
}

void MModelComponent::ReadFromStruct(const MStruct& srt, MComponentRefTable& refTable)
{
	Super::ReadFromStruct(srt, refTable);

	M_SERIALIZER_READ_BEGIN;
	M_SERIALIZER_READ_VALUE("SkeletonResource", SetSkeletonResourcePath, String);

	M_SERIALIZER_END;
}
