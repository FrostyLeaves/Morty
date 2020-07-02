/**
 * @File         MTransparentRenderTarget
 * 
 * @Created      2020-05-06 02:09:43
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTRANSPARENTRENDERTARGET_H_
#define _M_MTRANSPARENTRENDERTARGET_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MResource.h"
#include "MTextureRenderTarget.h"

#include "MMaterialGroup.h"
#include "MRenderStructure.h"

class MORTY_CLASS MTransparentRenderTarget : public MTextureRenderTarget
{
public:
	M_OBJECT(MTransparentRenderTarget);
    MTransparentRenderTarget();
    virtual ~MTransparentRenderTarget();

public:

public:
	virtual void OnCreated() override;
	virtual void OnDelete() override;

	virtual void OnRender(MIRenderer* pRenderer) override;

    void Render(MIRenderer* pRenderer, MViewport* pViewport, MIRenderTarget* pRenderTarget, std::vector<MMaterialGroup>* pGroup);

    void SetPrevLayerFrontDepthTexture(MITexture* pFrontDepthTexture) { m_pFrontDepthTexture = pFrontDepthTexture; }
    void SetPrevLayerBackDepthTexture(MITexture* pBackDepthTexture) { m_pBackDepthTexture = pBackDepthTexture; }

    void ResetPrevLayerTexture();

private:

    MResourceKeeper m_Material;

    MITexture* m_pFrontDepthTexture;
    MITexture* m_pBackDepthTexture;

    MITexture* m_pWhiteTexture;
    MITexture* m_pBlackTexture;

    std::vector<MMaterialGroup>* m_pTransparentMeshes;
    MViewport* m_pViewport;
};

#endif
