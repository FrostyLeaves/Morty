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
#include "MIRenderTarget.h"

class MIDevice;
class MViewport;
class MTexture;
class MRenderDepthTexture;
class MORTY_CLASS MTextureRenderTarget : public MIRenderTarget
{
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

	MRenderDepthTexture* GetDepthTexture(){ return m_pDepthTexture; }

	unsigned int GetRenderTargetType() { return m_eRenderTargetType; }

public:
	virtual void OnResize(const unsigned int& unWidth, const unsigned int& unHeight) override;

	static MTextureRenderTarget* CreateForTexture(MIDevice* pDevice, const unsigned int& eRenderTargetType, const unsigned int& unWidth, const unsigned int& unHeight);

public:
	MTexture* m_pBackTexture;
	MRenderDepthTexture* m_pDepthTexture;
protected:
	MIDevice* m_pDevice;
	unsigned int m_eRenderTargetType;
};

#endif
