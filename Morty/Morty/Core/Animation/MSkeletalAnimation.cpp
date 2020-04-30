#include "MSkeletalAnimation.h"
#include "MMath.h"

MSkeletalAnimation::MSkeletalAnimation()
	: m_pSkeletonTemplate(nullptr)
	, m_unIndex(0)
	, m_strName()
	, m_fTicksDuration(0.0f)
	, m_fTicksPerSecond(25.0f)
{

}

MSkeletalAnimation::~MSkeletalAnimation()
{

}

void MSkeletalAnimation::Update(const float& fTime, MSkeletonInstance* pSkeletonIns)
{
	const std::vector<MBone*>& bones = pSkeletonIns->GetAllBones();

	for (unsigned int i = 0; i < bones.size(); ++i)
	{
		MBone* pBone = bones[i];
// 		if(pBone->unIndex >= m_vSkeletalAnimNodes.size())
// 			continue;

		MSkeletalAnimNode* pAnimNode = m_vSkeletalAnimNodes[pBone->unIndex];
		Matrix4 matLocalTrans;
		MTransform trans;
		if (pAnimNode && FindTransform(fTime, pAnimNode, trans))
		{
			//Use animation transform
			matLocalTrans = trans.GetMatrix();
		}
		else
		{
			//Use default transform
			matLocalTrans = pSkeletonIns->GetBoneTemplateByIndex(i)->m_matTransform;
		}

		Matrix4 matParentTrans = pBone->unParentIndex == MBone::InvalidIndex ? Matrix4::IdentityMatrix : bones[pBone->unParentIndex]->m_matTransform;
		pBone->m_matTransform = matParentTrans * matLocalTrans;
	}

}

bool MSkeletalAnimation::FindTransform(const float& fTime, MSkeletalAnimNode* pAnimNode, MTransform& trans)
{
	Vector3 v3Position, v3Scale;
	Quaternion quatRotation;

	for (unsigned int i = pAnimNode->m_unPositionKeysNum - 1; i >= 0; --i)
	{
		const MSkeletalAnimNode::MAnimNodeKey<Vector3>& curkey = pAnimNode->m_vPositionKeys[i];
		if (fTime >= curkey.mTime)
		{
			if (i == pAnimNode->m_unPositionKeysNum - 1)
			{
				v3Position = curkey.mValue;
			}
			else
			{
				const MSkeletalAnimNode::MAnimNodeKey<Vector3>& nextKey = pAnimNode->m_vPositionKeys[i + 1];
				if (nextKey.mTime - curkey.mTime > 1e-6f)
					v3Position = MMath::Lerp(curkey.mValue, nextKey.mValue, (fTime - curkey.mTime) / (nextKey.mTime - curkey.mTime));
				else
					v3Position = nextKey.mValue;
			}
			
			break;
		}
	}

	for (unsigned int i = pAnimNode->m_unRotationKeysNum - 1; i >= 0; --i)
	{
		const MSkeletalAnimNode::MAnimNodeKey<Quaternion>& curkey = pAnimNode->m_vRotationKeys[i];
		if (fTime >= curkey.mTime)
		{
			if (i == pAnimNode->m_unRotationKeysNum - 1)
			{
				quatRotation = curkey.mValue;
			}
			else
			{
				const MSkeletalAnimNode::MAnimNodeKey<Quaternion>& nextKey = pAnimNode->m_vRotationKeys[i + 1];
				if (nextKey.mTime - curkey.mTime > 1e-6f)
					quatRotation = Quaternion::Slerp(curkey.mValue, nextKey.mValue, (fTime - curkey.mTime) / (nextKey.mTime - curkey.mTime));
				else
					quatRotation = nextKey.mValue;
			}

			break;
		}
	}

	for (unsigned int i = pAnimNode->m_unScalingKeysNum - 1; i >= 0; --i)
	{
		const MSkeletalAnimNode::MAnimNodeKey<Vector3>& curkey = pAnimNode->m_vScalingKeys[i];
		if (fTime >= curkey.mTime)
		{
			if (i == pAnimNode->m_unScalingKeysNum - 1)
			{
				v3Scale = curkey.mValue;
			}
			else
			{
				const MSkeletalAnimNode::MAnimNodeKey<Vector3>& nextKey = pAnimNode->m_vScalingKeys[i + 1];
				if (nextKey.mTime - curkey.mTime > 1e-6f)
					v3Scale = MMath::Lerp(curkey.mValue, nextKey.mValue, (fTime - curkey.mTime) / (nextKey.mTime - curkey.mTime));
				else
					v3Scale = nextKey.mValue;
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
	: m_pSkeletonIns(nullptr)
	, m_pAnimation(nullptr)
	, m_bInitialized(false)
	, m_fTicks(0.0f)
	, m_eState(MIAnimController::MEAnimControllerState::EStop)
	, m_bLoop(false)
{

}

MSkeletalAnimController::~MSkeletalAnimController()
{

}

bool MSkeletalAnimController::Initialize(MSkeletonInstance* pSkeletonIns, MSkeletalAnimation* pAnimation)
{
	if (m_bInitialized)
		return false;

	if (nullptr == pSkeletonIns || nullptr == pAnimation)
		return false;

	if (pSkeletonIns->GetSkeletonTemplate() != pAnimation->GetSkeletonTemplate())
		return false;

	m_pSkeletonIns = pSkeletonIns;
	m_pAnimation = pAnimation;

	m_bInitialized = true;

	return true;
}

void MSkeletalAnimController::Play()
{
	if (!m_bInitialized) return;

	m_eState = MEAnimControllerState::EPlay;

}

void MSkeletalAnimController::Pause()
{
	if (!m_bInitialized) return;

	m_eState = MEAnimControllerState::EPause;
}

void MSkeletalAnimController::Stop()
{
	if (!m_bInitialized) return;

	m_eState = MEAnimControllerState::EStop;
}

void MSkeletalAnimController::SetLoop(const bool& bLoop)
{
	m_bLoop = bLoop;
}

void MSkeletalAnimController::Update(const float& fDelta, const bool& bAnimStep)
{
	if (!m_bInitialized) return;

	//if (MEAnimControllerState::EPlay == m_eState)
	{
		m_fTicks += m_pAnimation->GetTicksPerSecond() * fDelta;
		if (m_fTicks >= m_pAnimation->GetTicksDuration())
		{
			if (m_bLoop)
			{
				m_fTicks = fmodf(m_fTicks, m_pAnimation->GetTicksDuration());
				if (bAnimStep)
					m_pAnimation->Update(fmodf(m_fTicks, m_pAnimation->GetTicksDuration()), m_pSkeletonIns);
			}
			else
			{
				m_fTicks = m_pAnimation->GetTicksDuration();
				if (bAnimStep)
					m_pAnimation->Update(fmodf(m_fTicks, m_pAnimation->GetTicksDuration()), m_pSkeletonIns);
				this->Stop();
			}
		}
		else
		{
			if (bAnimStep)
				m_pAnimation->Update(m_fTicks, m_pSkeletonIns);
		}
	}
}

void MSkeletalAnimController::SetPercent(const float& fPercent)
{
	if (nullptr == m_pAnimation)
		return;

	if (fPercent <= 0.0f)
		m_fTicks = 0.0f;
	else if (fPercent >= 100.0f)
		m_fTicks = m_pAnimation->GetTicksDuration();
	else
		m_fTicks = m_pAnimation->GetTicksDuration() * fPercent / 100.0f;
}	

float MSkeletalAnimController::GetPercent()
{
	if (nullptr == m_pAnimation)
		return 0.0f;

	return m_fTicks / m_pAnimation->GetTicksDuration() * 100.0f;
}
