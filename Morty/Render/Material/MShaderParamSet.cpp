#include "Material/MShaderParamSet.h"
#include "Render/MIDevice.h"
#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanDevice.h"
#endif

MShaderParamSet::MShaderParamSet()
	: m_vParams()
	, m_vTextures()
	, m_vSamples()
	, m_unKey(0)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorSet = VK_NULL_HANDLE;
	m_nDescriptorSetInitMaterialIdx = MGlobal::M_INVALID_INDEX;
	m_unLayoutDataIdx = MGlobal::M_INVALID_INDEX;
#endif
}

MShaderParamSet::MShaderParamSet(const uint32_t& unKey)
	: m_vParams()
	, m_vTextures()
	, m_vSamples()
	, m_unKey(unKey)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorSet = VK_NULL_HANDLE;
	m_nDescriptorSetInitMaterialIdx = MGlobal::M_INVALID_INDEX;
	m_unLayoutDataIdx = MGlobal::M_INVALID_INDEX;
#endif
}

MShaderParamSet::~MShaderParamSet()
{
	for (MShaderConstantParam* pParam : m_vParams)
		delete pParam;

	for (MShaderTextureParam* pParam : m_vTextures)
		delete pParam;

	for (MShaderSampleParam* pParam : m_vSamples)
		delete pParam;

	m_vParams.clear();
	m_vTextures.clear();
	m_vSamples.clear();
}

MShaderConstantParam* MShaderParamSet::FindConstantParam(const MString& strParamName)
{
	for (MShaderConstantParam* pParam : m_vParams)
	{
		if (pParam->strName == strParamName)
			return pParam;
	}

	return nullptr;
}

MShaderStorageParam* MShaderParamSet::FindStorageParam(const MString& strParamName)
{
	for (MShaderStorageParam* pParam : m_vStorages)
	{
		if (pParam->strName == strParamName)
			return pParam;
	}

	return nullptr;
}

MVariant* MShaderParamSet::FindValue(const MString& strName)
{
	for (MShaderConstantParam* pParam : m_vParams)
	{
		if (pParam->strName == strName)
			return &pParam->var;
		else if (MVariant* pVariant = FindValue(strName, pParam->var))
			return pVariant;
	}

	return nullptr;
}

MVariant* MShaderParamSet::FindValue(const MString& strName, MVariant& value)
{
	if (MStruct* pStruct = value.GetStruct())
	{
		for (size_t i = 0; i < pStruct->GetMemberCount(); ++i)
		{
			if (MStruct::MContainer::MStructMember* pContainer = pStruct->GetMember(i))
			{
				if (pContainer->strName == strName)
					return &pContainer->var;
				else if (MVariant* pVariant = FindValue(strName, pContainer->var))
					return pVariant;
			}
		}
	}

	return nullptr;
}

bool MShaderParamSet::SetValue(const MString& strName, MTexture* pTexture)
{
	for (MShaderTextureParam* pParam : m_vTextures)
	{
		if (pParam->strName == strName)
		{
			pParam->SetTexture(pTexture);
			return true;
		}
	}

	return false;
}

bool MShaderParamSet::SetValue(const MString& strName, const MVariant& value)
{
	for (MShaderConstantParam* pParam : m_vParams)
	{
		if (pParam->strName == strName)
		{
			if (SetValue(pParam->var, value))
			{
				pParam->SetDirty();
				return true;
			}
		}
		else if (MVariant* pVariant = FindValue(strName, pParam->var))
		{
			if (SetValue(*pVariant, value))
			{
				pParam->SetDirty();
				return true;
			}
		}
	}

	return false;
}

bool MShaderParamSet::SetValue(MVariant& target, const MVariant& source)
{
	if (target.GetType() == source.GetType() && target.GetSize() == source.GetSize())
	{
		target = source;
		return true;
	}

	return false;
}

bool MShaderParamSet::HasValue(const uint32_t& unBinding, const uint32_t& unSet)
{
	for (MShaderParam* pParam : m_vParams)
	{
		if (pParam->unSet == unSet && pParam->unBinding == unBinding)
			return true;
	}

	for (MShaderParam* pParam : m_vTextures)
	{
		if (pParam->unSet == unSet && pParam->unBinding == unBinding)
			return true;
	}

	for (MShaderParam* pParam : m_vSamples)
	{
		if (pParam->unSet == unSet && pParam->unBinding == unBinding)
			return true;
	}

	return false;
}

void MShaderParamSet::GenerateBuffer(MIDevice* pDevice)
{
	pDevice->GenerateShaderParamSet(this);
}

void MShaderParamSet::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyShaderParamSet(this);
	m_nDescriptorSetInitMaterialIdx = MGlobal::M_INVALID_INDEX;

	for (MShaderConstantParam* pParam : m_vParams)
		pDevice->DestroyShaderParamBuffer(pParam);
}

MShaderParamSet* MShaderParamSet::Clone()
{
	MShaderParamSet* pParamSet = new MShaderParamSet(m_unKey);

	pParamSet->m_vParams.resize(m_vParams.size());
	pParamSet->m_vTextures.resize(m_vTextures.size());
	pParamSet->m_vSamples.resize(m_vSamples.size());

	for (uint32_t i = 0; i < m_vParams.size(); ++i)
		pParamSet->m_vParams[i] = new MShaderConstantParam(*m_vParams[i], 0);

	for (uint32_t i = 0; i < m_vTextures.size(); ++i)
		pParamSet->m_vTextures[i] = new MShaderTextureParam(*m_vTextures[i]);

	for (uint32_t i = 0; i < m_vSamples.size(); ++i)
		pParamSet->m_vSamples[i] = new MShaderSampleParam(*m_vSamples[i]);

	return pParamSet;
}
