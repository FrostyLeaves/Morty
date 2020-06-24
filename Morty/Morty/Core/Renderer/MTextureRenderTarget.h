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
#include "MIDevice.h"
#include "MObject.h"
#include "MIRenderTarget.h"
#include "MTexture.h"

class MIDevice;
class MViewport;
class MTexture;
class MRenderTargetTexture;
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
	
	MRenderTargetTexture* GetBackTexture(const uint32_t& unIndex = 0) { return &m_vBackTexture[unIndex]; }
	virtual MRenderDepthTexture* GetDepthTexture() override { return m_pDepthTexture; }

	virtual void SetBackgroundColor(const uint32_t& unTargetIndex, const MColor& color) override;
	virtual const MColor& GetBackgroundColor(const uint32_t& unTargetIndex) const override;

	virtual bool GetNeedCleanTargetView(const uint32_t& unTargetIndex) const override;

	uint32_t GetRenderTargetType() { return m_eRenderTargetType; }

public:

	void Initialize(const uint32_t& eType, const uint32_t& unWidth, const uint32_t& unHeight);
	void Initialize(const uint32_t& eType, const uint32_t& unWidth, const uint32_t& unHeight, const std::vector<MERenderTextureType>& vTextureTypes);

	virtual void OnCreated() override;

	virtual void OnResize(const uint32_t& unWidth, const uint32_t& unHeight) override;

	virtual void Release(MIDevice* pDevice) override;

public:
	bool m_vNeedCleanBeforeRender[8];

	MRenderTargetTexture* m_vBackTexture;
	MRenderDepthTexture* m_pDepthTexture;
protected:

	MColor* m_vBackgroundColor;
	uint32_t m_eRenderTargetType;
	uint32_t m_fWidth;
	uint32_t m_fHeight;
};

#endif
