/**
 * @File         MDebugRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"

#include "RenderProgram/MRenderInfo.h"
#include "MRenderWork.h"
#include "Render/MRenderPass.h"
#include "Basic/MCameraFrustum.h"

MORTY_SPACE_BEGIN

class MORTY_API MDebugRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MDebugRenderWork)

    static const MStringId BackBufferOutput;
	static const MStringId DepthBufferOutput;
public:

	void Render(const MRenderInfo& info) override;
	void Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

protected:

	void BindTarget() override;

	std::vector<MRenderTaskInputDesc> InitInputDesc() override;

	std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

MORTY_SPACE_END