﻿/**
 * @File         MTexture
 *
 * @Created      2019-09-11 16:12:31
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Render/MRenderGlobal.h"
#include "Math/Vector.h"
#include "Flatbuffer/MTexture_generated.h"

MORTY_SPACE_BEGIN

class MIDevice;

enum METextureType
{
	ETexture2D = 1,
	ETextureCube = 2,
	ETexture2DArray = 3,
	ETexture3D = 4,
};

using METextureLayout = morty::fbs::METextureLayout;

using MEMipmapDataType = morty::fbs::MEMipmapDataType;

enum class METextureWriteUsage
{
	EUnknow = 0,
	ERenderBack,
	ERenderDepth,
	ERenderPresent,
	EStorageWrite,
};

class METextureReadUsage
{
public:
	static constexpr uint32_t EUnknow = 0;
	static constexpr uint32_t EPixelSampler = 1;
	static constexpr uint32_t EStorageRead = 2;
	static constexpr uint32_t EShadingRateMask = 4;
};

struct MORTY_API MTextureDesc
{
	MString strTextureName = "Default";
	Vector3i n3Size = Vector3i(1, 1, 1);
	uint32_t nLayer = 1;
	METextureType eTextureType = METextureType::ETexture2D;
	METextureLayout eTextureLayout = METextureLayout::UNorm_RGBA8;
	METextureWriteUsage eWriteUsage = METextureWriteUsage::EUnknow;
	MEMipmapDataType eMipmapDataType = MEMipmapDataType::Disable;
	uint32_t nShaderUsage = METextureReadUsage::EUnknow;
	bool bReadable = false;

	MTextureDesc& InitName(const MString& name)
	{
		strTextureName = name;
		return *this;
	}
	MTextureDesc& InitSize(const Vector2i& n2Size)
	{
		n3Size = Vector3i(n2Size.x, n2Size.y, 1);
		return *this;
	}
};

class MORTY_API MTexture
{
public:
	MTexture();
	virtual ~MTexture();

public:

	void SetName(const MString& strName) { m_strTextureName = strName; }
	MString GetName() const { return m_strTextureName; }

	void SetSize(const Vector2i& n2Size) { m_n3Size = Vector3i(n2Size.x, n2Size.y, 1); }
	void SetSize(const Vector3i& n3Size) { m_n3Size = n3Size; }
	Vector3i GetSize() const { return m_n3Size; }
	Vector2i GetSize2D() const { return Vector2i(m_n3Size.x, m_n3Size.y); }

	void SetLayer(const uint32_t& nLayer) { m_nLayer = nLayer; }
	uint32_t GetLayer() const { return m_nLayer; }

	void SetTextureLayout(const METextureLayout& eLayout) { m_eRenderType = eLayout; }
	METextureLayout GetTextureLayout() { return m_eRenderType; }

	void SetReadable(const bool& bReadable) { m_bReadable = bReadable; }
	bool GetReadable() { return m_bReadable; }

	void SetMipmapDataType(const MEMipmapDataType& eMipmap) { m_eMipmapType = eMipmap; }
	MEMipmapDataType GetMipmapDataType() const { return m_eMipmapType; }

	void SetRenderUsage(const METextureWriteUsage& usage) { m_eRenderUsage = usage; }
	METextureWriteUsage GetRenderUsage() const { return m_eRenderUsage; }

	void SetShaderUsage(const uint32_t& usage) { m_eShaderUsage = usage; }
	uint32_t GetShaderUsage() const { return m_eShaderUsage; }

	void SetTextureType(const METextureType& eType) { m_eTextureType = eType; }
	METextureType GetTextureType() const { return m_eTextureType; }


	Vector2 GetMipmapSize(const uint32_t& nMipmapLevel);

public:

	void GenerateBuffer(MIDevice* pDevice);
	void GenerateBuffer(MIDevice* pDevice, const std::vector<std::vector<MByte>>& buffer);
	void DestroyBuffer(MIDevice* pDevice);

	static uint32_t GetImageMemorySize(const METextureLayout& layout);

public:

	static std::shared_ptr<MTexture> CreateTexture(const MTextureDesc& desc);

	static MTextureDesc CreateDepthBuffer();
	static MTextureDesc CreateShadowMapArray(const int& nSize, const uint32_t& nArraySize);
	static MTextureDesc CreateRenderTarget(METextureLayout eLayout = METextureLayout::UNorm_RGBA8);
	static MTextureDesc CreateRenderTargetGBuffer();
	static std::shared_ptr<MTexture> CreateRenderTargetFloat32();
	static MTextureDesc CreateShadingRate();

	static std::shared_ptr<MTexture> CreateVXGIMap();

public:

	//name
	MString m_strTextureName;

	//texture size.
	Vector3i m_n3Size = Vector3i(1, 1, 1);

	//rgba8
	METextureLayout m_eRenderType = METextureLayout::UNorm_RGBA8;

	//render target
	METextureWriteUsage m_eRenderUsage = METextureWriteUsage::EUnknow;

	uint32_t m_eShaderUsage = METextureReadUsage::EUnknow;

	METextureType m_eTextureType = METextureType::ETexture2D;

	//CPU readable
	bool m_bReadable = false;

	//generate mipmap
	MEMipmapDataType m_eMipmapType = MEMipmapDataType::Disable;

	uint32_t m_unMipmapLevel = 1;
	uint32_t m_nLayer = 1;

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkFormat m_VkTextureFormat;
	VkImageLayout m_VkImageLayout;
	VkImage m_VkTextureImage;
	VkDeviceMemory m_VkTextureImageMemory;
	VkImageView m_VkImageView;
	VkSampler m_VkSampler;
#endif

};

MORTY_SPACE_END