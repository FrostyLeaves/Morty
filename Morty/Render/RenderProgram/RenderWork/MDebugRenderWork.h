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

class MORTY_API MDebugRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MDebugRenderWork)
public:

	void Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

};
