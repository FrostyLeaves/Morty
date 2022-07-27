#include "MModelComponent.h"

MORTY_CLASS_IMPLEMENT(MModelComponent, MComponent)

#include "MEntity.h"
#include "MEngine.h"
#include "MResourceSystem.h"
#include "MSkeletonResource.h"
#include "MSkeletalAnimationResource.h"

#include "MSceneComponent.h"
#include "MRenderableMeshComponent.h"

#include "Flatbuffer/MModelComponent_generated.h"

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

void MModelComponent::SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeleton)
{
	if (m_pSkeleton)
	{
		m_pSkeleton = nullptr;
	}

	if (pSkeleton)
	{
		m_pSkeleton = std::make_shared<MSkeletonInstance>(pSkeleton);
	}


	m_SkeletonResource.SetResource(pSkeleton);
}

void MModelComponent::SetSkeletonResourcePath(const MString& strSkeletonPath)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strSkeletonPath))
	{
		SetSkeletonResource(MTypeClass::DynamicCast<MSkeletonResource>(pResource));
	}
}

MString MModelComponent::GetSkeletonResourcePath() const
{
	return m_SkeletonResource.GetResourcePath();
}

bool MModelComponent::PlayAnimation(const MString& strAnimationName)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pAnimResource = pResourceSystem->LoadResource(strAnimationName))
	{
		return PlayAnimation(pAnimResource);
	}

	return false;
}

bool MModelComponent::PlayAnimation(std::shared_ptr<MResource> pAnimation)
{
	RemoveAnimation();

	if (std::shared_ptr<MSkeletalAnimationResource> pAnimRes = MTypeClass::DynamicCast<MSkeletalAnimationResource>(pAnimation))
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

flatbuffers::Offset<void> MModelComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
	auto&& fb_ske_resource = fbb.CreateString(GetSkeletonResourcePath());
	auto&& fb_super = Super::Serialize(fbb).o;

	mfbs::MModelComponentBuilder builder(fbb);

	builder.add_skeleton_resource_path(fb_ske_resource);
	builder.add_super(fb_super);

	return builder.Finish().Union();
}

void MModelComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
	const mfbs::MModelComponent* fbcomponent = mfbs::GetMModelComponent(fbb.GetCurrentBufferPointer());
	Deserialize(fbcomponent);
}

void MModelComponent::Deserialize(const void* pBufferPointer)
{
	const mfbs::MModelComponent* pComponent = reinterpret_cast<const mfbs::MModelComponent*>(pBufferPointer);

	Super::Deserialize(pComponent->super());

	SetSkeletonResourcePath(pComponent->skeleton_resource_path()->c_str());
}
