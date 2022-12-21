#include "Model/MSkeleton.h"
#include <algorithm>

#include "Utility/MJson.h"
#include "Engine/MEngine.h"
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

void MBone::WriteToStruct(MStruct& srt)
{
	MVariantArray vChildren;
	for (uint32_t unChildIdx : vChildrenIndices)
		vChildren.AppendValue((int)unChildIdx);

	srt.SetValue("Name", strName);
	srt.SetValue("Index", (int)unIndex);
	srt.SetValue("ParentIndex", (int)unParentIndex);
	srt.SetValue("matTrans", m_matTransform);
	srt.SetValue("matOffset", m_matOffsetMatrix);
	srt.SetValue("Children", vChildren);
}

void MBone::ReadFromStruct(const MStruct& srt)
{
	srt.GetValue<MString>("Name", strName);

	if (const int* value = srt.GetValue<int>("Index"))
		unIndex = *value;

	if (const int* value = srt.GetValue<int>("ParentIndex"))
		unParentIndex = *value;

	srt.GetValue<Matrix4>("matTrans", m_matTransform);
	srt.GetValue<Matrix4>("matOffset", m_matOffsetMatrix);
		
	if (const MVariantArray* pChildren = srt.GetValue<MVariantArray>("Children"))
	{
		uint32_t unChildrenCount = pChildren->GetMemberCount();
		vChildrenIndices.resize(unChildrenCount);

		for (uint32_t cldIdx = 0; cldIdx < unChildrenCount; ++cldIdx)
		{
			if (const int* pIndex = pChildren->GetMember<int>(cldIdx))
				vChildrenIndices[cldIdx] = *pIndex;
		}
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
	, m_pShaderParamSet(MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_SKELETON))
	, m_pShaderBonesArray(nullptr)
{
	m_vAllBones = m_pSkeletonTemplate->GetAllBones();

	ResetOriginPose();
}

MSkeletonInstance::MSkeletonInstance(const MSkeletonInstance& instance)
	: m_pEngine(instance.m_pEngine)
	, m_pSkeletonTemplate(instance.m_pSkeletonTemplate)
	, m_bShaderParamSetDirty(true)
	, m_pShaderParamSet(MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_SKELETON))
	, m_pShaderBonesArray(nullptr)
{
	m_vAllBones = m_pSkeletonTemplate->GetAllBones();
}

MSkeletonInstance::~MSkeletonInstance()
{
	m_pShaderBonesArray = nullptr;

	if (m_pShaderParamSet)
	{
		MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
		m_pShaderParamSet->DestroyBuffer(pRenderSystem->GetDevice());
		delete m_pShaderParamSet;
		m_pShaderParamSet = nullptr;
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
		MShaderConstantParam* pBonesSet = new MShaderConstantParam();
		pBonesSet->unSet = 3;
		pBonesSet->unBinding = 0;

		MVariantArray bonesArr;
		for (int i = 0; i < 128; ++i)
			bonesArr.AppendValue<Matrix4>();

		MStruct bonesSrt;
		bonesSrt.SetValue("U_vBonesMatrix", bonesArr);

		pBonesSet->var = bonesSrt;

		m_pShaderParamSet->m_vParams.push_back(pBonesSet);
	//	m_pShaderParamSet->GenerateBuffer(m_pEngine->GetDevice());

		m_pShaderBonesArray = pBonesSet->var.GetStruct()->GetValue<MVariantArray>("U_vBonesMatrix");
	}

	if (m_bShaderParamSetDirty)
	{
		const std::vector<MBone>& bones = GetAllBones();
		uint32_t size = bones.size();
		if (size > MRenderGlobal::BONES_MAX_NUMBER) size = MRenderGlobal::BONES_MAX_NUMBER;

		for (uint32_t i = 0; i < size; ++i)
		{
			(*m_pShaderBonesArray)[i] = bones[i].m_matWorldTransform;
		}

		m_pShaderParamSet->m_vParams[0]->SetDirty();

		m_bShaderParamSetDirty = false;
	}

	return m_pShaderParamSet;
}

void MSkeletonInstance::SetDirty()
{
	m_bShaderParamSetDirty = true;
}

void MSkeleton::WriteToStruct(MStruct& srt)
{
	std::vector<MBone>& vBones = m_vAllBones;

	srt.SetValue("Bones", MVariantArray());
	MVariantArray* pArray = srt.GetValue("Bones")->GetArray();

	for (uint32_t i = 0; i < vBones.size(); ++i)
	{
		MBone bone = vBones[i];
		pArray->AppendValue(MStruct());
		MStruct& boneSrt = *(*pArray)[i].GetStruct();

		bone.WriteToStruct(boneSrt);
	}
}

void MSkeleton::ReadFromStruct(const MStruct& srt)
{
	if (const MVariant* pBonesVar = srt.GetValue("Bones"))
	{
		if (const MVariantArray* pBonesArray = pBonesVar->GetArray())
		{
			uint32_t unBonesCount = pBonesArray->GetMemberCount();

			m_vAllBones.resize(unBonesCount);

			for (uint32_t i = 0; i < unBonesCount; ++i)
			{
				const MVariant& boneVar = pBonesArray->GetMember(i)->var;
				if (const MStruct* pBoneSrt = boneVar.GetStruct())
				{
					MBone& bone = m_vAllBones[i];

					bone.ReadFromStruct(*pBoneSrt);
				}
			}
		}
	}
}

bool MSkeleton::Load(const MString& strResourcePath)
{
	MString code;
	if (!MFileHelper::ReadString(strResourcePath, code))
		return false;

	MVariant var;
	MJson::JsonToMVariant(code, var);

	MStruct& srt = *var.GetStruct();
	ReadFromStruct(srt);

	return true;
}

bool MSkeleton::SaveTo(const MString& strResourcePath)
{
	MVariant var = MStruct();
	MStruct& srt = *var.GetStruct();

	WriteToStruct(srt);

	MString code;
	MJson::MVariantToJson(var, code);

	return MFileHelper::WriteString(strResourcePath, code);
}

