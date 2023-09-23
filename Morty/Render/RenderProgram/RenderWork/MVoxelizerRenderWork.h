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
    void RenderDebugVoxel(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

    std::shared_ptr<MMaterial> GetVoxelizerMaterial() const { return m_pVoxelizerMaterial; }
    std::shared_ptr<MMaterial> GetVoxelDebugMaterial() const { return m_pVoxelDebugMaterial; }
    const MBuffer* GetVoxelTableBuffer() const;
    const MBuffer* GetVoxelDebugBuffer() const;


protected:

    void InitializeBuffer();
    void ReleaseBuffer();

    void InitializeDispatcher();
    void ReleaseDispatcher();

    void InitializeRenderPass();
    void ReleaseRenderPass();

    MComputeDispatcher* m_pVoxelMapGenerator = nullptr;
    std::shared_ptr<MShaderConstantParam> m_pVoxelMapSetting = nullptr;
    std::shared_ptr<MMaterial> m_pVoxelizerMaterial = nullptr;
    std::shared_ptr<MMaterial> m_pVoxelDebugMaterial = nullptr;
    MBuffer m_voxelizerBuffer;
    MBuffer m_drawIndirectBuffer;

    std::shared_ptr<MTexture> m_pVoxelizerRenderTarget = nullptr;
    MRenderPass m_voxelizerRenderPass;
};
