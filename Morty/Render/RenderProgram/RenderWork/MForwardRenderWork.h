/**
 * @File         MForwardRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"

#include "MRenderWork.h"
#include "RenderProgram/MRenderInfo.h"

class MORTY_API MForwardRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MForwardRenderWork)

public:

	void Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

};
