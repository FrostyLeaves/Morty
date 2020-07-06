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

class MORTY_CLASS MTextureRenderTarget : public MIRenderTarget, public MObject
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

	virtual uint32_t GetBackNum() override { return m_vBackTexture.size(); }
	virtual MRenderTextureBuffer* GetBackBuffer(const uint32_t& unIndex) override;
	virtual MRenderDepthTexture* GetDepthTexture() override { return m_pDepthTexture; }
	virtual MColor GetBackClearColor(const uint32_t& unIndex) override;


	MRenderTargetTexture* GetBackTexture(const uint32_t& unIndex);

	void SetBackTexture(MRenderTargetTexture* pBackTexture, const uint32_t& unIndex, const bool& bClearWhenRender, const MColor& clearColor = MColor::Black);
	void SetDepthTexture(MRenderDepthTexture* pDepthTexture, const bool& bClearWhenRender);

	uint32_t GetRenderTargetType() { return m_eRenderTargetType; }


	void ResizeAllTexture(const Vector2& v2Size);

public:

	virtual void OnCreated() override;

	virtual void Release(MIDevice* pDevice) override;


	void InitRenderPass();

public:
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	virtual std::vector<struct ID3D11RenderTargetView*> GetRenderTargetViews() override;
	virtual struct ID3D11DepthStencilView* GetDepthStencilView() override;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	virtual VkFramebuffer GetFrameBuffer(const uint32_t& unIndex) override;
	VkRenderPass m_VkRenderPass;
#endif

public:

	std::vector<MRenderTargetTexture*> m_vBackTexture;
	std::vector<MColor> m_vBackClearColor;
	MRenderDepthTexture* m_pDepthTexture;
protected:

	uint32_t m_eRenderTargetType;
	uint32_t m_fWidth;
	uint32_t m_fHeight;
};

#endif
