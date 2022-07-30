/**
 * @File         MForwardTransparentRenderProgram
 * 
 * @Created      2020-08-25 11:25:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_TRANSPARENT_RENDER_WORK_H_
#define _M_TRANSPARENT_RENDER_WORK_H_
#include "MGlobal.h"
#include "MObject.h"

#include "MMesh.h"
#include "MRenderInfo.h"
#include "MRenderPass.h"
#include "MForwardRenderShaderParamSet.h"

#include <array>

class MTexture;
class MTextureResource;
class MORTY_API MTransparentRenderWork : public MObject
{
public:
    MORTY_CLASS(MTransparentRenderWork);
    MTransparentRenderWork();
    virtual ~MTransparentRenderWork();

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void SetRenderTarget(MTexture* pOutputTexture, MTexture* pDepthTexture);

    void Render(MRenderInfo& info);

	void RenderDepthPeel(MRenderInfo& info);

	void Resize(const Vector2& v2Size);

protected:

    void InitializeMesh();
    void ReleaseMesh();

    void InitializeMaterial();
    void ReleaseMaterial();

    void InitializeTexture();
    void ReleaseTexture();

	void InitializePeelRenderPass();
	void ReleasePeelRenderPass();

	void InitializeFillRenderPass();
	void ReleaseFillRenderPass();

	void InitializeFrameShaderParams();
	void ReleaseFrameShaderParams();
private:

	std::shared_ptr<MTextureResource> m_pWhiteTexture;
	std::shared_ptr<MTextureResource> m_pBlackTexture;

	std::shared_ptr<MMaterial> m_pDrawFillMaterial;
	std::shared_ptr<MMaterial> m_pDrawPeelMaterial;

	MTexture* m_pOutputTexture;
	MTexture* m_pDepthTexture;
	MTexture* m_pDefaultOutputTexture;

	MTexture* m_pFrontTexture;
	MTexture* m_pBackTexture;
	MTexture* m_pFrontDepthForPassA;
	MTexture* m_pBackDepthForPassA;
	MTexture* m_pFrontDepthForPassB;
	MTexture* m_pBackDepthForPassB;

	MMesh<Vector2> m_TransparentDrawMesh;

	MRenderPass m_peelRenderPass;
	MRenderPass m_fillRenderPass;
	MForwardRenderTransparentShaderParamSet m_aFrameParamSet[2];
};

#endif
