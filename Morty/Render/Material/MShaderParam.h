/**
 * @File         MRenderPass
 *
 * @Created      2020-07-05 19:33:41
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSHADER_PARAM_H_
#define _M_MSHADER_PARAM_H_
#include "Utility/MGlobal.h"
#include "Utility/MVariant.h"
#include "Basic/MTexture.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanWrapper.h"
#endif


enum MEShaderParamType
{
	EVertex = 1,
	EPixel = 2,
	EBoth = 3,
};

enum class MESamplerType
{
	ENearest,
	ELinear,
};

struct MORTY_API MShaderParam
{
public:
	MShaderParam();
	virtual ~MShaderParam() {}

	MString strName;
	uint32_t  eShaderType;

	bool bDirty;
	void SetDirty() { bDirty = true; }

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkDescriptorType m_VkDescriptorType;
	uint32_t unSet;
	uint32_t unBinding;
#endif
};

struct MORTY_API MShaderConstantParam : public MShaderParam
{
	MShaderConstantParam();

	MShaderConstantParam(const MShaderConstantParam& param, const int& unNone);

	MVariant var;


#if RENDER_GRAPHICS == MORTY_VULKAN
	VkBuffer m_VkBuffer;
	VkDeviceMemory m_VkBufferMemory;
	uint32_t m_unMemoryOffset;
	MByte* m_pMemoryMapping;
	uint32_t m_unVkMemorySize;
#endif
};


struct MShaderTextureParam : public MShaderParam
{
	MShaderTextureParam();

	MTexture* pTexture;
	void* pImageIdent;
	METextureType eType;

public:
	void SetTexture(MTexture* pTexture);
};

struct MShaderSubpasssInputParam : public MShaderTextureParam
{
	MShaderSubpasssInputParam();
};

struct MShaderSampleParam : public MShaderParam
{
	MShaderSampleParam();

	MESamplerType eSamplerType;


#if RENDER_GRAPHICS == MORTY_VULKAN
	VkSampler m_VkSampler;
#endif
	

};

#endif