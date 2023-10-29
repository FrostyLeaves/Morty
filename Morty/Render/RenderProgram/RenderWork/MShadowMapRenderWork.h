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

class MTaskNode;
class MIRenderCommand;
class MORTY_API MShadowMapRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MShadowMapRenderWork)

public:
	void Initialize(MEngine* pEngine) override;

	void Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

	void Resize(Vector2i size) override;

	std::shared_ptr<ITextureInputAdapter> GetShadowMap() const;
};
