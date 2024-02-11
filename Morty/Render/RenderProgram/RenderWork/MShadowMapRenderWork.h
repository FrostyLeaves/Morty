/**
 * @File         MShadowMapRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"

#include "Render/MRenderPass.h"
#include "Basic/MCameraFrustum.h"
#include "RenderProgram/MRenderInfo.h"

MORTY_SPACE_BEGIN

class MTaskNode;
class MIRenderCommand;
class MORTY_API MShadowMapRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MShadowMapRenderWork)
	static const MStringId ShadowMapBufferOutput;

public:
	void Initialize(MEngine* pEngine) override;

	void Render(const MRenderInfo& info) override;
	void Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

	std::shared_ptr<IGetTextureAdapter> GetShadowMap() const;

	std::shared_ptr<IShaderPropertyUpdateDecorator> GetFramePropertyDecorator() override;

protected:

	void OnCreated() override;
	void BindTarget() override;

	std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

MORTY_SPACE_END