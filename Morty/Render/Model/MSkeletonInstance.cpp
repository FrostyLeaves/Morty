#include "Model/MSkeletonInstance.h"
#include <algorithm>

#include "Engine/MEngine.h"
#include "Flatbuffer/MBone_generated.h"
#include "Flatbuffer/MSkeleton_generated.h"
#include "Utility/MFileHelper.h"
#include "System/MRenderSystem.h"
#include "Material/MShaderParamSet.h"

#include "Resource/MSkeletonResource.h"

MORTY_CLASS_IMPLEMENT(MSkeletonInstance, MObject)

void MSkeletonInstance::SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeletonRsource)
{
	m_skeletonResource = pSkeletonRsource;

	if (auto pResource = m_skeletonResource.GetResource<MSkeletonResource>())
	{
		m_pSkeletonTemplate = pResource->GetSkeleton();
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

void MSkeletonInstance::OnCreated()
{
	m_pShaderPropertyBlock = MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_SKELETON);
}

void MSkeletonInstance::OnDelete()
{
	m_pShaderBonesArray = nullptr;

	if (m_pShaderPropertyBlock)
	{
		const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
		m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
		m_pShaderPropertyBlock = nullptr;
	}
}
