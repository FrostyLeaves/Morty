#include "MShaderParam.h"
#include "MTexture.h"

MShaderParam::MShaderParam()
	: strName()
	, eShaderType(0)
#if RENDER_GRAPHICS == MORTY_VULKAN
	, unSet(0)
	, unBinding(0)
#endif
{
	SetDirty();
}

MShaderConstantParam::MShaderConstantParam()
	: MShaderParam()
	, var()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

	m_VkBuffer = VK_NULL_HANDLE;
	m_VkBufferMemory = VK_NULL_HANDLE;
	m_unMemoryOffset = 0;
	m_pMemoryMapping = 0;
	m_unVkMemorySize = 0;
#endif
}

MShaderConstantParam::MShaderConstantParam(const MShaderConstantParam& param, const int& unNone)
{
	strName = param.strName;
	var = param.var;
	eShaderType = param.eShaderType;

	SetDirty();
	
#if RENDER_GRAPHICS == MORTY_VULKAN
	unSet = param.unSet;
	unBinding = param.unBinding;
	m_VkDescriptorType = param.m_VkDescriptorType;

	m_VkBuffer = VK_NULL_HANDLE;
	m_VkBufferMemory = VK_NULL_HANDLE;
	m_unMemoryOffset = 0;
	m_pMemoryMapping = 0;

	m_unVkMemorySize = param.m_unVkMemorySize;
#endif


}

MShaderTextureParam::MShaderTextureParam()
	: MShaderParam()
	, pTexture(nullptr)
	, pImageIdent(nullptr)
	, eType(METextureType::ETexture2D)

{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
#endif
}

MShaderSampleParam::MShaderSampleParam()
	: MShaderParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
#endif
}

MShaderSubpasssInputParam::MShaderSubpasssInputParam()
	: MShaderTextureParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
#endif
}
