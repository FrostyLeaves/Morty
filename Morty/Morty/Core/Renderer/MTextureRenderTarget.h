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
	
	MRenderTargetTexture* GetBackTexture(const unsigned int& unIndex = 0) { return &m_vBackTexture[unIndex]; }
	virtual MRenderDepthTexture* GetDepthTexture() override { return m_pDepthTexture; }

	virtual void SetBackgroundColor(const unsigned int& unTargetIndex, const MColor& color) override;
	virtual const MColor& GetBackgroundColor(const unsigned int& unTargetIndex) const override;

	virtual bool GetNeedCleanTargetView(const unsigned int& unTargetIndex) const override;

	unsigned int GetRenderTargetType() { return m_eRenderTargetType; }

public:

	void Initialize(const unsigned int& eType, const unsigned int& unWidth, const unsigned int& unHeight);
	void Initialize(const unsigned int& eType, const unsigned int& unWidth, const unsigned int& unHeight, const std::vector<MERenderTextureType>& vTextureTypes);

	virtual void OnCreated() override;

	virtual void OnResize(const unsigned int& unWidth, const unsigned int& unHeight) override;

	virtual void Release(MIDevice* pDevice) override;

public:
	bool m_vNeedCleanBeforeRender[8];

	MRenderTargetTexture* m_vBackTexture;
	MRenderDepthTexture* m_pDepthTexture;
protected:

	MColor* m_vBackgroundColor;
	unsigned int m_eRenderTargetType;
	unsigned int m_fWidth;
	unsigned int m_fHeight;
};

#endif
