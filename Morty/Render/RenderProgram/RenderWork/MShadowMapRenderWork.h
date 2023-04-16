/**
 * @File         MShadowMapRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSHADOWMAPRENDERWORK_H_
#define _M_MSHADOWMAPRENDERWORK_H_
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

	std::shared_ptr<ITextureInputAdapter> GetShadowMap() const;
};


#endif
