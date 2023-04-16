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

class MORTY_API MTextureResource : public MResource
{
public:
	enum class PixelFormat
	{
		Unknow,
		Byte8,
		Float32,
	};

	struct ImportInfo
	{
		ImportInfo();
		ImportInfo(PixelFormat pixelFormat);

		PixelFormat ePixelFormat;
	};

public:
	MORTY_CLASS(MTextureResource)
    MTextureResource();
    virtual ~MTextureResource();

	std::shared_ptr<MTexture> GetTextureTemplate(){ return m_pTexture; }

	static MString GetResourceTypeName() { return "Texture"; }
	static std::vector<MString> GetSuffixList() { return { "png", "jpg", "tga", "hdr", "tif", "mtex"}; }

public:

	METextureLayout GetTextureLayout() const;
	PixelFormat GetPixelFormat() const { return m_ePixelFormat; }
	size_t GetChannel() const { return m_nChannel; }
	size_t GetWidth() const;
	size_t GetHeight() const;
	const std::vector<MByte>& GetRawData() const { return m_aByteData; }
	
public:

	bool ImportTexture(const MString&  strResourcePath, const ImportInfo& importInfo);
	bool ImportTextureFromMemory(char* buffer, size_t nSize, const ImportInfo& importInfo);

	bool ImportCubeMap(const std::array<MString, 6>& vResourcePath, const ImportInfo& importInfo);
	
	void LoadFromMemory(const MByte* aByteData, const uint32_t& nWidth, const uint32_t& nHeight, uint32_t nChannel, PixelFormat ePixelFormat = PixelFormat::Byte8);

	void CreateCubeMapRenderTarget(const uint32_t& nWidth, const uint32_t& nHeight, uint32_t nChannel, const METextureLayout& eLayout, const bool& bMipmapEnable);

public:

	virtual void OnDelete() override;

protected:

	METextureLayout GetTextureLayout(const uint32_t& nChannel, const PixelFormat& format);

protected:

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(const void* pBufferPointer);

	virtual bool Load(const MString& strResourcePath) override;
	virtual bool SaveTo(const MString& strResourcePath) override;
private:
	
	std::shared_ptr<MTexture> m_pTexture;


	//RawData
	std::vector<MByte> m_aByteData;
	PixelFormat m_ePixelFormat = PixelFormat::Unknow;
	size_t m_nChannel = 0;
};


#endif
