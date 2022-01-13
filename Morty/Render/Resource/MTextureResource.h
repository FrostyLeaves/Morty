/**
 * @File         MTextureResource
 * 
 * @Created      2019-08-29 16:35:04
 *
 * @Author       DoubleYe
**/

#ifndef _M_MTEXTURERESOURCE_H_
#define _M_MTEXTURERESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"

#include "MTexture.h"

class MORTY_API MTextureResource : public MResource
{
public:
	enum class PixelFormat
	{
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

	MTexture* GetTextureTemplate(){ return &m_texture; }

	static MString GetResourceTypeName() { return "Texture"; }
	static std::vector<MString> GetSuffixList() { return { "png", "jpg", "tga", "hdr", "tif"}; }

public:

	bool ImportTexture(const MString&  strResourcePath, const ImportInfo& importInfo);

	bool ImportCubeMap(const std::array<MString, 6>& vResourcePath, const ImportInfo& importInfo);
	
	void LoadFromMemory(MByte* aByteData, const uint32_t& nWidth, const uint32_t& nHeight, uint32_t nChannel, PixelFormat ePixelFormat = PixelFormat::Byte8, bool bCopyMemory = true);

public:

	virtual void OnDelete() override;

protected:

	METextureLayout GetTextureLayout(const uint32_t& nChannel, const PixelFormat& format);

protected:

	virtual bool Load(const MString& strResourcePath) override;
	virtual bool SaveTo(const MString& strResourcePath) override;
private:

	MTexture m_texture;

	MByte* m_aByteData;
};


#endif
