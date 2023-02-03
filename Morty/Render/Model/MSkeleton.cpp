#include "Model/MSkeleton.h"
#include <algorithm>

#include "Engine/MEngine.h"
#include "Flatbuffer/MBone_generated.h"
#include "Flatbuffer/MSkeleton_generated.h"
#include "Utility/MFileHelper.h"
#include "System/MRenderSystem.h"
#include "Material/MShaderParamSet.h"

MORTY_CLASS_IMPLEMENT(MSkeleton, MResource)


MBone::MBone()
	: m_matTransform(Matrix4::IdentityMatrix)
	, m_matOffsetMatrix(Matrix4::IdentityMatrix)
	, m_matWorldTransform(Matrix4::IdentityMatrix)
{

}

flatbuffers::Offset<void> MBone::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	auto fbName = fbb.CreateString(strName);
	auto fbTransform = m_matTransform.Serialize(fbb);
	auto fbOffset = m_matOffsetMatrix.Serialize(fbb);
	auto fbChildren = fbb.CreateVector(vChildrenIndices);

	mfbs::MBoneBuilder builder(fbb);

	builder.add_name(fbName);
	builder.add_index(unIndex);
	builder.add_parent(unParentIndex);
	builder.add_transform(fbTransform);
	builder.add_offset(fbOffset);
	builder.add_children(fbChildren);

	return builder.Finish().Union();
}

void MBone::Deserialize(const void* pBufferPointer)
{
	const mfbs::MBone* fbData = reinterpret_cast<const mfbs::MBone*>(pBufferPointer);

	strName = fbData->name()->c_str();
	unIndex = fbData->index();
	unParentIndex = fbData->parent();
	m_matTransform.Deserialize(fbData->transform());
	m_matOffsetMatrix.Deserialize(fbData->offset());

	vChildrenIndices.resize(fbData->children()->size());
	for (size_t idx = 0; idx < fbData->children()->size(); ++idx)
	{
		vChildrenIndices[idx] = fbData->children()->Get(idx);
	}
}

MSkeleton::MSkeleton()
	: m_tBonesMap()
	, m_vAllBones()
{

}

MSkeleton::~MSkeleton()
{

}

void MSkeleton::CopyAllBones(std::vector<MBone*>& allBones)
{
	for (MBone* pBone : allBones)
		delete pBone;

	allBones.resize(m_vAllBones.size());

	for (int i = 0; i < m_vAllBones.size(); ++i)
	{
		allBones[i] = new MBone();
		(*allBones[i]) = (m_vAllBones[i]);
	}
}

MBone* MSkeleton::FindBoneByName(const MString& strName)
{
	auto iter = m_tBonesMap.find(strName);
	if (iter != m_tBonesMap.cend())
	{
		return &m_vAllBones[iter->second];
	}

	return nullptr;
}

const MBone* MSkeleton::FindBoneByName(const MString& strName) const
{
	auto iter = m_tBonesMap.find(strName);
	if (iter != m_tBonesMap.cend())
	{
		return &m_vAllBones[iter->second];
	}

	return nullptr;
}

MBone* MSkeleton::AppendBone(const MString& strName)
{
	m_vAllBones.push_back(MBone());
	MBone& bone = m_vAllBones.back();
	bone.strName = strName;
	bone.unIndex = m_vAllBones.size() - 1;
	bone.unParentIndex = MGlobal::M_INVALID_INDEX;
	m_tBonesMap[strName] = bone.unIndex;
	return &m_vAllBones.back();
}

void MSkeleton::SortByDeep()
{
	std::vector<int> map(m_vAllBones.size());
	std::map<uint32_t, int> tDeep;

	for (MBone& bone : m_vAllBones)
	{
		int deep = 0;
		uint32_t unParentIdx = bone.unIndex;
		while (unParentIdx != MGlobal::M_INVALID_INDEX)
		{
			unParentIdx = m_vAllBones[unParentIdx].unParentIndex;
			++deep;
		}

		tDeep[bone.unIndex] = deep;
	}

	std::vector<MBone>& vBones = m_vAllBones;

	std::sort(vBones.begin(), vBones.end(), [&tDeep](MBone& a, MBone& b) { return tDeep[a.unIndex] < tDeep[b.unIndex]; });

	for (uint32_t i = 0; i < vBones.size(); ++i)
		map[vBones[i].unIndex] = i;

	for (uint32_t i = 0; i < vBones.size(); ++i)
	{
		MBone& bone = vBones[i];
		bone.unIndex = map[bone.unIndex];
		if (MGlobal::M_INVALID_INDEX != bone.unParentIndex)
			bone.unParentIndex = map[bone.unParentIndex];
		for (uint32_t& index : bone.vChildrenIndices)
			index = map[index];
	}

	for (auto& iter : m_tBonesMap)
		iter.second = map[iter.second];

}

void MSkeleton::RebuildBonesMap()
{
	m_tBonesMap.clear();
	for (MBone& bone : m_vAllBones)
	{
		m_tBonesMap[bone.strName] = bone.unIndex;
	}
}

MSkeletonInstance::MSkeletonInstance(std::shared_ptr<const MSkeleton> templateSke)
	: m_pEngine(templateSke->GetEngine())
	, m_pSkeletonTemplate(templateSke)
	, m_bShaderParamSetDirty(true)
	, m_pShaderPropertyBlock(MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_SKELETON))
	, m_pShaderBonesArray(nullptr)
{
	m_vAllBones = m_pSkeletonTemplate->GetAllBones();

	ResetOriginPose();
}

MSkeletonInstance::MSkeletonInstance(const MSkeletonInstance& instance)
	: m_pEngine(instance.m_pEngine)
	, m_pSkeletonTemplate(instance.m_pSkeletonTemplate)
	, m_bShaderParamSetDirty(true)
	, m_pShaderPropertyBlock(MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_SKELETON))
	, m_pShaderBonesArray(nullptr)
{
	m_vAllBones = m_pSkeletonTemplate->GetAllBones();
}

MSkeletonInstance::~MSkeletonInstance()
{
	m_pShaderBonesArray = nullptr;

	if (m_pShaderPropertyBlock)
	{
		MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
		m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
		m_pShaderPropertyBlock = nullptr;
	}
}

MBone* MSkeletonInstance::FindBoneByName(const MString& strName)
{
	const MBone* pBoneTemp = m_pSkeletonTemplate->FindBoneByName(strName);
	if (nullptr == pBoneTemp)
		return nullptr;

	return &m_vAllBones[pBoneTemp->unIndex];
}

const MBone* MSkeletonInstance::FindBoneTemplateByName(const MString& strName)
{
	return m_pSkeletonTemplate->FindBoneByName(strName);
}

const MBone* MSkeletonInstance::GetBoneTemplateByIndex(const uint32_t& unIndex)
{
	return &m_pSkeletonTemplate->GetAllBones()[unIndex];
}

void MSkeletonInstance::ResetOriginPose()
{
	for (MBone& bone : m_vAllBones)
	{
		if (bone.unParentIndex != MGlobal::M_INVALID_INDEX)
		{
			bone.m_matWorldTransform = m_vAllBones[bone.unParentIndex].m_matWorldTransform * bone.m_matTransform;
		}
		else
		{
			bone.m_matWorldTransform = bone.m_matTransform;
		}
	}

	for (MBone& bone : m_vAllBones)
	{
		bone.m_matWorldTransform = bone.m_matWorldTransform * bone.m_matOffsetMatrix;
	}
}


std::shared_ptr<MShaderPropertyBlock> MSkeletonInstance::GetShaderParamSet()
{
	if (!m_pShaderBonesArray)
	{
		std::shared_ptr<MShaderConstantParam> pBonesSet = std::make_shared<MShaderConstantParam>();
		pBonesSet->unSet = 3;
		pBonesSet->unBinding = 0;

		MVariantArray bonesArr;
		MVariantArrayBuilder builder(bonesArr);
		for (int i = 0; i < 128; ++i)
		{
			builder.AppendVariant(Matrix4());
		}

		MVariantStruct bonesSrt;
		MVariantStructBuilder srtBuilder(bonesSrt);
		srtBuilder.AppendVariant("u_vBonesMatrix", bonesArr);
		srtBuilder.Finish();

		pBonesSet->var = std::move(MVariant(bonesSrt));

		m_pShaderPropertyBlock->m_vParams.push_back(pBonesSet);

		m_pShaderBonesArray = &pBonesSet->var.GetValue<MVariantStruct>().GetVariant<MVariantArray>("u_vBonesMatrix");
	}

	if (m_bShaderParamSetDirty)
	{
		const std::vector<MBone>& bones = GetAllBones();
		uint32_t size = bones.size();
		if (size > MRenderGlobal::BONES_MAX_NUMBER) size = MRenderGlobal::BONES_MAX_NUMBER;

		for (uint32_t i = 0; i < size; ++i)
		{
			(*m_pShaderBonesArray)[i].SetValue(bones[i].m_matWorldTransform);
		}

		m_pShaderPropertyBlock->m_vParams[0]->SetDirty();

		m_bShaderParamSetDirty = false;
	}

	return m_pShaderPropertyBlock;
}

void MSkeletonInstance::SetDirty()
{
	m_bShaderParamSetDirty = true;
}

flatbuffers::Offset<void> MSkeleton::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	std::vector<flatbuffers::Offset<mfbs::MBone>> vBoneOffset;

	auto fbBones = fbb.CreateVector(vBoneOffset);

	mfbs::MSkeletonBuilder builder(fbb);

	builder.add_bones(fbBones);

	return builder.Finish().Union();
}

void MSkeleton::Deserialize(const void* pBufferPointer)
{
	const mfbs::MSkeleton* fbData = reinterpret_cast<const mfbs::MSkeleton*>(pBufferPointer);

	m_vAllBones.resize(fbData->bones()->size());
	for (size_t idx = 0; idx < fbData->bones()->size(); ++idx)
	{
		m_vAllBones[idx].Deserialize(fbData->bones()->Get(idx));
	}
}

bool MSkeleton::Load(const MString& strResourcePath)
{
	std::vector<MByte> data;
	MFileHelper::ReadData(strResourcePath, data);

	flatbuffers::FlatBufferBuilder fbb;
	fbb.PushBytes((const uint8_t*)data.data(), data.size());

	const mfbs::MSkeleton* fbSkeleton = mfbs::GetMSkeleton(fbb.GetCurrentBufferPointer());

	Deserialize(fbSkeleton);
	return true;
}

bool MSkeleton::SaveTo(const MString& strResourcePath)
{
	flatbuffers::FlatBufferBuilder fbb;
	Serialize(fbb);

	std::vector<MByte> data(fbb.GetSize());
	memcpy(data.data(), (MByte*)fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));

	return MFileHelper::WriteData(strResourcePath, data);
}

