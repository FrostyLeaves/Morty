#include "Component/MModelComponent.h"

#include "MRenderNotify.h"

#include "Engine/MEngine.h"
#include "Model/MSkeletonInstance.h"
#include "Resource/MSkeletalAnimationResource.h"
#include "Resource/MSkeletonResource.h"
#include "Scene/MEntity.h"

#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include "Flatbuffer/MModelComponent_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MModelComponent, MComponent)

MModelComponent::MModelComponent()
    : MComponent()
    , m_SkeletonResource()
    , m_skeleton(nullptr)
    , m_currentAnimationController(nullptr)
    , m_boundingBoxVisiable(false)
{}

MModelComponent::~MModelComponent() {}

void MModelComponent::Release()
{
    if (m_skeleton)
    {
        m_skeleton->DeleteLater();
        m_skeleton = nullptr;
    }
}

void MModelComponent::SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeletonRsource)
{
    if (!m_skeleton)
    {
        auto pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
        m_skeleton         = pObjectSystem->CreateObject<MSkeletonInstance>();
    }

    if (m_skeleton) { m_skeleton->SetSkeletonResource(pSkeletonRsource); }

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

MString MModelComponent::GetSkeletonResourcePath() const { return m_SkeletonResource.GetResourcePath(); }

bool    MModelComponent::PlayAnimation(const MString& strAnimationName)
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

    if (std::shared_ptr<MSkeletalAnimationResource> pAnimRes =
                MTypeClass::DynamicCast<MSkeletalAnimationResource>(pAnimation))
    {
        MSkeletalAnimController* pController = new MSkeletalAnimController();
        if (pController->Initialize(m_skeleton, pAnimRes))
        {
            m_currentAnimationController = pController;

            return true;
        }
    }

    return false;
}

void MModelComponent::RemoveAnimation()
{
    if (m_currentAnimationController)
    {
        delete m_currentAnimationController;
        m_currentAnimationController = nullptr;
    }
}

MSkeletalAnimController*  MModelComponent::GetSkeletalAnimationController() { return m_currentAnimationController; }

flatbuffers::Offset<void> MModelComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    auto                        fb_ske_resource = fbb.CreateString(GetSkeletonResourcePath());
    auto                        fb_super        = Super::Serialize(fbb).o;

    fbs::MModelComponentBuilder builder(fbb);

    builder.add_skeleton_resource_path(fb_ske_resource);
    builder.add_super(fb_super);

    return builder.Finish().Union();
}

void MModelComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
    const fbs::MModelComponent* fbcomponent = fbs::GetMModelComponent(fbb.GetCurrentBufferPointer());
    Deserialize(fbcomponent);
}

void MModelComponent::Deserialize(const void* pBufferPointer)
{
    const fbs::MModelComponent* pComponent = reinterpret_cast<const fbs::MModelComponent*>(pBufferPointer);

    Super::Deserialize(pComponent->super());

    SetSkeletonResourcePath(pComponent->skeleton_resource_path()->c_str());
}
