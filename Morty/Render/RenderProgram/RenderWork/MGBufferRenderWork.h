/**
 * @File         MGBufferRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_G_BUFFER_RENDER_WORK_H_
#define _M_G_BUFFER_RENDER_WORK_H_
#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"

#include "RenderProgram/MRenderInfo.h"
#include "MRenderWork.h"
#include "Render/MRenderPass.h"
#include "Basic/MCameraFrustum.h"



class MORTY_API MGBufferRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MGBufferRenderWork)

public:

    void Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

	std::shared_ptr<IGBufferAdapter> CreateGBuffer();
};


#endif
