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
class MITexture;
class MORTY_CLASS MTextureRenderTarget : public MIRenderTarget
{
public:
	MTextureRenderTarget(MIDevice* m_pDevice);
    virtual ~MTextureRenderTarget();

public:
	//×¼±¸äÖÈ¾×´Ì¬
	virtual void OnReadyRenderState() override;
	//»Ö¸´äÖÈ¾×´Ì¬
	virtual void OnRecoverRenderState() override;
	virtual void OnResize(int nWidth, int nHeight) override;

	static MTextureRenderTarget* CreateForTexture(MIDevice* pDevice, const unsigned int& unWidth, const unsigned int& unHeight);

private:
	MIDevice* m_pDevice;
	MITexture* m_pTargetTexture;
};

#endif
