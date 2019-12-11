/**
 * @File         MSkeletalAnimation
 * 
 * @Created      2019-12-09 22:38:06
 *
 * @Author       Morty
**/

#ifndef _M_MSKELETALANIMATION_H_
#define _M_MSKELETALANIMATION_H_
#include "MGlobal.h"
#include "MIAnimation.h"
#include "MSkeleton.h"
#include "Vector.h"
#include "Quaternion.h"
#include "MTransform.h"

#include <vector>

class MORTY_CLASS MSkeletalAnimNode
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

public:

	unsigned int m_unPositionKeysNum;
	unsigned int m_unRotationKeysNum;
	unsigned int m_unScalingKeysNum;
	MAnimNodeKey<Vector3>* m_vPositionKeys;
	MAnimNodeKey<Quaternion>* m_vRotationKeys;
	MAnimNodeKey<Vector3>* m_vScalingKeys;

};

class MORTY_CLASS MSkeletalAnimation : public MIAnimation
{
public:
    MSkeletalAnimation();
    virtual ~MSkeletalAnimation();

public:

	MSkeleton* GetSkeletonTemplate() { return m_pSkeletonTemplate; }

	MString GetName() { return m_strName; }
	float GetTicksDuration() { return m_fTicksDuration; }
	float GetTicksPerSecond() { return m_fTicksPerSecond; }

	void Update(const float& fTime, MSkeletonInstance* pSkeletonIns);

protected:

	bool FindTransform(const float& fTime, MSkeletalAnimNode* pAnimNode, MTransform& trans);

private:
	friend class MModelResource;
	std::vector<MSkeletalAnimNode*> m_vSkeletalAnimNodes;
	MSkeleton* m_pSkeletonTemplate;

	MString m_strName;
	float m_fTicksDuration;
	float m_fTicksPerSecond;
};

class MORTY_CLASS MSkeletalAnimController : public MIAnimController
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
	virtual void Update(const float& fDelta) override;
private:
	MSkeletonInstance* m_pSkeletonIns;
	MSkeletalAnimation* m_pAnimation;
	bool m_bInitialized;

	float m_fTicks;
	MEAnimControllerState m_eState;
	bool m_bLoop;
};

#endif
