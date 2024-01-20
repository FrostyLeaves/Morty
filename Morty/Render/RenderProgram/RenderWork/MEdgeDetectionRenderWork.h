/**
 * @File         MEdgeDetectionRenderWork
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

class MORTY_API MEdgeDetectionRenderWork : public MBasicPostProcessRenderWork
{
	MORTY_CLASS(MEdgeDetectionRenderWork)

    static const MStringId EdgeDetectionResult;

	std::shared_ptr<MMaterial> CreateMaterial() override;

protected:

	std::vector<MStringId> GetInputName() override;

	std::vector<MRenderTaskOutputDesc> GetOutputName() override;

};
