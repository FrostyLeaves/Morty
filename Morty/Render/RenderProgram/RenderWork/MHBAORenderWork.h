/**
 * @File         MHBAORenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "MBasicPostProcessRenderWork.h"
#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"

#include "RenderProgram/MRenderInfo.h"
#include "MRenderWork.h"
#include "Render/MRenderPass.h"
#include "Basic/MCameraFrustum.h"

class MORTY_API MHBAORenderWork : public MBasicPostProcessRenderWork
{
	MORTY_CLASS(MHBAORenderWork)

    static const MStringId HBAOOutput;

	void Release() override;

	std::shared_ptr<MMaterial> CreateMaterial() override;

	void RenderSetup(const MRenderInfo& info) override;

	void RegisterSetting() override;

protected:

	std::vector<MRenderTaskInputDesc> InitInputDesc() override;

	std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;

};
