/**
 * @File         MSkeletalAnimation
 * 
 * @Created      2019-12-09 22:38:06
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSKELETALANIMATION_H_
#define _M_MSKELETALANIMATION_H_
#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Model/MSkeleton.h"
#include "Resource/MResource.h"
#include "Math/Quaternion.h"
#include "Utility/MTransform.h"
#include "MIAnimation.h"
#include "Utility/MSerializer.h"

#include <vector>

#include "Flatbuffer/MSkeletalAnimation_generated.h"

class MORTY_API MSkeletalAnimNode
{
public:

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(flatbuffers::FlatBufferBuilder& fbb);
	void Deserialize(const void* pBufferPointer);

public:

	std::vector<mfbs::MSkeletalPositionKey> m_vPositionTrack;
	std::vector<mfbs::MSkeletalRotationKey> m_vRotationTrack;
	std::vector<mfbs::MSkeletalScaleKey> m_vScaleTrack;
};

class MORTY_API MSkeletonAnimMap
{
public:
	std::vector<uint32_t> m_vSkelToAnim;
	std::vector<uint32_t> m_vAnimToSkel;
};

class MORTY_API MSkeletalAnimation : public MIAnimation, public MResource
{
public:
	MORTY_CLASS(MSkeletalAnimation);
    MSkeletalAnimation();
    virtual ~MSkeletalAnimation();

public:

	std::shared_ptr<MSkeleton> GetSkeletonTemplate();

	std::shared_ptr<MResource> GetSkeletonResource() { return m_Skeleton.GetResource(); }

	uint32_t GetIndex() { return m_unIndex; }
	MString GetName() { return m_strName; }
	float GetTicksDuration() { return m_fTicksDuration; }
	float GetTicksPerSecond() { return m_fTicksPerSecond; }

	void Update(const float& fTime, std::shared_ptr<MSkeletonInstance> pSkeletonIns, const MSkeletonAnimMap& skelAnimMap);

public:

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(const void* pBufferPointer);

public:
	
	virtual void OnDelete() override;

public:

	virtual bool Load(const MString& strResourcePath) override;
	virtual bool SaveTo(const MString& strResourcePath) override;

	static MString GetResourceTypeName() { return "Animation"; };
	static std::vector<MString> GetSuffixList() { return { "anim" }; };

protected:

	bool FindTransform(const float& fTime, const MSkeletalAnimNode& animNode, MTransform& trans);

private:
	friend class MModelConverter;
	std::vector<MSkeletalAnimNode> m_vSkeletalAnimNodes;
	MResourceRef m_Skeleton;

	uint32_t m_unIndex;
	MString m_strName;
	float m_fTicksDuration;
	float m_fTicksPerSecond;
};

class MSkeletalAnimationResource;
class MORTY_API MSkeletalAnimController : public MIAnimController
{
public:
	MSkeletalAnimController();
	virtual ~MSkeletalAnimController();

	bool Initialize(std::shared_ptr<MSkeletonInstance> pSkeletonIns, std::shared_ptr<MSkeletalAnimation> pAnimation);

public:

	virtual void Play() override;
	virtual void Pause() override;
	virtual void Stop() override;
	virtual void SetLoop(const bool& bLoop) override;
	virtual bool GetLoop() override { return m_bLoop; }
	virtual void Update(const float& fDelta, const bool& bAnimStep = true) override;

	//range 0.0f ~ 100.0f
	virtual void SetPercent(const float& fPercent);
	virtual float GetPercent();

	virtual MEAnimControllerState GetState() override { return m_eState; }

	std::shared_ptr<MSkeletalAnimation> GetAnimation() { return m_pAnimation; }

protected:

	void BindMapping();

private:
	std::shared_ptr<MSkeletonInstance> m_pSkeletonIns;
	std::shared_ptr<MSkeletalAnimation> m_pAnimation;
	MResourceRef m_AnimResource;

	bool m_bInitialized;

	float m_fTicks;
	MEAnimControllerState m_eState;
	bool m_bLoop;

	MSkeletonAnimMap m_SkeletonAnimMap;
};

#endif
