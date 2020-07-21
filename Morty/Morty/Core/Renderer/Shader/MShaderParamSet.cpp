#include "MShaderParamSet.h"
#include "MIDevice.h"

MShaderParamSet::MShaderParamSet()
	: m_vParams()
	, m_vTextures()
	, m_vSamples()
	, m_unKey(0)
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
	, m_VkDescriptorSet(VK_NULL_HANDLE)
#endif
{

}

MShaderParamSet::MShaderParamSet(const uint32_t& unKey)
	: m_vParams()
	, m_vTextures()
	, m_vSamples()
	, m_unKey(unKey)
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
	, m_VkDescriptorSet(VK_NULL_HANDLE)
#endif
{

}

MShaderParamSet::~MShaderParamSet()
{

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
}
