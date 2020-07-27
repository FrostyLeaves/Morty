#include "MShaderParam.h"

MShaderParam::MShaderParam()
	: strName()
	, unCode(SHADER_PARAM_CODE_DEFAULT)
	, eShaderType(0)
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	, pBuffer(nullptr)
	, unBindPoint(0)
	, unBindCount(0)
#elif RENDER_GRAPHICS == MORTY_VULKAN
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
	m_VkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	memset(m_VkBuffer, VK_NULL_HANDLE, sizeof(VkBuffer) * M_BUFFER_NUM);
	memset(m_VkBufferMemory, VK_NULL_HANDLE, sizeof(VkDeviceMemory) * M_BUFFER_NUM);
	memset(m_unMemoryOffset, 0, sizeof(uint32_t) * M_BUFFER_NUM);
	memset(m_pMemoryMapping, 0, sizeof(MByte) * M_BUFFER_NUM);

	m_unVkMemorySize = 0;
#endif

}

MShaderConstantParam::MShaderConstantParam(const MShaderConstantParam& param, const int& unNone)
{
	strName = param.strName;
	unCode = param.unCode;
	var = param.var;
	eShaderType = param.eShaderType;

	SetDirty();
	
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	pBuffer = nullptr;
	unBindPoint = param.unBindPoint;
	unBindCount = param.unBindCount;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	unSet = param.unSet;
	unBinding = param.unBinding;
	m_VkDescriptorType = param.m_VkDescriptorType;

	memset(m_VkBuffer, VK_NULL_HANDLE, sizeof(VkBuffer) * M_BUFFER_NUM);
	memset(m_VkBufferMemory, VK_NULL_HANDLE, sizeof(VkDeviceMemory) * M_BUFFER_NUM);
	memset(m_unMemoryOffset ,0, sizeof(uint32_t) * M_BUFFER_NUM);
	memset(m_pMemoryMapping, 0, sizeof(MByte) * M_BUFFER_NUM);

	m_unVkMemorySize = param.m_unVkMemorySize;
#endif


}

MShaderTextureParam::MShaderTextureParam()
	: MShaderParam()
	, pTexture(nullptr)
	, eType(METextureType::ETexture2D)

{

}

MShaderSampleParam::MShaderSampleParam()
	: MShaderParam()
{

}

