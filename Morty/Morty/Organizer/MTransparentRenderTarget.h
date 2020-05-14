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
	virtual void OnCreated() override;
	virtual void OnDelete() override;

	virtual void OnRender(MIRenderer* pRenderer) override;

    void Render(MIRenderer* pRenderer, MIRenderTarget* pRenderTarget, std::vector<MMaterialGroup>* pGroup);

	void SetSceneDepthTexture(MRenderDepthTexture* pDepthTexture) { m_pSceneDepthTexture = pDepthTexture; }
	void SetPrevLevelRenderTarget(MTransparentRenderTarget* pRenderTarget) { m_pPrevLevelRenderTarget = pRenderTarget; }

private:

    MResourceKeeper m_Material;

    MRenderDepthTexture* m_pSceneDepthTexture;

    MIRenderTarget* m_pBackRenderTarget;
    MTransparentRenderTarget* m_pPrevLevelRenderTarget;
    std::vector<MMaterialGroup>* m_pTransparentMeshes;
};

#endif
