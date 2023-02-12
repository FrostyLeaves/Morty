/**
 * @File         MForwardTransparentRenderProgram
 * 
 * @Created      2020-08-25 11:25:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_TRANSPARENT_RENDER_WORK_H_
#define _M_TRANSPARENT_RENDER_WORK_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "Render/MMesh.h"
#include "MRenderInfo.h"
#include "Render/MRenderPass.h"
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

	void SetRenderTarget(std::shared_ptr<MTexture> pOutputTexture, std::shared_ptr<MTexture> pDepthTexture);

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
	std::shared_ptr<MMaterial> m_pForwardMaterial;

	std::shared_ptr<MTexture> m_pOutputTexture;
	std::shared_ptr<MTexture> m_pDepthTexture;
	std::shared_ptr<MTexture> m_pDefaultOutputTexture;

	std::shared_ptr<MTexture> m_pFrontTexture;
	std::shared_ptr<MTexture> m_pBackTexture;
	std::shared_ptr<MTexture> m_pFrontDepthForPassA;
	std::shared_ptr<MTexture> m_pBackDepthForPassA;
	std::shared_ptr<MTexture> m_pFrontDepthForPassB;
	std::shared_ptr<MTexture> m_pBackDepthForPassB;

	MMesh<Vector2> m_TransparentDrawMesh;

	MRenderPass m_peelRenderPass;
	MRenderPass m_fillRenderPass;
	std::array<MForwardRenderTransparentShaderPropertyBlock, 2> m_aFramePropertyBlock;
};

#endif
