﻿/**
 * @File         MTexture
 *
 * @Created      2019-09-11 16:12:31
 *
 * @Author       DoubleYe
**/

#ifndef _M_MTEXTURE_H_
#define _M_MTEXTURE_H_
#include "MGlobal.h"
#include "Vector.h"
#include "Type/MColor.h"
#include "MRenderStructure.h"

#include <array>

class MIDevice;
class MTextureBuffer;
class MRenderTextureBuffer;
class MDepthTextureBuffer;
class MORTY_API MITexture
{
public:
	MITexture() {}
	virtual ~MITexture() {}

public:


	uint32_t static GetImageMemorySize(const METextureLayout& layout);

	virtual Vector2 GetSize() = 0;
	virtual METextureLayout GetType() = 0;
	virtual MTextureBuffer* GetBuffer(const uint32_t& nFrameIdx) = 0;
	virtual bool GetReadable() = 0;
	virtual unsigned char* GetImageData() { return nullptr; }
};

class MORTY_API MTexture : public MITexture
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

	virtual MTextureBuffer* GetBuffer(const uint32_t& nFrameIdx) override { return m_pTextureBuffer; }

private:

	unsigned char* m_pImageData;
	Vector2 m_v2Size;

	uint32_t m_unImageDataArraySize;

	MTextureBuffer* m_pTextureBuffer;
	METextureLayout m_eRenderType;
	bool m_bReadable;
	bool m_bMipmapsEnable;

};

class MORTY_API MTextureCube : public MITexture
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

	virtual MTextureBuffer* GetBuffer(const uint32_t& nFrameIdx) override { return m_pTextureBuffer; }

	void SetTextures(MTexture* vTexture[6]);
	void SetTexture(MTexture* pTexture, const MECubeFace& eFace);

private:
	Vector2 m_v2Size;
	MTexture* m_vTexture[6];
	MTextureBuffer* m_pTextureBuffer;
};

class MORTY_API MIRenderTexture : public MITexture
{
public:

	MIRenderTexture();
	virtual ~MIRenderTexture() {}
	//Should Set before GenerateBuffer

	void SetType(const METextureLayout& eType) { m_eRenderType = eType; }
	virtual METextureLayout GetType() override { return m_eRenderType; }

	void SetUsage(const METextureUsage& eType) { m_eUsageType = eType; }
	METextureUsage GetUsage() { return m_eUsageType; }


	void SetReadable(const bool& bReadable) { m_bReadable = bReadable; }
	virtual bool GetReadable() override { return m_bReadable; }

	void SetSize(const Vector2& v2Size) { m_v2Size = v2Size; }
	virtual Vector2 GetSize() override { return m_v2Size; }

public:

	virtual uint32_t GetBufferNum() = 0;

	virtual void GenerateBuffer(MIDevice* pDevice) = 0;
	virtual void DestroyBuffer(MIDevice* pDevice) = 0;

protected:
	bool m_bReadable;
	Vector2 m_v2Size;
	METextureLayout m_eRenderType;
	METextureUsage m_eUsageType;
};

class MORTY_API MRenderSwapchainTexture : public MIRenderTexture
{
public:
	MRenderSwapchainTexture(const size_t& nSize):
	m_aTextureBuffer(nSize){}
	virtual ~MRenderSwapchainTexture() {}


	virtual uint32_t GetBufferNum() override { return m_aTextureBuffer.size(); }

	virtual void GenerateBuffer(MIDevice* pDevice) override;
	virtual void DestroyBuffer(MIDevice* pDevice) override;

	virtual MTextureBuffer* GetBuffer(const uint32_t& nFrameIdx) override;

protected:

	std::vector<MRenderTextureBuffer> m_aTextureBuffer;
};

class MORTY_API MRenderTexture : public MIRenderTexture
{
public:
	MRenderTexture();
	virtual ~MRenderTexture() {}

public:

	virtual uint32_t GetBufferNum() override { return M_BUFFER_NUM; }

	virtual MTextureBuffer* GetBuffer(const uint32_t& nFrameIdx) override;

	virtual void GenerateBuffer(MIDevice* pDevice) override;

	virtual void DestroyBuffer(MIDevice* pDevice) override;
protected:

	std::array<MRenderTextureBuffer, M_BUFFER_NUM> m_aTextureBuffer;
};


#endif
