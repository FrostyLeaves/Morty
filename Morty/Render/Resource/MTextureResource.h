/**
 * @File         MTextureResource
 * 
 * @Created      2019-08-29 16:35:04
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Resource/MResource.h"

#include "Basic/MTexture.h"
#include "Resource/MResourceLoader.h"

enum class MTexturePixelFormat
{
	Unknow,
	Byte8,
	Float32,
};

struct MTextureImportInfo
{
	MTextureImportInfo();
	MTextureImportInfo(MTexturePixelFormat pixelFormat);

	MTexturePixelFormat ePixelFormat;
};

struct MORTY_API MTextureResourceData : public MFbResourceData
{
public:
	size_t nWidth = 0;
	size_t nHeight = 0;
	size_t nImageLayerNum = 1;
	METextureType eTextureType = METextureType::ETexture2D;
	MTexturePixelFormat ePixelFormat = MTexturePixelFormat::Unknow;
	std::vector<MByte> aByteData{};
	size_t nChannel = 0;
	MString strTextureName = "";

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const override;
	void Deserialize(const void* pBufferPointer) override;
};

class MORTY_API MTextureResource : public MResource
{
public:
	MORTY_CLASS(MTextureResource)
    MTextureResource();
    virtual ~MTextureResource();

	std::shared_ptr<MTexture> GetTextureTemplate(){ return m_pTexture; }

public:

	METextureLayout GetTextureLayout() const;
	MTexturePixelFormat GetPixelFormat() const;
	size_t GetChannel() const;
	size_t GetWidth() const;
	size_t GetHeight() const;
	const MByte* GetRawData() const;
	bool GetReadable() const { return m_bReadable; }
public:

	void CreateCubeMapRenderTarget(const uint32_t& nWidth, const uint32_t& nHeight, uint32_t nChannel, const METextureLayout& eLayout, const bool& bMipmapEnable);

	void OnDelete() override;

	bool Load(std::unique_ptr<MResourceData>&& pResourceData) override;
	bool SaveTo(std::unique_ptr<MResourceData>& pResourceData) override;

protected:

	METextureLayout GetTextureLayout(const uint32_t& nChannel, const MTexturePixelFormat& format);

	bool m_bReadable = false;
	std::shared_ptr<MTexture> m_pTexture = nullptr;
	std::unique_ptr<MResourceData> m_pResourceData = nullptr;
};


class MORTY_API MReadableTextureResource : public MTextureResource
{
public:
	MReadableTextureResource() { m_bReadable = true; }
};

class MORTY_API MTextureResourceLoader : public MResourceLoader
{
public:

	static MString GetResourceTypeName() { return "Texture"; }
	static std::vector<MString> GetSuffixList() { return { "png", "jpg", "tga", "hdr", "tif", "mtex" }; }

	const MType* ResourceType() const override;
	std::unique_ptr<MResourceData> LoadResource(const MString& svFullPath) override;
};
