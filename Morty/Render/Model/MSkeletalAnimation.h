/**
 * @File         MSkeletalAnimation
 * 
 * @Created      2019-12-09 22:38:06
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MIAnimation.h"
#include "Math/Quaternion.h"
#include "Math/Vector.h"
#include "Model/MSkeleton.h"
#include "Resource/MResource.h"
#include "Utility/MTransform.h"

#include "Flatbuffer/MSkeletalAnimation_generated.h"

namespace morty
{

class MSkeletonInstance;
class MORTY_API MSkeletalAnimNode
{
public:
    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    void                      Deserialize(flatbuffers::FlatBufferBuilder& fbb);

    void                      Deserialize(const void* pBufferPointer);

public:
    std::vector<fbs::MSkeletalPositionKey> m_positionTrack;
    std::vector<fbs::MSkeletalRotationKey> m_rotationTrack;
    std::vector<fbs::MSkeletalScaleKey>    m_scaleTrack;
};

class MORTY_API MSkeletonAnimMap
{
public:
    std::vector<uint32_t> m_skelToAnim;
    std::vector<uint32_t> m_animToSkel;
};

class MORTY_API MSkeletalAnimation : public MIAnimation
{
public:
    MSkeletalAnimation();

    virtual ~MSkeletalAnimation();

public:
    uint32_t GetIndex() const { return m_unIndex; }

    MString  GetName() const { return m_strName; }

    float    GetTicksDuration() const { return m_ticksDuration; }

    float    GetTicksPerSecond() const { return m_ticksPerSecond; }

    void     SamplePose(
                MSkeletonPose&          outputPose,
                const float&            fTime,
                MSkeletonInstance*      pSkeletonIns,
                const MSkeletonAnimMap& skelAnimMap
        ) const;

    void                        SetSkeletonTemplate(MSkeleton* pSkeleton);

    MSkeleton*                  GetSkeletonTemplate() const { return m_skeleton; }

    flatbuffers::Offset<void>   Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    void                        Deserialize(const void* pBufferPointer);

    static MString              GetResourceTypeName() { return "Animation"; };

    static std::vector<MString> GetSuffixList() { return {"anim"}; };

protected:
    bool FindTransform(const float& fTime, const MSkeletalAnimNode& animNode, MTransform& trans) const;

private:
    friend class MModelConverter;

    std::vector<MSkeletalAnimNode> m_skeletalAnimNodes;
    MSkeleton*                     m_skeleton = nullptr;

    uint32_t                       m_unIndex;
    MString                        m_strName;
    float                          m_ticksDuration;
    float                          m_ticksPerSecond;
};

class MSkeletonInstance;
class MSkeletalAnimationResource;
class MORTY_API MSkeletalAnimController : public MIAnimController
{
public:
    MSkeletalAnimController();

    virtual ~MSkeletalAnimController();

    bool Initialize(MSkeletonInstance* pSkeletonIns, std::shared_ptr<MSkeletalAnimationResource> pAnimationResource);

public:
    virtual void                                Play() override;

    virtual void                                Pause() override;

    virtual void                                Stop() override;

    virtual void                                SetLoop(const bool& bLoop) override;

    virtual bool                                GetLoop() override { return m_loop; }

    virtual void                                NextStep(const float& fDelta, const bool& bAnimStep = true) override;

    //range 0.0f ~ 100.0f
    virtual void                                SetPercent(const float& fPercent);

    virtual float                               GetPercent();

    virtual MEAnimControllerState               GetState() override { return m_state; }

    const MSkeletalAnimation*                   GetAnimation() const { return m_animation; }

    std::shared_ptr<MSkeletalAnimationResource> GetAnimationResource() const;

protected:
    void BindMapping();

private:
    MSkeletonInstance*        m_skeletonIns = nullptr;
    const MSkeletalAnimation* m_animation   = nullptr;
    MResourceRef              m_AnimResource;

    bool                      m_initialized;

    float                     m_ticks;
    MEAnimControllerState     m_state;
    bool                      m_loop;

    MSkeletonAnimMap          m_SkeletonAnimMap;
};

}// namespace morty