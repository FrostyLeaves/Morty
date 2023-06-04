/**
 * @File         MTextureResource
 * 
 * @Created      2019-08-29 16:35:04
 *
 * @Author       DoubleYe
**/

#ifndef _M_MTEXTURERESOURCE_H_
#define _M_MTEXTURERESOURCE_H_
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
	
public:

	void CreateCubeMapRenderTarget(const uint32_t& nWidth, const uint32_t& nHeight, uint32_t nChannel, const METextureLayout& eLayout, const bool& bMipmapEnable);

	virtual void OnDelete() override;

	bool Load(std::unique_ptr<MResourceData>& pResourceData) override;
	virtual bool SaveTo(std::unique_ptr<MResourceData>& pResourceData) override;

protected:

	METextureLayout GetTextureLayout(const uint32_t& nChannel, const MTexturePixelFormat& format);

private:

	std::shared_ptr<MTexture> m_pTexture = nullptr;
	std::unique_ptr<MResourceData> m_pResourceData = nullptr;
};

class MORTY_API MTextureResourceLoader : public MResourceLoader
{
public:

	static MString GetResourceTypeName() { return "Texture"; }
	static std::vector<MString> GetSuffixList() { return { "png", "jpg", "tga", "hdr", "tif", "mtex" }; }

	std::shared_ptr<MResource> Create(MResourceSystem* pManager) override;
	std::unique_ptr<MResourceData> LoadResource(const MString& svFullPath, const MString& svPath) override;
};

#endif
