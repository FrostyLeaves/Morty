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

#include "RenderProgram/MForwardRenderShaderPropertyBlock.h"


class MTexture;
class MTextureResource;
class MORTY_API MTransparentRenderWork : public IRenderWork
{
public:
    MORTY_CLASS(MTransparentRenderWork);

public:

    void Initialize(MEngine* pEngine) override;
	void Release(MEngine* pEngine) override;
	void Resize(Vector2 size) override;

    MEngine* GetEngine() const { return m_pEngine; }

	void SetRenderTarget(std::shared_ptr<MTexture> pOutputTexture, std::shared_ptr<MTexture> pDepthTexture);

    void Render(MRenderInfo& info);

	void RenderDepthPeel(MRenderInfo& info);

protected:

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

	MEngine* m_pEngine = nullptr;

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

	MRenderPass m_peelRenderPass;
	MRenderPass m_fillRenderPass;
	std::array<MForwardRenderTransparentShaderPropertyBlock, 2> m_aFramePropertyBlock;
};
