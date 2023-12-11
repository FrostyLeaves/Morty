#include "Model/MSkeletalAnimation.h"
#include "Math/MMath.h"
#include "Engine/MEngine.h"
#include "Flatbuffer/MSkeletalAnimation_generated.h"
#include "Utility/MFileHelper.h"
#include "System/MResourceSystem.h"
#include "Resource/MSkeletonResource.h"
#include "Resource/MSkeletalAnimationResource.h"
#include "Model/MSkeletonInstance.h"

MSkeletalAnimation::MSkeletalAnimation()
	: MIAnimation()
	, m_unIndex(0)
	, m_strName()
	, m_fTicksDuration(0.0f)
	, m_fTicksPerSecond(25.0f)
{

}

MSkeletalAnimation::~MSkeletalAnimation()
{

}

void MSkeletalAnimation::SamplePose(MSkeletonPose& outputPose, const float& fTime, MSkeletonInstance* pSkeletonIns, const MSkeletonAnimMap& skelAnimMap) const
{
	const std::vector<MBone>& bones = pSkeletonIns->GetAllBones();

	const uint32_t nBonesSize = static_cast<uint32_t>(bones.size());
	outputPose.vBoneMatrix.resize(nBonesSize);

	for (uint32_t i = 0; i < nBonesSize; ++i)
	{
		const MBone& bone = bones[i];

		const int nAnimNodeIndex = skelAnimMap.m_vSkelToAnim[bone.unIndex];
		if (MGlobal::M_INVALID_INDEX == nAnimNodeIndex)
		{
			continue;
		}

		const MSkeletalAnimNode& animNode = m_vSkeletalAnimNodes[nAnimNodeIndex];
		Matrix4 matParentTrans = bone.unParentIndex == MGlobal::M_INVALID_UINDEX ? Matrix4::IdentityMatrix : outputPose.vBoneMatrix[bone.unParentIndex];

		MTransform trans;
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
		const MBone& bone = bones[i];
		outputPose.vBoneMatrix[i] = outputPose.vBoneMatrix[i] * bone.m_matOffsetMatrix;
	}
}

void MSkeletalAnimation::SetSkeletonTemplate(MSkeleton* pSkeleton)
{
	m_pSkeleton = pSkeleton;
}

flatbuffers::Offset<void> MSkeletalAnimNode::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	auto fbPositionTrack = fbb.CreateVectorOfStructs(m_vPositionTrack);
	auto fbRotationTrack = fbb.CreateVectorOfStructs(m_vRotationTrack);
	auto fbScaleTrack = fbb.CreateVectorOfStructs(m_vScaleTrack);

	mfbs::MSkeletalAnimNodeBuilder builder(fbb);

	builder.add_position_track(fbPositionTrack);
	builder.add_rotation_track(fbRotationTrack);
	builder.add_scale_track(fbScaleTrack);

	return builder.Finish().Union();
}

void MSkeletalAnimNode::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
	const mfbs::MSkeletalAnimNode* fbcomponent = mfbs::GetMSkeletalAnimNode(fbb.GetCurrentBufferPointer());
	Deserialize(fbcomponent);
}

void MSkeletalAnimNode::Deserialize(const void* pBufferPointer)
{
	const mfbs::MSkeletalAnimNode* fbData = reinterpret_cast<const mfbs::MSkeletalAnimNode*>(pBufferPointer);

	m_vPositionTrack.resize(fbData->position_track()->size());
	for (size_t nIdx = 0; nIdx < fbData->position_track()->size(); ++nIdx)
	{
		m_vPositionTrack[nIdx] = *fbData->position_track()->Get(static_cast<uint32_t>(nIdx));
	}

	m_vRotationTrack.resize(fbData->rotation_track()->size());
	for (size_t nIdx = 0; nIdx < fbData->rotation_track()->size(); ++nIdx)
	{
		m_vRotationTrack[nIdx] = *fbData->rotation_track()->Get(static_cast<uint32_t>(nIdx));
	}

	m_vScaleTrack.resize(fbData->scale_track()->size());
	for (size_t nIdx = 0; nIdx < fbData->scale_track()->size(); ++nIdx)
	{
		m_vScaleTrack[nIdx] = *fbData->scale_track()->Get(static_cast<uint32_t>(nIdx));
	}
}

flatbuffers::Offset<void> MSkeletalAnimation::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	std::vector<flatbuffers::Offset<mfbs::MSkeletalAnimNode>> fbAnimNode;
	for (const MSkeletalAnimNode& node : m_vSkeletalAnimNodes)
	{
		fbAnimNode.push_back(node.Serialize(fbb).o);
	}

	auto fbName = fbb.CreateString(m_strName);
    auto fbAnimNodeOffset = fbb.CreateVector(fbAnimNode);


	mfbs::MSkeletalAnimationBuilder builder(fbb);

	builder.add_name(fbName);
	builder.add_duration(m_fTicksDuration);
	builder.add_speed(m_fTicksPerSecond);
	builder.add_animation_node(fbAnimNodeOffset);

	return builder.Finish().Union();
}

void MSkeletalAnimation::Deserialize(const void* pBufferPointer)
{
	const mfbs::MSkeletalAnimation* fbData = reinterpret_cast<const mfbs::MSkeletalAnimation*>(pBufferPointer);

	m_strName = fbData->name()->str();
	m_fTicksDuration = fbData->duration();
	m_fTicksPerSecond = fbData->speed();

	if (fbData->animation_node())
	{
		m_vSkeletalAnimNodes.resize(fbData->animation_node()->size());
		for (size_t nIdx = 0; nIdx < fbData->animation_node()->size(); ++nIdx)
		{
			m_vSkeletalAnimNodes[nIdx].Deserialize(fbData->animation_node()->Get(static_cast<uint32_t>(nIdx)));
		}
	}
	else
	{
		m_vSkeletalAnimNodes.clear();
	}
}

bool MSkeletalAnimation::FindTransform(const float& fTime, const MSkeletalAnimNode& animNode, MTransform& trans) const
{
	if (animNode.m_vPositionTrack.size() + animNode.m_vRotationTrack.size() + animNode.m_vScaleTrack.size() == 0)
		return false;

	Vector3 v3Position, v3Scale;
	Quaternion quatRotation;

	for (size_t nIdx = 0; nIdx < animNode.m_vPositionTrack.size(); ++nIdx)
	{
		size_t i = animNode.m_vPositionTrack.size() - nIdx - 1;

		const auto& curkey = animNode.m_vPositionTrack[i];
		if (fTime >= curkey.time())
		{
			if (i == animNode.m_vPositionTrack.size() - 1)
			{
				v3Position = curkey.value();
			}
			else
			{
				const auto& nextKey = animNode.m_vPositionTrack[i + 1];
				if (nextKey.time() - curkey.time() > 1e-6f)
				{
					Vector3 current, next;
					current.Deserialize(&curkey.value());
					next.Deserialize(&nextKey.value());
					v3Position = MMath::Lerp(current, next, (fTime - curkey.time()) / (nextKey.time() - curkey.time()));
				}
				else
				{
					v3Position.Deserialize(&nextKey.value());
				}
			}
			
			break;
		}
	}
	
	for (size_t nIdx = 0; nIdx < animNode.m_vRotationTrack.size(); ++nIdx)
	{
		size_t i = animNode.m_vRotationTrack.size() - nIdx - 1;

		const auto& curkey = animNode.m_vRotationTrack[i];
		if (fTime >= curkey.time())
		{
			if (i == animNode.m_vRotationTrack.size() - 1)
			{
				quatRotation = curkey.value();
			}
			else
			{
				const auto& nextKey = animNode.m_vRotationTrack[i + 1];
				if (nextKey.time() - curkey.time() > 1e-6f)
				{
					Quaternion current, next;
					current.Deserialize(&curkey.value());
					next.Deserialize(&nextKey.value());
					quatRotation = Quaternion::Slerp(current, next, (fTime - curkey.time()) / (nextKey.time() - curkey.time()));
				}
				else
				{
					quatRotation.Deserialize(&nextKey.value());
				}
			}

			break;
		}
	}

	for (size_t nIdx = 0; nIdx < animNode.m_vScaleTrack.size(); ++nIdx)
	{
		size_t i = animNode.m_vScaleTrack.size() - nIdx - 1;

		const auto& curkey = animNode.m_vScaleTrack[i];
		if (fTime >= curkey.time())
		{
			if (i == animNode.m_vScaleTrack.size() - 1)
			{
				v3Scale = curkey.value();
			}
			else
			{
				const auto& nextKey = animNode.m_vScaleTrack[i + 1];
				if (nextKey.time() - curkey.time() > 1e-6f)
				{
					Vector3 current, next;
					current.Deserialize(&curkey.value());
					next.Deserialize(&nextKey.value());
					v3Scale = MMath::Lerp(current, next, (fTime - curkey.time()) / (nextKey.time() - curkey.time()));
				}
				else
				{
					v3Scale.Deserialize(&nextKey.value());
				}
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

bool MSkeletalAnimController::Initialize(MSkeletonInstance* pSkeletonIns, std::shared_ptr<MSkeletalAnimationResource> pAnimationResource)
{
	if (!pAnimationResource)
		return false;

	if (m_bInitialized)
		return false;

	m_pSkeletonIns = pSkeletonIns;
	m_AnimResource = pAnimationResource;
	m_pAnimation = pAnimationResource->GetAnimation();

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

void MSkeletalAnimController::NextStep(const float& fDelta, const bool& bAnimStep)
{
	if (!m_bInitialized)
	{
		return;
	}

	m_fTicks += m_pAnimation->GetTicksPerSecond() * fDelta;
	if (m_fTicks >= m_pAnimation->GetTicksDuration())
	{
		if (m_bLoop)
		{
			m_fTicks = fmodf(m_fTicks, m_pAnimation->GetTicksDuration());
		}
		else
		{
			m_fTicks = m_pAnimation->GetTicksDuration();
			this->Stop();
		}
	}

	if (bAnimStep)
	{
		m_pAnimation->SamplePose(m_pSkeletonIns->GetCurrentPose(), m_fTicks, m_pSkeletonIns, m_SkeletonAnimMap);
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

std::shared_ptr<MSkeletalAnimationResource> MSkeletalAnimController::GetAnimationResource() const
{
	return m_AnimResource.GetResource<MSkeletalAnimationResource>();
}

void MSkeletalAnimController::BindMapping()
{
	if (!m_pSkeletonIns)
	{
		return;
	}

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
