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

class MSkeletonInstance;
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

class MORTY_API MSkeletalAnimation : public MIAnimation
{
public:
    MSkeletalAnimation();
    virtual ~MSkeletalAnimation();

public:


	uint32_t GetIndex() const { return m_unIndex; }
	MString GetName() const { return m_strName; }
	float GetTicksDuration() const { return m_fTicksDuration; }
	float GetTicksPerSecond() const { return m_fTicksPerSecond; }

	void Update(const float& fTime, MSkeletonInstance* pSkeletonIns, const MSkeletonAnimMap& skelAnimMap) const;

	void SetSkeletonTemplate(MSkeleton* pSkeleton);
	MSkeleton* GetSkeletonTemplate() const { return m_pSkeleton; }

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(const void* pBufferPointer);

	static MString GetResourceTypeName() { return "Animation"; };
	static std::vector<MString> GetSuffixList() { return { "anim" }; };

protected:

	bool FindTransform(const float& fTime, const MSkeletalAnimNode& animNode, MTransform& trans) const;

private:
	friend class MModelConverter;
	std::vector<MSkeletalAnimNode> m_vSkeletalAnimNodes;
	MSkeleton* m_pSkeleton = nullptr;

	uint32_t m_unIndex;
	MString m_strName;
	float m_fTicksDuration;
	float m_fTicksPerSecond;
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

	const MSkeletalAnimation* GetAnimation() const { return m_pAnimation; }
	std::shared_ptr<MSkeletalAnimationResource> GetAnimationResource() const;

protected:

	void BindMapping();

private:
	MSkeletonInstance* m_pSkeletonIns = nullptr;
	const MSkeletalAnimation* m_pAnimation = nullptr;
	MResourceRef m_AnimResource;

	bool m_bInitialized;

	float m_fTicks;
	MEAnimControllerState m_eState;
	bool m_bLoop;

	MSkeletonAnimMap m_SkeletonAnimMap;
};

#endif
