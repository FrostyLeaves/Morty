/**
 * @File         MSkeletalAnimation
 * 
 * @Created      2019-12-09 22:38:06
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSKELETALANIMATION_H_
#define _M_MSKELETALANIMATION_H_
#include "MGlobal.h"
#include "Vector.h"
#include "MSkeleton.h"
#include "MResource.h"
#include "Quaternion.h"
#include "MTransform.h"
#include "MIAnimation.h"
#include "MSerializer.h"

#include <vector>

class MORTY_API MSkeletalAnimNode
{
public:
	template <typename T>
	class MAnimNodeKey
	{
	public:
		MAnimNodeKey() :mTime(0.0f), mValue() {}
	public:
		float mTime;
		T mValue;
	};

	MSkeletalAnimNode();
	~MSkeletalAnimNode();

public:

	uint32_t m_unPositionKeysNum;
	uint32_t m_unRotationKeysNum;
	uint32_t m_unScalingKeysNum;
	MAnimNodeKey<Vector3>* m_vPositionKeys;
	MAnimNodeKey<Quaternion>* m_vRotationKeys;
	MAnimNodeKey<Vector3>* m_vScalingKeys;

	void WriteToStruct(MStruct& srt);
	void ReadFromStruct(const MStruct& srt);
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

	MSkeleton* GetSkeletonTemplate();

	MResource* GetSkeletonResource() { return m_Skeleton.GetResource(); }

	uint32_t GetIndex() { return m_unIndex; }
	MString GetName() { return m_strName; }
	float GetTicksDuration() { return m_fTicksDuration; }
	float GetTicksPerSecond() { return m_fTicksPerSecond; }

	void Update(const float& fTime, MSkeletonInstance* pSkeletonIns, const MSkeletonAnimMap& skelAnimMap);

	void WriteToStruct(MStruct& srt);
	void ReadFromStruct(const MStruct& srt);

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
	MResourceKeeper m_Skeleton;

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

	bool Initialize(MSkeletonInstance* pSkeletonIns, MSkeletalAnimation* pAnimation);

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

	MSkeletalAnimation* GetAnimation() { return m_pAnimation; }

protected:

	void BindMapping();

private:
	MSkeletonInstance* m_pSkeletonIns;
	MSkeletalAnimation* m_pAnimation;
	MResourceKeeper m_AnimResource;

	bool m_bInitialized;

	float m_fTicks;
	MEAnimControllerState m_eState;
	bool m_bLoop;

	MSkeletonAnimMap m_SkeletonAnimMap;
};

#endif
