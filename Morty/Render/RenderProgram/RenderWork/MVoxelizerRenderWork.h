/**
 * @File         MVoxelizerRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MMaterial.h"
#include "Render/MRenderGlobal.h"
#include "MSinglePassRenderWork.h"

#include "RenderProgram/MRenderInfo.h"
#include "MRenderWork.h"
#include "Render/MRenderPass.h"
#include "Basic/MCameraFrustum.h"
#include "Render/MBuffer.h"
#include <memory>


class MORTY_API MVoxelizerRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MVoxelizerRenderWork)

public:

    void Initialize(MEngine* pEngine) override;
	void Release(MEngine* pEngine) override;

    void Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

protected:

    void InitializeDispatcher();
    void ReleaseDispatcher();

    void DrawVoxelizerMap(MIRenderCommand* pCommand);

    MComputeDispatcher* m_pVoxelMapGenerator = nullptr;
    std::shared_ptr<MMaterial> m_pVoxelDebugMaterial = nullptr;
    MBuffer m_drawIndirectBuffer;
    bool m_bDebugMode = false;
};
