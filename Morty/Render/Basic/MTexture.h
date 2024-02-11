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

MORTY_SPACE_BEGIN

class MIDevice;

enum METextureType
{
	ETexture2D = 1,
	ETextureCube = 2,
	ETexture2DArray = 3,
	ETexture3D = 4,
};

enum class METextureLayout
{
	Unknow = 0,
	Depth,

	UNorm_R8,
	UNorm_RG8,
	UNorm_RGB8,
	UNorm_RGBA8,

	UInt_R8,

	Float_R16,
	Float_RG16,
	Float_RGB16,
	Float_RGBA16,

	Float_R32,
	Float_RG32,
	Float_RGB32,
	Float_RGBA32,

	UNorm_RGBA8_ASTC4x4,
	UNorm_RGBA8_ASTC8x8,

	UNorm_RGBA8_BC1,
	UNorm_RGBA8_BC2,
	UNorm_RGBA8_BC3,
	UNorm_RGBA8_BC4,
	UNorm_RGBA8_BC5,
	UNorm_RGBA8_BC7,

	SNorm_RGBA8_BC4,
	SNorm_RGBA8_BC5,
};

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
	uint32_t nShaderUsage = METextureReadUsage::EUnknow;
	bool bReadable = false;
	bool bMipmapEnable = false;

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

	void SetMipmapsEnable(const bool& bEnable) { m_bMipmapsEnable = bEnable; }
	bool GetMipmapsEnable() { return m_bMipmapsEnable; }

	void SetRenderUsage(const METextureWriteUsage& usage) { m_eRenderUsage = usage; }
	METextureWriteUsage GetRenderUsage() const { return m_eRenderUsage; }

	void SetShaderUsage(const uint32_t& usage) { m_eShaderUsage = usage; }
	uint32_t GetShaderUsage() const { return m_eShaderUsage; }

	void SetTextureType(const METextureType& eType) { m_eTextureType = eType; }
	METextureType GetTextureType() const { return m_eTextureType; }


	Vector2 GetMipmapSize(const uint32_t& nMipmapLevel);

public:

	void GenerateBuffer(MIDevice* pDevice);
	void GenerateBuffer(MIDevice* pDevice, const MSpan<MByte>& buffer);
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
	Vector3i m_n3Size;

	//rgba8
	METextureLayout m_eRenderType;

	//render target
	METextureWriteUsage m_eRenderUsage;

	uint32_t m_eShaderUsage;

	METextureType m_eTextureType;

	//CPU readable
	bool m_bReadable;

	//generate mipmap
	bool m_bMipmapsEnable;

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