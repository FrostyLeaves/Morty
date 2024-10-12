#include "Model/MSkeletalAnimation.h"
#include "Engine/MEngine.h"
#include "Flatbuffer/MSkeletalAnimation_generated.h"
#include "Math/MMath.h"
#include "Model/MSkeletonInstance.h"
#include "Resource/MSkeletalAnimationResource.h"
#include "Resource/MSkeletonResource.h"
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"

using namespace morty;

MSkeletalAnimation::MSkeletalAnimation()
    : MIAnimation()
    , m_unIndex(0)
    , m_strName()
    , m_ticksDuration(0.0f)
    , m_ticksPerSecond(25.0f)
{}

MSkeletalAnimation::~MSkeletalAnimation() {}

void MSkeletalAnimation::SamplePose(
        MSkeletonPose&          outputPose,
        const float&            fTime,
        MSkeletonInstance*      pSkeletonIns,
        const MSkeletonAnimMap& skelAnimMap
) const
{
    const std::vector<MBone>& bones = pSkeletonIns->GetAllBones();

    const uint32_t            nBonesSize = static_cast<uint32_t>(bones.size());
    outputPose.vBoneMatrix.resize(nBonesSize);

    for (uint32_t i = 0; i < nBonesSize; ++i)
    {
        const MBone& bone = bones[i];

        const int    nAnimNodeIndex = skelAnimMap.m_skelToAnim[bone.unIndex];
        if (MGlobal::M_INVALID_INDEX == nAnimNodeIndex) { continue; }

        const MSkeletalAnimNode& animNode       = m_skeletalAnimNodes[nAnimNodeIndex];
        Matrix4                  matParentTrans = bone.unParentIndex == MGlobal::M_INVALID_UINDEX
                                                          ? Matrix4::IdentityMatrix
                                                          : outputPose.vBoneMatrix[bone.unParentIndex];

        MTransform               trans;
        if (FindTransform(fTime, animNode, trans))
        {
            //Use animation transform
            outputPose.vBoneMatrix[i] = matParentTrans * trans.GetMatrix();
        }
        else
        {
            //Use default transform
            outputPose.vBoneMatrix[i] = matParentTrans * pSkeletonIns->GetBoneTemplateByIndex(i)->m_matTransform;
        }
    }

    for (size_t i = 0; i < nBonesSize; ++i)
    {
        const MBone& bone         = bones[i];
        outputPose.vBoneMatrix[i] = outputPose.vBoneMatrix[i] * bone.m_matOffsetMatrix;
    }
}

void                      MSkeletalAnimation::SetSkeletonTemplate(MSkeleton* pSkeleton) { m_skeleton = pSkeleton; }

flatbuffers::Offset<void> MSkeletalAnimNode::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    auto                          fbPositionTrack = fbb.CreateVectorOfStructs(m_positionTrack);
    auto                          fbRotationTrack = fbb.CreateVectorOfStructs(m_rotationTrack);
    auto                          fbScaleTrack    = fbb.CreateVectorOfStructs(m_scaleTrack);

    fbs::MSkeletalAnimNodeBuilder builder(fbb);

    builder.add_position_track(fbPositionTrack);
    builder.add_rotation_track(fbRotationTrack);
    builder.add_scale_track(fbScaleTrack);

    return builder.Finish().Union();
}

void MSkeletalAnimNode::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
    const fbs::MSkeletalAnimNode* fbcomponent = fbs::GetMSkeletalAnimNode(fbb.GetCurrentBufferPointer());
    Deserialize(fbcomponent);
}

void MSkeletalAnimNode::Deserialize(const void* pBufferPointer)
{
    const fbs::MSkeletalAnimNode* fbData = reinterpret_cast<const fbs::MSkeletalAnimNode*>(pBufferPointer);

    m_positionTrack.resize(fbData->position_track()->size());
    for (size_t nIdx = 0; nIdx < fbData->position_track()->size(); ++nIdx)
    {
        m_positionTrack[nIdx] = *fbData->position_track()->Get(static_cast<uint32_t>(nIdx));
    }

    m_rotationTrack.resize(fbData->rotation_track()->size());
    for (size_t nIdx = 0; nIdx < fbData->rotation_track()->size(); ++nIdx)
    {
        m_rotationTrack[nIdx] = *fbData->rotation_track()->Get(static_cast<uint32_t>(nIdx));
    }

    m_scaleTrack.resize(fbData->scale_track()->size());
    for (size_t nIdx = 0; nIdx < fbData->scale_track()->size(); ++nIdx)
    {
        m_scaleTrack[nIdx] = *fbData->scale_track()->Get(static_cast<uint32_t>(nIdx));
    }
}

flatbuffers::Offset<void> MSkeletalAnimation::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    std::vector<flatbuffers::Offset<fbs::MSkeletalAnimNode>> fbAnimNode;
    for (const MSkeletalAnimNode& node: m_skeletalAnimNodes) { fbAnimNode.push_back(node.Serialize(fbb).o); }

    auto                           fbName           = fbb.CreateString(m_strName);
    auto                           fbAnimNodeOffset = fbb.CreateVector(fbAnimNode);


    fbs::MSkeletalAnimationBuilder builder(fbb);

    builder.add_name(fbName);
    builder.add_duration(m_ticksDuration);
    builder.add_speed(m_ticksPerSecond);
    builder.add_animation_node(fbAnimNodeOffset);

    return builder.Finish().Union();
}

void MSkeletalAnimation::Deserialize(const void* pBufferPointer)
{
    const fbs::MSkeletalAnimation* fbData = reinterpret_cast<const fbs::MSkeletalAnimation*>(pBufferPointer);

    m_strName        = fbData->name()->str();
    m_ticksDuration  = fbData->duration();
    m_ticksPerSecond = fbData->speed();

    if (fbData->animation_node())
    {
        m_skeletalAnimNodes.resize(fbData->animation_node()->size());
        for (size_t nIdx = 0; nIdx < fbData->animation_node()->size(); ++nIdx)
        {
            m_skeletalAnimNodes[nIdx].Deserialize(fbData->animation_node()->Get(static_cast<uint32_t>(nIdx)));
        }
    }
    else { m_skeletalAnimNodes.clear(); }
}

bool MSkeletalAnimation::FindTransform(const float& fTime, const MSkeletalAnimNode& animNode, MTransform& trans) const
{
    if (animNode.m_positionTrack.size() + animNode.m_rotationTrack.size() + animNode.m_scaleTrack.size() == 0)
        return false;

    Vector3    v3Position, v3Scale;
    Quaternion quatRotation;

    for (size_t nIdx = 0; nIdx < animNode.m_positionTrack.size(); ++nIdx)
    {
        size_t      i = animNode.m_positionTrack.size() - nIdx - 1;

        const auto& curkey = animNode.m_positionTrack[i];
        if (fTime >= curkey.time())
        {
            if (i == animNode.m_positionTrack.size() - 1) { v3Position = curkey.value(); }
            else
            {
                const auto& nextKey = animNode.m_positionTrack[i + 1];
                if (nextKey.time() - curkey.time() > 1e-6f)
                {
                    Vector3 current, next;
                    current.Deserialize(&curkey.value());
                    next.Deserialize(&nextKey.value());
                    v3Position = MMath::Lerp(current, next, (fTime - curkey.time()) / (nextKey.time() - curkey.time()));
                }
                else { v3Position.Deserialize(&nextKey.value()); }
            }

            break;
        }
    }

    for (size_t nIdx = 0; nIdx < animNode.m_rotationTrack.size(); ++nIdx)
    {
        size_t      i = animNode.m_rotationTrack.size() - nIdx - 1;

        const auto& curkey = animNode.m_rotationTrack[i];
        if (fTime >= curkey.time())
        {
            if (i == animNode.m_rotationTrack.size() - 1) { quatRotation = curkey.value(); }
            else
            {
                const auto& nextKey = animNode.m_rotationTrack[i + 1];
                if (nextKey.time() - curkey.time() > 1e-6f)
                {
                    Quaternion current, next;
                    current.Deserialize(&curkey.value());
                    next.Deserialize(&nextKey.value());
                    quatRotation = Quaternion::Slerp(
                            current,
                            next,
                            (fTime - curkey.time()) / (nextKey.time() - curkey.time())
                    );
                }
                else { quatRotation.Deserialize(&nextKey.value()); }
            }

            break;
        }
    }

    for (size_t nIdx = 0; nIdx < animNode.m_scaleTrack.size(); ++nIdx)
    {
        size_t      i = animNode.m_scaleTrack.size() - nIdx - 1;

        const auto& curkey = animNode.m_scaleTrack[i];
        if (fTime >= curkey.time())
        {
            if (i == animNode.m_scaleTrack.size() - 1) { v3Scale = curkey.value(); }
            else
            {
                const auto& nextKey = animNode.m_scaleTrack[i + 1];
                if (nextKey.time() - curkey.time() > 1e-6f)
                {
                    Vector3 current, next;
                    current.Deserialize(&curkey.value());
                    next.Deserialize(&nextKey.value());
                    v3Scale = MMath::Lerp(current, next, (fTime - curkey.time()) / (nextKey.time() - curkey.time()));
                }
                else { v3Scale.Deserialize(&nextKey.value()); }
            }

            break;
        }
    }

    trans.SetPosition(v3Position);
    trans.SetRotation(quatRotation);
    trans.SetScale(v3Scale);

    return true;
}

MSkeletalAnimController::MSkeletalAnimController()
    : m_skeletonIns(nullptr)
    , m_animation(nullptr)
    , m_initialized(false)
    , m_ticks(0.0f)
    , m_state(MIAnimController::MEAnimControllerState::EStop)
    , m_loop(false)
{}

MSkeletalAnimController::~MSkeletalAnimController() {}

bool MSkeletalAnimController::Initialize(
        MSkeletonInstance*                          pSkeletonIns,
        std::shared_ptr<MSkeletalAnimationResource> pAnimationResource
)
{
    if (!pAnimationResource) return false;

    if (m_initialized) return false;

    m_skeletonIns  = pSkeletonIns;
    m_AnimResource = pAnimationResource;
    m_animation    = pAnimationResource->GetAnimation();

    BindMapping();

    m_initialized = true;

    return true;
}

void MSkeletalAnimController::Play()
{
    if (!m_initialized) return;

    m_state = MEAnimControllerState::EPlay;
}

void MSkeletalAnimController::Pause()
{
    if (!m_initialized) return;

    m_state = MEAnimControllerState::EPause;
}

void MSkeletalAnimController::Stop()
{
    if (!m_initialized) return;

    m_state = MEAnimControllerState::EStop;
}

void MSkeletalAnimController::SetLoop(const bool& bLoop) { m_loop = bLoop; }

void MSkeletalAnimController::NextStep(const float& fDelta, const bool& bAnimStep)
{
    if (!m_initialized) { return; }

    m_ticks += m_animation->GetTicksPerSecond() * fDelta;
    if (m_ticks >= m_animation->GetTicksDuration())
    {
        if (m_loop) { m_ticks = fmodf(m_ticks, m_animation->GetTicksDuration()); }
        else
        {
            m_ticks = m_animation->GetTicksDuration();
            this->Stop();
        }
    }

    if (bAnimStep)
    {
        m_animation->SamplePose(m_skeletonIns->GetCurrentPose(), m_ticks, m_skeletonIns, m_SkeletonAnimMap);
    }
}

void MSkeletalAnimController::SetPercent(const float& fPercent)
{
    if (nullptr == m_animation) return;

    if (fPercent <= 0.0f) m_ticks = 0.0f;
    else if (fPercent >= 100.0f)
        m_ticks = m_animation->GetTicksDuration();
    else
        m_ticks = m_animation->GetTicksDuration() * fPercent / 100.0f;
}

float MSkeletalAnimController::GetPercent()
{
    if (nullptr == m_animation) return 0.0f;

    return m_ticks / m_animation->GetTicksDuration() * 100.0f;
}

std::shared_ptr<MSkeletalAnimationResource> MSkeletalAnimController::GetAnimationResource() const
{
    return m_AnimResource.GetResource<MSkeletalAnimationResource>();
}

void MSkeletalAnimController::BindMapping()
{
    if (!m_skeletonIns) { return; }

    m_SkeletonAnimMap.m_animToSkel.resize(
            m_animation->GetSkeletonTemplate()->GetAllBones().size(),
            MGlobal::M_INVALID_INDEX
    );
    m_SkeletonAnimMap.m_skelToAnim.resize(m_skeletonIns->GetAllBones().size(), MGlobal::M_INVALID_INDEX);

    if (m_skeletonIns->GetSkeletonTemplate() == m_animation->GetSkeletonTemplate())
    {
        for (uint32_t i = 0; i < m_SkeletonAnimMap.m_animToSkel.size(); ++i)
        {
            m_SkeletonAnimMap.m_skelToAnim[i] = i;
            m_SkeletonAnimMap.m_animToSkel[i] = i;
        }
    }
    else
    {
        for (MBone& skelBone: m_skeletonIns->GetAllBones())
        {
            const MBone* pAnimBone = m_animation->GetSkeletonTemplate()->FindBoneByName(skelBone.strName);

            m_SkeletonAnimMap.m_skelToAnim[skelBone.unIndex]   = pAnimBone->unIndex;
            m_SkeletonAnimMap.m_animToSkel[pAnimBone->unIndex] = skelBone.unIndex;
        }
    }
}
