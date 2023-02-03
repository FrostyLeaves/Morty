#include "Model/MSkeletalAnimation.h"
#include "Math/MMath.h"
#include "Engine/MEngine.h"
#include "Flatbuffer/MSkeletalAnimation_generated.h"
#include "Utility/MFileHelper.h"
#include "System/MResourceSystem.h"
#include "Resource/MSkeletonResource.h"
#include "Resource/MSkeletalAnimationResource.h"

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

flatbuffers::Offset<void> MSkeletalAnimNode::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	mfbs::MSkeletalAnimNodeBuilder builder(fbb);

	builder.add_position_track(fbb.CreateVectorOfStructs(m_vPositionTrack));
	builder.add_rotation_track(fbb.CreateVectorOfStructs(m_vRotationTrack));
	builder.add_scale_track(fbb.CreateVectorOfStructs(m_vScaleTrack));

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
		m_vPositionTrack[nIdx] = *fbData->position_track()->Get(nIdx);
	}

	m_vRotationTrack.resize(fbData->rotation_track()->size());
	for (size_t nIdx = 0; nIdx < fbData->rotation_track()->size(); ++nIdx)
	{
		m_vRotationTrack[nIdx] = *fbData->rotation_track()->Get(nIdx);
	}

	m_vScaleTrack.resize(fbData->scale_track()->size());
	for (size_t nIdx = 0; nIdx < fbData->scale_track()->size(); ++nIdx)
	{
		m_vScaleTrack[nIdx] = *fbData->scale_track()->Get(nIdx);
	}
}

flatbuffers::Offset<void> MSkeletalAnimation::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	std::vector<flatbuffers::Offset<mfbs::MSkeletalAnimNode>> fbAnimNode;
	for (const MSkeletalAnimNode& node : m_vSkeletalAnimNodes)
	{
		fbAnimNode.push_back(node.Serialize(fbb).o);
	}
	auto fbAnimNodeOffset = fbb.CreateVector(fbAnimNode);
	auto fbResource = m_Skeleton.Serialize(fbb);

	mfbs::MSkeletalAnimationBuilder builder(fbb);

	builder.add_name(fbb.CreateString(m_strName));
	builder.add_duration(m_fTicksDuration);
	builder.add_speed(m_fTicksPerSecond);
	builder.add_skeleton_resource(fbResource.o);
	builder.add_animation_node(fbAnimNodeOffset);

	return builder.Finish().Union();
}

void MSkeletalAnimation::Deserialize(const void* pBufferPointer)
{
	const mfbs::MSkeletalAnimation* fbData = reinterpret_cast<const mfbs::MSkeletalAnimation*>(pBufferPointer);

	m_strName = fbData->name()->str();
	m_fTicksDuration = fbData->duration();
	m_fTicksPerSecond = fbData->speed();
	m_Skeleton.Deserialize(GetEngine(), fbData->skeleton_resource());

	if (fbData->animation_node())
	{
		m_vSkeletalAnimNodes.resize(fbData->animation_node()->size());
		for (size_t nIdx = 0; nIdx < fbData->animation_node()->size(); ++nIdx)
		{
			m_vSkeletalAnimNodes[nIdx].Deserialize(fbData->animation_node()->Get(nIdx));
		}
	}
	else
	{
		m_vSkeletalAnimNodes.clear();
	}
}

void MSkeletalAnimation::OnDelete()
{
	m_Skeleton.SetResource(nullptr);

	Super::OnDelete();
}

bool MSkeletalAnimation::FindTransform(const float& fTime, const MSkeletalAnimNode& animNode, MTransform& trans)
{
	if (animNode.m_vPositionTrack.size() + animNode.m_vRotationTrack.size() + animNode.m_vScaleTrack.size() == 0)
		return false;

	Vector3 v3Position, v3Scale;
	Quaternion quatRotation;

	for (int i = animNode.m_vPositionTrack.size() - 1; i >= 0; --i)
	{
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

	for (int i = animNode.m_vRotationTrack.size() - 1; i >= 0; --i)
	{
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

	for (int i = animNode.m_vScaleTrack.size() - 1; i >= 0; --i)
	{
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

bool MSkeletalAnimation::Load(const MString& strResourcePath)
{
	std::vector<MByte> data;
	MFileHelper::ReadData(strResourcePath, data);

	flatbuffers::FlatBufferBuilder fbb;
	fbb.PushBytes((const uint8_t*)data.data(), data.size());

	const mfbs::MSkeletalAnimation* fbAnimation = mfbs::GetMSkeletalAnimation(fbb.GetCurrentBufferPointer());

	Deserialize(fbAnimation);
	return true;
}

bool MSkeletalAnimation::SaveTo(const MString& strResourcePath)
{
	flatbuffers::FlatBufferBuilder fbb;
	Serialize(fbb);

	std::vector<MByte> data(fbb.GetSize());
	memcpy(data.data(), (MByte*)fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));

	return MFileHelper::WriteData(strResourcePath, data);
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
