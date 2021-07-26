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
	MORTY_CLASS(MTextureResource)
    MTextureResource();
    virtual ~MTextureResource();

	MTexture* GetTextureTemplate(){ return &m_texture; }

	static MString GetResourceTypeName() { return "Texture"; }
	static std::vector<MString> GetSuffixList() { return { "png", "jpg", "tga" }; }

public:

	void LoadFromMemory(MByte* aByteData, const uint32_t& nWidth, const uint32_t& nHeight, const int& format = 32);

public:

	virtual void OnDelete() override;

protected:

	virtual bool Load(const MString& strResourcePath) override;
	virtual bool SaveTo(const MString& strResourcePath) override;
private:


	MTexture m_texture;

	MByte* m_aByteData;
};


#endif
