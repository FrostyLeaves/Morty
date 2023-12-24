/**
 * @File         MRenderPass
 *
 * @Created      2020-07-05 19:33:41
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Variant/MVariant.h"
#include "Basic/MTexture.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanWrapper.h"
#endif

enum class MESamplerType
{
	ENearest,
	ELinear,
};

class MBuffer;

struct MORTY_API MShaderParam
{
public:
	MShaderParam() = default;
	virtual ~MShaderParam() = default;

	MStringId strName;
	uint32_t  eShaderType = 0;

	bool bDirty = true;
	void SetDirty() { bDirty = true; }

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkDescriptorType m_VkDescriptorType;
	uint32_t unSet = 0;
	uint32_t unBinding = 0;
#endif
};

struct MORTY_API MShaderConstantParam : public MShaderParam
{
	MShaderConstantParam();
	MShaderConstantParam(const MShaderConstantParam& param);

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkBuffer m_VkBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_VkBufferMemory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo m_VkBufferInfo = { VK_NULL_HANDLE, 0, 0 };
	uint32_t m_unMemoryOffset = 0;
	MByte* m_pMemoryMapping = 0;
	uint32_t m_unVkMemorySize = 0;
#endif

	MVariant var;
};


struct MShaderTextureParam : public MShaderParam
{
	MShaderTextureParam();

public:
	virtual void SetTexture(std::shared_ptr<MTexture> pTexture);
	virtual std::shared_ptr<MTexture> GetTexture() { return pTexture; }

	virtual std::shared_ptr<MShaderTextureParam> Clone() const;

public:
	std::shared_ptr<MTexture> pTexture = nullptr;
	void* pImageIdent = nullptr;
	METextureType eType = METextureType::ETexture2D;

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkDescriptorImageInfo m_VkImageInfo = {};
#endif
};

struct MShaderStorageParam : public MShaderParam
{
	MShaderStorageParam();

public:
	const MBuffer* pBuffer = nullptr;
	bool bWritable = false;
	void* pImageIdent = nullptr;

	void SetBuffer(const MBuffer* buf) { pBuffer = buf; }
#if RENDER_GRAPHICS == MORTY_VULKAN
	VkDescriptorBufferInfo m_VkBufferInfo = { VK_NULL_HANDLE, 0, 0 };
#endif

};

struct MShaderSubpasssInputParam : public MShaderTextureParam
{
	MShaderSubpasssInputParam();
};

struct MShaderSampleParam : public MShaderParam
{
	MShaderSampleParam();

	MESamplerType eSamplerType = MESamplerType::ELinear;

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkSampler m_VkSampler = VK_NULL_HANDLE;
#endif
	
};
