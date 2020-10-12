#include "MShaderParamSet.h"
#include "MIDevice.h"
#if RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanDevice.h"
#endif

MShaderParamSet::MShaderParamSet()
	: m_vParams()
	, m_vTextures()
	, m_vSamples()
	, m_unKey(0)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	memset(m_VkDescriptorSet, VK_NULL_HANDLE, sizeof(VkDescriptorSet) * M_BUFFER_NUM);
	m_nDescriptorSetInitMaterialIdx = M_INVALID_INDEX;
#endif
}

MShaderParamSet::MShaderParamSet(const uint32_t& unKey)
	: m_vParams()
	, m_vTextures()
	, m_vSamples()
	, m_unKey(unKey)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	memset(m_VkDescriptorSet, VK_NULL_HANDLE, sizeof(VkDescriptorSet) * M_BUFFER_NUM);
	m_nDescriptorSetInitMaterialIdx = M_INVALID_INDEX;
#endif
}

MShaderParamSet::~MShaderParamSet()
{

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

void MShaderParamSet::ClearAndDestroy(MIDevice* pDevice)
{
	for (MShaderConstantParam* pParam : m_vParams)
	{
		pDevice->DestroyShaderParamBuffer(pParam);
		delete pParam;
	}

	for (MShaderTextureParam* pParam : m_vTextures)
	{
		delete pParam;
	}

	for (MShaderSampleParam* pParam : m_vSamples)
	{
		delete pParam;
	}

	m_vParams.clear();
	m_vTextures.clear();
	m_vSamples.clear();

#if RENDER_GRAPHICS == MORTY_VULKAN
	MVulkanDevice* pVkDevice = dynamic_cast<MVulkanDevice*>(pDevice);

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
// 		if (m_VkDescriptorSet[i])
// 			pVkDevice->m_ObjectDestructor.DestroyDescriptorSetLater(m_VkDescriptorSet[i]);
	}

	memset(m_VkDescriptorSet, VK_NULL_HANDLE, sizeof(VkDescriptorSet) * M_BUFFER_NUM);
	m_nDescriptorSetInitMaterialIdx = M_INVALID_INDEX;
#endif
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
