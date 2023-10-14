#include "Material/MShaderParam.h"
#include "Basic/MTexture.h"

MShaderConstantParam::MShaderConstantParam()
	: MShaderParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
#endif
}

MShaderConstantParam::MShaderConstantParam(const MShaderConstantParam& param)
{
	strName = param.strName;
	var = MVariant::Clone(param.var);
	eShaderType = param.eShaderType;

	SetDirty();
	
#if RENDER_GRAPHICS == MORTY_VULKAN
	unSet = param.unSet;
	unBinding = param.unBinding;
	m_VkDescriptorType = param.m_VkDescriptorType;

	m_VkBuffer = VK_NULL_HANDLE;
	m_VkBufferInfo = { VK_NULL_HANDLE, 0, 0 };
	m_VkBufferMemory = VK_NULL_HANDLE;
	m_unMemoryOffset = 0;
	m_pMemoryMapping = 0;

	m_unVkMemorySize = param.m_unVkMemorySize;
#endif


}

MShaderTextureParam::MShaderTextureParam()
	: MShaderParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
#endif
}

void MShaderTextureParam::SetTexture(std::shared_ptr<MTexture> pTexture)
{
	this->pTexture = pTexture;
	SetDirty();
}

MShaderSampleParam::MShaderSampleParam()
	: MShaderParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
#endif
}

MShaderStorageParam::MShaderStorageParam()
	: MShaderParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
#endif
}

MShaderSubpasssInputParam::MShaderSubpasssInputParam()
	: MShaderTextureParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
#endif
}
