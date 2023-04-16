/**
 * @File         MForwardRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_FORWARD_RENDER_WORK_H_
#define _M_FORWARD_RENDER_WORK_H_
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


#endif
