/**
 * @File         MTexture
 * 
 * @Created      2019-09-11 16:12:31
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTEXTURE_H_
#define _M_MTEXTURE_H_
#include "MGlobal.h"
#include "Vector.h"
#include "Type/MColor.h"
#include "MRenderStructure.h"

class MIDevice;
class MTextureBuffer;
class MRenderTextureBuffer;
class MDepthTextureBuffer;
class MORTY_CLASS MITexture
{
public:
	MITexture(){}
	virtual ~MITexture(){}

public:


	uint32_t static GetImageMemorySize(const METextureLayout& layout);

	virtual Vector2 GetSize() = 0;
	virtual METextureLayout GetType() = 0;
	virtual MTextureBuffer* GetBuffer() = 0;
	virtual bool GetReadable() = 0;
	virtual unsigned char* GetImageData() { return nullptr; }
};

class MORTY_CLASS MTexture : public MITexture
{
public:
    MTexture();
    virtual ~MTexture();

public:

	void SetSize(const Vector2& v2Size);
	virtual Vector2 GetSize() override { return m_v2Size; }
	void SetType(const METextureLayout& eLayout) { m_eRenderType = eLayout; }
	virtual METextureLayout GetType() override { return m_eRenderType; }

	void SetReadable(const bool& bReadable) { m_bReadable = bReadable; }
	virtual bool GetReadable() override { return m_bReadable; }

	void SetMipmapsEnable(const bool& bEnable) { m_bMipmapsEnable = bEnable; }
	bool GetMipmapsEnable() { return m_bMipmapsEnable; }

	virtual unsigned char* GetImageData() override { return m_pImageData; }

	void FillColor(const MColor& color);

	virtual void GenerateBuffer(MIDevice* pDevice);
	virtual void DestroyBuffer(MIDevice* pDevice);

	virtual MTextureBuffer* GetBuffer() override { return m_pTextureBuffer; }

private:

	unsigned char* m_pImageData;
	Vector2 m_v2Size;

	uint32_t m_unImageDataArraySize;

	MTextureBuffer* m_pTextureBuffer;
	METextureLayout m_eRenderType;
	bool m_bReadable;
	bool m_bMipmapsEnable;

};

class MORTY_CLASS MTextureCube : public MITexture
{
public:
	MTextureCube();
	virtual ~MTextureCube();

	enum MECubeFace
	{
		ERight = 0,
		ELeft = 1,
		ETop = 2,
		EBottom = 3,
		EFront = 4,
		EBack = 5,
	};

public:

	virtual Vector2 GetSize() override { return m_v2Size; }
	virtual METextureLayout GetType() override { return METextureLayout::ERGBA8; }

	virtual bool GetReadable() override { return false; }

	virtual void GenerateBuffer(MIDevice* pDevice, const bool& bMipmap = false);
	virtual void DestroyTexture(MIDevice* pDevice);

	virtual MTextureBuffer* GetBuffer() override { return m_pTextureBuffer; }

	void SetTextures(MTexture* vTexture[6]);
	void SetTexture(MTexture* pTexture, const MECubeFace& eFace);

private:
	Vector2 m_v2Size;
	MTexture* m_vTexture[6];
	MTextureBuffer* m_pTextureBuffer;
};

class MORTY_CLASS MIRenderTexture : public MITexture
{
};

class MORTY_CLASS MIRenderBackTexture : public MIRenderTexture
{
public:
	MIRenderBackTexture();
	virtual ~MIRenderBackTexture() {}
	//Should Set before GenerateBuffer

	void SetType(const METextureLayout& eType) { m_eRenderType = eType; }
	virtual METextureLayout GetType() override { return m_eRenderType; }

	void SetReadable(const bool& bReadable) { m_bReadable = bReadable; }
	virtual bool GetReadable() override { return m_bReadable; }

	void SetSize(const Vector2& v2Size) { m_v2Size = v2Size; }
	virtual Vector2 GetSize() override { return m_v2Size; }

public:
	virtual MTextureBuffer* GetBuffer() override { return m_pTextureBuffer; }
	MRenderTextureBuffer* GetRenderBuffer() { return m_pTextureBuffer; }

	virtual void GenerateBuffer(MIDevice* pDevice) = 0;
	virtual void DestroyBuffer(MIDevice* pDevice) = 0;

protected:
	bool m_bReadable;
	Vector2 m_v2Size;
	MRenderTextureBuffer* m_pTextureBuffer;
	METextureLayout m_eRenderType;
};

class MORTY_CLASS MRenderSwapchainTexture : public MIRenderBackTexture
{
public:
	MRenderSwapchainTexture();
	virtual ~MRenderSwapchainTexture();
public:
	virtual void GenerateBuffer(MIDevice* pDevice) override;
	virtual void DestroyBuffer(MIDevice* pDevice) override;
};

class MORTY_CLASS MRenderBackTexture : public MIRenderBackTexture
{
public:
	MRenderBackTexture();
	virtual ~MRenderBackTexture() {}

public:
	virtual void GenerateBuffer(MIDevice* pDevice) override;
	virtual void DestroyBuffer(MIDevice* pDevice) override;
};

class MORTY_CLASS MRenderSubpassTexture : public MIRenderBackTexture
{
public:
	MRenderSubpassTexture();
	virtual ~MRenderSubpassTexture() {}

public:
	virtual void GenerateBuffer(MIDevice* pDevice) override;
	virtual void DestroyBuffer(MIDevice* pDevice) override;
};

class MORTY_CLASS MRenderDepthTexture : public MIRenderTexture
{
public:
	MRenderDepthTexture();
	virtual ~MRenderDepthTexture() {}

public:

	void SetSize(const Vector2& v2Size) { m_v2Size = v2Size; }
	virtual Vector2 GetSize() override { return m_v2Size; }
	virtual METextureLayout GetType() override { return METextureLayout::ER32; }
	virtual bool GetReadable() override { return false; }
	virtual MTextureBuffer* GetBuffer() override;
	MDepthTextureBuffer* GetDepthBuffer() { return m_pTextureBuffer; };
	void GenerateBuffer(MIDevice* pDevice);
	void DestroyBuffer(MIDevice* pDevice);
		
private:
	Vector2 m_v2Size;
	MDepthTextureBuffer* m_pTextureBuffer;
};

#endif
