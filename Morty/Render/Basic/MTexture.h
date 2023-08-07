/**
 * @File         MTexture
 *
 * @Created      2019-09-11 16:12:31
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Render/MRenderGlobal.h"

#include "Math/Vector.h"
#include <array>

class MIDevice;

enum METextureType
{
	ETexture2D = 1,
	ETextureCube = 2,
	ETexture2DArray = 3,
};

enum class METextureLayout
{
	E_UNKNOW = 0,
	
	ER_UNORM_8,
	ERG_UNORM_8,
	ERGB_UNORM_8,
	ERGBA_UNORM_8,

	ER_FLOAT_16,
	ERG_FLOAT_16,
	ERGB_FLOAT_16,
	ERGBA_FLOAT_16,

	ER_FLOAT_32,
	ERG_FLOAT_32,
	ERGB_FLOAT_32,
	ERGBA_FLOAT_32,

	EDepth,

	E_NUM,
};

enum class METextureRenderUsage
{
	EUnknow = 0,
	ERenderBack,
	ERenderDepth,
	ERenderPresent,
};

enum class METextureShaderUsage
{
	EUnknow = 0,
	ESampler
};

class MORTY_API MTexture
{
public:
	MTexture();
	virtual ~MTexture();

public:

	void SetName(const MString& strName) { m_strTextureName = strName; }
	MString GetName() const { return m_strTextureName; }

	void SetSize(const Vector2& v2Size) { m_v2Size = v2Size; }
	Vector2 GetSize() { return m_v2Size; }

	void SetTextureLayout(const METextureLayout& eLayout) { m_eRenderType = eLayout; }
	METextureLayout GetTextureLayout() { return m_eRenderType; }

	void SetImageLayerNum(const size_t& unNum) { m_unImageLayerNum = unNum; }
	size_t GetImageLayerNum() const { return m_unImageLayerNum; }

	void SetReadable(const bool& bReadable) { m_bReadable = bReadable; }
	bool GetReadable() { return m_bReadable; }

	void SetMipmapsEnable(const bool& bEnable) { m_bMipmapsEnable = bEnable; }
	bool GetMipmapsEnable() { return m_bMipmapsEnable; }

	void SetRenderUsage(const METextureRenderUsage& usage) { m_eRenderUsage = usage; }
	METextureRenderUsage GetRenderUsage() const { return m_eRenderUsage; }

	void SetShaderUsage(const METextureShaderUsage& usage) { m_eShaderUsage = usage; }
	METextureShaderUsage GetShaderUsage() const { return m_eShaderUsage; }

	void SetTextureType(const METextureType& eType) { m_eTextureType = eType; }
	METextureType GetTextureType() const { return m_eTextureType; }


	Vector2 GetMipmapSize(const uint32_t& nMipmapLevel);

public:

	void GenerateBuffer(MIDevice* pDevice, const MByte* aImageData = nullptr);
	void DestroyBuffer(MIDevice* pDevice);

	static uint32_t GetImageMemorySize(const METextureLayout& layout);

public:

	static std::shared_ptr<MTexture> CreateShadowMap();
	static std::shared_ptr<MTexture> CreateShadowMapArray(const size_t& nArraySize);
	static std::shared_ptr<MTexture> CreateRenderTarget(METextureLayout eLayout = METextureLayout::ERGBA_UNORM_8);
	static std::shared_ptr<MTexture> CreateRenderTargetGBuffer();
	static std::shared_ptr<MTexture> CreateRenderTargetFloat32();

	static std::shared_ptr<MTexture> CreateCubeMap();

public:

	//name
	MString m_strTextureName;

	//texture size.
	Vector2 m_v2Size;

	//rgba8
	METextureLayout m_eRenderType;

	//render target
	METextureRenderUsage m_eRenderUsage;

	METextureShaderUsage m_eShaderUsage;

	METextureType m_eTextureType;

	//CPU readable
	bool m_bReadable;

	//generate mipmap
	bool m_bMipmapsEnable;

	int m_unMipmapLevel;

private:

	size_t m_unImageLayerNum;

public:

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkFormat m_VkTextureFormat;
	VkImageLayout m_VkImageLayout;
	VkImage m_VkTextureImage;
	VkDeviceMemory m_VkTextureImageMemory;
	VkImageView m_VkImageView;
	VkSampler m_VkSampler;
#endif

};
