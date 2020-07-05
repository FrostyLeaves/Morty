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

	virtual MRenderTargetTexture* GetBackTexture(const uint32_t& unIndex) override;
	virtual MRenderDepthTexture* GetDepthTexture() override { return m_pDepthTexture; }

	void SetBackTexture(MRenderTargetTexture* pBackTexture, const uint32_t& unIndex);
	void SetDepthTexture(MRenderDepthTexture* pDepthTexture);

	uint32_t GetRenderTargetType() { return m_eRenderTargetType; }

	virtual VkFramebuffer GetFrameBuffer(const uint32_t& unIndex) override;

	void ResizeAllTexture(const Vector2& v2Size);

public:

	virtual void OnCreated() override;

	virtual void Release(MIDevice* pDevice) override;

public:
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	virtual std::vector<struct ID3D11RenderTargetView*> GetRenderTargetViews() override;
	virtual struct ID3D11DepthStencilView* GetDepthStencilView() override;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkRenderPass m_VkRenderPass;
#endif

public:

	std::vector<MRenderTargetTexture*> m_vBackTexture;
	MRenderDepthTexture* m_pDepthTexture;
protected:

	uint32_t m_eRenderTargetType;
	uint32_t m_fWidth;
	uint32_t m_fHeight;
};

#endif
