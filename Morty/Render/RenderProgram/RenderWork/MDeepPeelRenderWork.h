/**
 * @File         MDeepPeelRenderWork
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


class MORTY_API MDeepPeelRenderWork : public ISinglePassRenderWork
{
public:
    MORTY_CLASS(MDeepPeelRenderWork);

	static const MStringId FrontTextureOutput;
	static const MStringId BackTextureOutput;
	static const MStringId DepthOutput[4];
public:

    void Initialize(MEngine* pEngine) override;
	void Release() override;

	void Render(const MRenderInfo& info) override;
	void Render(const MRenderInfo& info, const std::vector<MCullingResultRenderable*>& vRenderable);

protected:

    void InitializeMaterial();
    void ReleaseMaterial();

    void InitializeTexture();
    void ReleaseTexture();

	void InitializeRenderPass();

	void InitializeFrameShaderParams();
	void ReleaseFrameShaderParams();

	void BindTarget() override;

	std::vector<MRenderTaskInputDesc> InitInputDesc() override;

	std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;


private:

	std::shared_ptr<MTextureResource> m_pWhiteTexture;
	std::shared_ptr<MTextureResource> m_pBlackTexture;

	std::shared_ptr<MMaterial> m_pDrawPeelMaterial;
	std::shared_ptr<MMaterial> m_pForwardMaterial;

	std::array<std::shared_ptr<MShaderPropertyBlock>, 2> m_aFramePropertyBlock;
};
