#include "Component/MModelComponent.h"

#include "MRenderNotify.h"

MORTY_CLASS_IMPLEMENT(MModelComponent, MComponent)

#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Resource/MSkeletonResource.h"
#include "Resource/MSkeletalAnimationResource.h"
#include "Model/MSkeletonInstance.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

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

void MModelComponent::Release()
{
	if (m_pSkeleton)
	{
		m_pSkeleton->DeleteLater();
		m_pSkeleton = nullptr;
	}
}

void MModelComponent::SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeletonRsource)
{
	if (!m_pSkeleton)
	{
		auto pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
		m_pSkeleton = pObjectSystem->CreateObject<MSkeletonInstance>();
	}

	if (m_pSkeleton)
	{
		m_pSkeleton->SetSkeletonResource(pSkeletonRsource);
	}

	m_SkeletonResource = pSkeletonRsource;

	SendComponentNotify(MRenderNotify::NOTIFY_ANIMATION_POSE_CHANGED);
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

flatbuffers::Offset<void> MModelComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
	auto fb_ske_resource = fbb.CreateString(GetSkeletonResourcePath());
	auto fb_super = Super::Serialize(fbb).o;

	morty::MModelComponentBuilder builder(fbb);

	builder.add_skeleton_resource_path(fb_ske_resource);
	builder.add_super(fb_super);

	return builder.Finish().Union();
}

void MModelComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
	const morty::MModelComponent* fbcomponent =morty::GetMModelComponent(fbb.GetCurrentBufferPointer());
	Deserialize(fbcomponent);
}

void MModelComponent::Deserialize(const void* pBufferPointer)
{
	const morty::MModelComponent* pComponent = reinterpret_cast<const morty::MModelComponent*>(pBufferPointer);

	Super::Deserialize(pComponent->super());

	SetSkeletonResourcePath(pComponent->skeleton_resource_path()->c_str());
}
