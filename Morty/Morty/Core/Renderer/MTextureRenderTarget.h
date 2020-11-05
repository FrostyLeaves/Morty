/**
 * @File         MTextureRenderTarget
 * 
 * @Created      2019-12-28 19:45:57
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTEXTURERENDERTARGET_H_
#define _M_MTEXTURERENDERTARGET_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MIDevice.h"
#include "MTexture.h"
#include "MIRenderTarget.h"
#include "MRenderStructure.h"

class MIDevice;
class MViewport;
class MTexture;
class MRenderDepthTexture;


class MORTY_CLASS MTextureRenderTarget : public MIRenderTarget
{
public:
	M_OBJECT(MTextureRenderTarget)
public:
	enum METextureRenderTargetType
	{
		ERenderNone = 0,
		ERenderBack = 1,
		ERenderDepth = 2,
	};

public:
	MTextureRenderTarget();
    virtual ~MTextureRenderTarget();

public:

	virtual MRenderDepthTexture* GetCurrDepthTexture() override;

	virtual bool GetDepthEnable() override;

	virtual uint32_t GetMFrameBufferNum() override { return m_vBufferInfo.size(); }
	virtual MFrameBuffer* GetFrameBuffer(const uint32_t& unIndex) override;
	virtual MFrameBuffer* GetCurrFrameBuffer(const uint32_t& unFrameIdx = 0) override { return GetFrameBuffer(unFrameIdx); }

	virtual void Resize(const Vector2& v2Size) override;

	std::vector<MIRenderBackTexture*>* GetBackTexture(const uint32_t& unIndex);

	void SetBackTexture(const std::array<MIRenderBackTexture*, M_BUFFER_NUM>& vBackTexture, const uint32_t& unIndex);
	void SetDepthTexture(const std::array<MRenderDepthTexture*, M_BUFFER_NUM> vDepthTexture);

	uint32_t GetRenderTargetType() { return m_eRenderTargetType; }


	void ResizeAllTexture(const Vector2& v2Size);

public:

	virtual void OnCreated() override;

	virtual void Release() override;


public:
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	virtual std::vector<struct ID3D11RenderTargetView*> GetRenderTargetViews() override;
	virtual struct ID3D11DepthStencilView* GetDepthStencilView() override;
#elif RENDER_GRAPHICS == MORTY_VULKAN
#endif

public:
;
	std::array<MFrameBuffer, M_BUFFER_NUM> m_vBufferInfo;
protected:

	uint32_t m_eRenderTargetType;
	uint32_t m_fWidth;
	uint32_t m_fHeight;
};

#endif
