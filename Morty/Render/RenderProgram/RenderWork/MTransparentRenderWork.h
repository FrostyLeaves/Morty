/**
 * @File         MForwardTransparentRenderProgram
 * 
 * @Created      2020-08-25 11:25:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "Render/MMesh.h"
#include "RenderProgram/MRenderInfo.h"
#include "Render/MRenderPass.h"

#include <array>

#include "MSinglePassRenderWork.h"
#include "RenderProgram/MFrameShaderPropertyBlock.h"


class MCullingResultRenderable;
class MTexture;
class MTextureResource;


//FIXME: delete it.
class MORTY_API MForwardRenderTransparentShaderPropertyBlock : public MFrameShaderPropertyBlock
{
public:

	void BindMaterial(const std::shared_ptr<MMaterial>& pMaterial) override { MORTY_UNUSED(pMaterial); }

};

class MORTY_API MTransparentRenderWork : public ISinglePassRenderWork
{
public:
    MORTY_CLASS(MTransparentRenderWork);

public:

    void Initialize(MEngine* pEngine) override;
	void Release(MEngine* pEngine) override;
	void Resize(Vector2i size) override;

	void Render(MRenderInfo& info, const std::vector<MCullingResultRenderable*>& vRenderable);

protected:

	void RenderDepthPeel(MRenderInfo& info, const std::vector<MCullingResultRenderable*>& vRenderable);
	void RenderScreenFill(MRenderInfo& info);

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

	std::shared_ptr<MTexture> m_pFrontTexture;
	std::shared_ptr<MTexture> m_pBackTexture;
	std::shared_ptr<MTexture> m_pFrontDepthForPassA;
	std::shared_ptr<MTexture> m_pBackDepthForPassA;
	std::shared_ptr<MTexture> m_pFrontDepthForPassB;
	std::shared_ptr<MTexture> m_pBackDepthForPassB;

	MRenderPass m_peelRenderPass;
	std::array<std::shared_ptr<MShaderPropertyBlock>, 2> m_aFramePropertyBlock;
};
