/**
 * @File         MBasicPostProcessRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"
#include "RenderProgram/MRenderInfo.h"

class MORTY_API MBasicPostProcessRenderWork : public ISinglePassRenderWork
{
	MORTY_INTERFACE(MBasicPostProcessRenderWork)

	void Initialize(MEngine* pEngine) override;
	void Release() override;

	void Render(const MRenderInfo& info) override;

	virtual std::shared_ptr<MMaterial> CreateMaterial() = 0;

protected:

	void BindTarget() override;

	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
};
