/**
 * @File         MRenderPass
 *
 * @Created      2020-07-05 19:33:41
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSHADER_PARAM_H_
#define _M_MSHADER_PARAM_H_
#include "MGlobal.h"
#include "MVariant.h"

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanWrapper.h"
#endif

class MITexture;

enum MEShaderParamType
{
	EVertex = 1,
	EPixel = 2,
	EBoth = 3,
};

struct MORTY_CLASS MShaderParam
{
public:
	MShaderParam();
	virtual ~MShaderParam() {}

	MString strName;
	uint32_t  eShaderType;

	bool bDirty[M_BUFFER_NUM];
	void SetDirty() { memset(bDirty , true, M_BUFFER_NUM * sizeof(bool)); }

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11Buffer* pBuffer;
	uint32_t unBindPoint;
	uint32_t unBindCount;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkDescriptorType m_VkDescriptorType;
	uint32_t unSet;
	uint32_t unBinding;
#endif
};

struct MORTY_CLASS MShaderConstantParam : public MShaderParam
{
	MShaderConstantParam();

	MShaderConstantParam(const MShaderConstantParam& param, const int& unNone);

	MVariant var;

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkBuffer m_VkBuffer[M_BUFFER_NUM];
	VkDeviceMemory m_VkBufferMemory[M_BUFFER_NUM];

	uint32_t m_unMemoryOffset[M_BUFFER_NUM];
	MByte* m_pMemoryMapping[M_BUFFER_NUM];

	uint32_t m_unVkMemorySize;
#endif
};


enum METextureType
{
	ETexture2D = 1,
	ETextureCube = 2,
};

struct MShaderTextureParam : public MShaderParam
{
	MShaderTextureParam();

	MString strName;
	uint32_t unCode;
	uint32_t  eShaderType;
	MITexture* pTexture;
	METextureType eType;

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	uint32_t unBindPoint;
	uint32_t unBindCount;
#endif
};

struct MShaderSubpasssInputParam : public MShaderTextureParam
{
	MShaderSubpasssInputParam();
};

struct MShaderSampleParam : public MShaderParam
{
	MShaderSampleParam();
	MString strName;
	uint32_t unCode;
	uint32_t  eShaderType;
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	uint32_t unBindPoint;
	uint32_t unBindCount;
#endif
};

#endif