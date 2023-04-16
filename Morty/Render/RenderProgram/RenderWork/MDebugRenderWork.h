/**
 * @File         MDebugRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_DEBUG_RENDER_WORK_H_
#define _M_DEBUG_RENDER_WORK_H_
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

	void Initialize(MEngine* pEngine) override;
	void Release(MEngine* pEngine) override;

	void InitializeMaterial();
	void ReleaseMaterial();

	void Render(MRenderInfo& info);

private:

	std::shared_ptr<MMaterial> m_pSkyBoxMaterial;
};


#endif
