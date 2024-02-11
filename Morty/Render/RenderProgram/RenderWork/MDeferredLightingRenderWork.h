/**
 * @File         MDeferredLightingRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "MGBufferRenderWork.h"
#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"

#include "MRenderWork.h"
#include "MShadowMapRenderWork.h"
#include "RenderProgram/MRenderInfo.h"

MORTY_SPACE_BEGIN

class MORTY_API MDeferredLightingRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MDeferredLightingRenderWork)
	static const MStringId DeferredLightingOutput;

	void Initialize(MEngine* pEngine) override;
	void Release() override;

	void Render(const MRenderInfo& info) override;

protected:

	void BindTarget() override;

	std::vector<MRenderTaskInputDesc> InitInputDesc() override;

	std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;


private:

	std::shared_ptr<MMaterial> m_pLightningMaterial = nullptr;
};

MORTY_SPACE_END