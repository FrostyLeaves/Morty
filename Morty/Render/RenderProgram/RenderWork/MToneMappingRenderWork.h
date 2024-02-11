/**
 * @File         MToneMappingRenderWork
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

MORTY_SPACE_BEGIN

class MORTY_API MToneMappingRenderWork : public MBasicPostProcessRenderWork
{
	MORTY_CLASS(MToneMappingRenderWork)

    static const MStringId ToneMappingResult;

	std::shared_ptr<MMaterial> CreateMaterial() override;

protected:

	std::vector<MRenderTaskInputDesc> InitInputDesc() override;

	std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;

};

MORTY_SPACE_END