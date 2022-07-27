#include "MSkeletalAnimation.h"
#include "MMath.h"
#include "MJson.h"
#include "MEngine.h"
#include "MFileHelper.h"
#include "MResourceSystem.h"
#include "MSkeletonResource.h"
#include "MSkeletalAnimationResource.h"

MORTY_CLASS_IMPLEMENT(MSkeletalAnimation, MResource)

MSkeletalAnimation::MSkeletalAnimation()
	: MIAnimation()
	, m_Skeleton()
	, m_unIndex(0)
	, m_strName()
	, m_fTicksDuration(0.0f)
	, m_fTicksPerSecond(25.0f)
{

}

MSkeletalAnimation::~MSkeletalAnimation()
{

}

std::shared_ptr<MSkeleton> MSkeletalAnimation::GetSkeletonTemplate()
{
	return m_Skeleton.GetResource<MSkeletonResource>();
}

void MSkeletalAnimation::Update(const float& fTime, std::shared_ptr<MSkeletonInstance> pSkeletonIns, const MSkeletonAnimMap& skelAnimMap)
{
	std::vector<MBone>& bones = pSkeletonIns->GetAllBones();

	uint32_t unBonesSize = bones.size();

	for (uint32_t i = 0; i < unBonesSize; ++i)
	{
		MBone& bone = bones[i];

		int nAnimNodeIndex = skelAnimMap.m_vSkelToAnim[bone.unIndex];
		if(MGlobal::M_INVALID_INDEX == nAnimNodeIndex)
			continue;

		MSkeletalAnimNode& animNode = m_vSkeletalAnimNodes[nAnimNodeIndex];
		Matrix4 matParentTrans = bone.unParentIndex == MGlobal::M_INVALID_INDEX ? Matrix4::IdentityMatrix : bones[bone.unParentIndex].m_matWorldTransform;

		MTransform trans;
		if (FindTransform(fTime, animNode, trans))
		{
			//Use animation transform
			bone.m_matWorldTransform = matParentTrans * trans.GetMatrix();
		}
		else
		{
			//Use default transform
			bone.m_matWorldTransform = matParentTrans * pSkeletonIns->GetBoneTemplateByIndex(i)->m_matTransform;
		}
	}

	for (MBone& bone : bones)
	{
		bone.m_matWorldTransform = bone.m_matWorldTransform * bone.m_matOffsetMatrix;
	}

	pSkeletonIns->SetDirty();
}

MSkeletalAnimNode::MSkeletalAnimNode()
	: m_unPositionKeysNum(0)
	, m_unRotationKeysNum(0)
	, m_unScalingKeysNum(0)
	, m_vPositionKeys(nullptr)
	, m_vRotationKeys(nullptr)
	, m_vScalingKeys(nullptr)
{

}

MSkeletalAnimNode::~MSkeletalAnimNode()
{
	if (m_vPositionKeys) delete[] m_vPositionKeys;
	if (m_vRotationKeys) delete[] m_vRotationKeys;
	if (m_vScalingKeys) delete[] m_vScalingKeys;
}

void MSkeletalAnimNode::WriteToStruct(MStruct& srt)
{
	if (MVariantArray* pArray = srt.AppendMVariant<MVariantArray>("p"))
	{
		for (int i = 0; i < m_unPositionKeysNum; ++i)
		{
			if (MStruct* pNode = pArray->AppendMVariant<MStruct>())
			{
				pNode->AppendMVariant("t", m_vPositionKeys[i].mTime);
				pNode->AppendMVariant("v", m_vPositionKeys[i].mValue);
			}
		}
	}

	if (MVariantArray* pArray = srt.AppendMVariant<MVariantArray>("r"))
	{
		for (int i = 0; i < m_unRotationKeysNum; ++i)
		{
			if (MStruct* pNode = pArray->AppendMVariant<MStruct>())
			{
				pNode->AppendMVariant("t", m_vRotationKeys[i].mTime);
				pNode->AppendMVariant("v", m_vRotationKeys[i].mValue);
			}
		}
	}

	if (MVariantArray* pArray = srt.AppendMVariant<MVariantArray>("s"))
	{
		for (int i = 0; i < m_unScalingKeysNum; ++i)
		{
			if (MStruct* pNode = pArray->AppendMVariant<MStruct>())
			{
				pNode->AppendMVariant("t", m_vScalingKeys[i].mTime);
				pNode->AppendMVariant("v", m_vScalingKeys[i].mValue);
			}
		}
	}
}

void MSkeletalAnimNode::ReadFromStruct(const MStruct& srt)
{
	if (const MVariantArray* pArray = srt.FindMember<MVariantArray>("p"))
	{
		m_unPositionKeysNum = pArray->GetMemberCount();
		m_vPositionKeys = new MAnimNodeKey<Vector3>[m_unPositionKeysNum];
		for (int i = 0; i < m_unPositionKeysNum; ++i)
		{
			if (const MStruct* pNode = pArray->GetMember(i)->var.GetStruct())
			{
				pNode->FindMember("t", m_vPositionKeys[i].mTime);
				pNode->FindMember("v", m_vPositionKeys[i].mValue);
			}
		}
	}

	if (const MVariantArray* pArray = srt.FindMember<MVariantArray>("r"))
	{
		m_unRotationKeysNum = pArray->GetMemberCount();
		m_vRotationKeys = new MAnimNodeKey<Quaternion>[m_unRotationKeysNum];
		for (int i = 0; i < m_unRotationKeysNum; ++i)
		{
			if (const MStruct* pNode = pArray->GetMember(i)->var.GetStruct())
			{
				pNode->FindMember("t", m_vRotationKeys[i].mTime);
				pNode->FindMember("v", m_vRotationKeys[i].mValue);
			}
		}
	}

	if (const MVariantArray* pArray = srt.FindMember<MVariantArray>("s"))
	{
		m_unScalingKeysNum = pArray->GetMemberCount();
		m_vScalingKeys = new MAnimNodeKey<Vector3>[m_unScalingKeysNum];
		for (int i = 0; i < m_unScalingKeysNum; ++i)
		{
			if (const MStruct* pNode = pArray->GetMember(i)->var.GetStruct())
			{
				pNode->FindMember("t", m_vScalingKeys[i].mTime);
				pNode->FindMember("v", m_vScalingKeys[i].mValue);
			}
		}
	}
}

void MSkeletalAnimation::WriteToStruct(MStruct& srt)
{
	if (MString* pSkePath = srt.AppendMVariant<MString>("ske"))
	{
		if (std::shared_ptr<MResource> pSkeletonRes = m_Skeleton.GetResource())
		{
			*pSkePath = pSkeletonRes->GetResourcePath();
		}
	}

	if (MString* pName = srt.AppendMVariant<MString>("name"))
		*pName = m_strName;

	if (float* pTime = srt.AppendMVariant<float>("dur"))
		*pTime = m_fTicksDuration;

	if (float* pTime = srt.AppendMVariant<float>("sec"))
		*pTime = m_fTicksPerSecond;

	if (MVariantArray* pNodeArray = srt.AppendMVariant<MVariantArray>("nodes"))
	{
		for (MSkeletalAnimNode& node : m_vSkeletalAnimNodes)
		{
			if (MStruct* pNodeSrt = pNodeArray->AppendMVariant<MStruct>())
			{
				node.WriteToStruct(*pNodeSrt);
			}
			
		}
	}
}

void MSkeletalAnimation::ReadFromStruct(const MStruct& srt)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (const MString* pSkePath = srt.FindMember<MString>("ske"))
	{
		if (std::shared_ptr<MResource> pSkeletonRes = pResourceSystem->LoadResource(*pSkePath))
		{
			m_Skeleton = pSkeletonRes;
		}
	}

	srt.FindMember<MString>("name", m_strName);

	srt.FindMember<float>("dur", m_fTicksDuration);

	srt.FindMember<float>("sec", m_fTicksPerSecond);

	if (const MVariantArray* pNodeArray = srt.FindMember<MVariantArray>("nodes"))
	{
		uint32_t nSize = pNodeArray->GetMemberCount();
		m_vSkeletalAnimNodes.resize(nSize);
		for (uint32_t i = 0 ; i < nSize; ++i)
		{
			if (const MStruct* pNodeSrt = pNodeArray->GetMember(i)->var.GetStruct())
			{
				m_vSkeletalAnimNodes[i].ReadFromStruct(*pNodeSrt);
			}
		}
	}
}

void MSkeletalAnimation::OnDelete()
{
	m_Skeleton.SetResource(nullptr);

	Super::OnDelete();
}

bool MSkeletalAnimation::FindTransform(const float& fTime, const MSkeletalAnimNode& animNode, MTransform& trans)
{
	if (animNode.m_unPositionKeysNum + animNode.m_unRotationKeysNum + animNode.m_unScalingKeysNum == 0)
		return false;

	Vector3 v3Position, v3Scale;
	Quaternion quatRotation;

	for (int i = animNode.m_unPositionKeysNum - 1; i >= 0; --i)
	{
		const MSkeletalAnimNode::MAnimNodeKey<Vector3>& curkey = animNode.m_vPositionKeys[i];
		if (fTime >= curkey.mTime)
		{
			if (i == animNode.m_unPositionKeysNum - 1)
			{
				v3Position = curkey.mValue;
			}
			else
			{
				const MSkeletalAnimNode::MAnimNodeKey<Vector3>& nextKey = animNode.m_vPositionKeys[i + 1];
				if (nextKey.mTime - curkey.mTime > 1e-6f)
					v3Position = MMath::Lerp(curkey.mValue, nextKey.mValue, (fTime - curkey.mTime) / (nextKey.mTime - curkey.mTime));
				else
					v3Position = nextKey.mValue;
			}
			
			break;
		}
	}

	for (int i = animNode.m_unRotationKeysNum - 1; i >= 0; --i)
	{
		const MSkeletalAnimNode::MAnimNodeKey<Quaternion>& curkey = animNode.m_vRotationKeys[i];
		if (fTime >= curkey.mTime)
		{
			if (i == animNode.m_unRotationKeysNum - 1)
			{
				quatRotation = curkey.mValue;
			}
			else
			{
				const MSkeletalAnimNode::MAnimNodeKey<Quaternion>& nextKey = animNode.m_vRotationKeys[i + 1];
				if (nextKey.mTime - curkey.mTime > 1e-6f)
					quatRotation = Quaternion::Slerp(curkey.mValue, nextKey.mValue, (fTime - curkey.mTime) / (nextKey.mTime - curkey.mTime));
				else
					quatRotation = nextKey.mValue;
			}

			break;
		}
	}

	for (int i = animNode.m_unScalingKeysNum - 1; i >= 0; --i)
	{
		const MSkeletalAnimNode::MAnimNodeKey<Vector3>& curkey = animNode.m_vScalingKeys[i];
		if (fTime >= curkey.mTime)
		{
			if (i == animNode.m_unScalingKeysNum - 1)
			{
				v3Scale = curkey.mValue;
			}
			else
			{
				const MSkeletalAnimNode::MAnimNodeKey<Vector3>& nextKey = animNode.m_vScalingKeys[i + 1];
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

bool MSkeletalAnimation::Load(const MString& strResourcePath)
{
	MString code;
	if (!MFileHelper::ReadString(strResourcePath, code))
		return false;

	MVariant var;
	MJson::JsonToMVariant(code, var);
	if (MStruct* pSrt = var.GetStruct())
	{
		ReadFromStruct(*pSrt);

		return true;
	}

	return false;
}

bool MSkeletalAnimation::SaveTo(const MString& strResourcePath)
{
	MVariant var = MStruct();
	MStruct& srt = *var.GetStruct();

	WriteToStruct(srt);

	MString code;
	MJson::MVariantToJson(srt, code);

	return MFileHelper::WriteString(strResourcePath, code);
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

bool MSkeletalAnimController::Initialize(std::shared_ptr<MSkeletonInstance> pSkeletonIns, std::shared_ptr<MSkeletalAnimation> pAnimation)
{
	if (!pAnimation)
		return false;

	if (m_bInitialized)
		return false;

	if (nullptr == pSkeletonIns || nullptr == pAnimation)
		return false;

	m_pSkeletonIns = pSkeletonIns;
	m_AnimResource = pAnimation;
	m_pAnimation = pAnimation;

	BindMapping();

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

	m_fTicks += m_pAnimation->GetTicksPerSecond() * fDelta;
	if (m_fTicks >= m_pAnimation->GetTicksDuration())
	{
		if (m_bLoop)
		{
			m_fTicks = fmodf(m_fTicks, m_pAnimation->GetTicksDuration());
			if (bAnimStep)
				m_pAnimation->Update(fmodf(m_fTicks, m_pAnimation->GetTicksDuration()), m_pSkeletonIns, m_SkeletonAnimMap);
		}
		else
		{
			m_fTicks = m_pAnimation->GetTicksDuration();
			if (bAnimStep)
				m_pAnimation->Update(fmodf(m_fTicks, m_pAnimation->GetTicksDuration()), m_pSkeletonIns, m_SkeletonAnimMap);
			this->Stop();
		}
	}
	else
	{
		if (bAnimStep)
			m_pAnimation->Update(m_fTicks, m_pSkeletonIns, m_SkeletonAnimMap);
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

void MSkeletalAnimController::BindMapping()
{
	m_SkeletonAnimMap.m_vAnimToSkel.resize(m_pAnimation->GetSkeletonTemplate()->GetAllBones().size(), MGlobal::M_INVALID_INDEX);
	m_SkeletonAnimMap.m_vSkelToAnim.resize(m_pSkeletonIns->GetAllBones().size(), MGlobal::M_INVALID_INDEX);

	if (m_pSkeletonIns->GetSkeletonTemplate() == m_pAnimation->GetSkeletonTemplate())
	{
		for (uint32_t i = 0; i < m_SkeletonAnimMap.m_vAnimToSkel.size(); ++i)
		{
			m_SkeletonAnimMap.m_vSkelToAnim[i] = i;
			m_SkeletonAnimMap.m_vAnimToSkel[i] = i;
		}
	}
	else
	{
		for (MBone& skelBone : m_pSkeletonIns->GetAllBones())
		{
			const MBone* pAnimBone = m_pAnimation->GetSkeletonTemplate()->FindBoneByName(skelBone.strName);

			m_SkeletonAnimMap.m_vSkelToAnim[skelBone.unIndex] = pAnimBone->unIndex;
			m_SkeletonAnimMap.m_vAnimToSkel[pAnimBone->unIndex] = skelBone.unIndex;
		}
	}
}
