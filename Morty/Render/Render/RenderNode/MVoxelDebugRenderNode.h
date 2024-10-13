/**
 * @File         MVoxelDebugRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Material/MMaterial.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

#include "Basic/MBuffer.h"
#include "Basic/MCameraFrustum.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"

namespace morty
{

class IShaderPropertyUpdateDecorator;
class MORTY_API MVoxelDebugRenderNode : public ISinglePassRenderNode
{
    MORTY_CLASS(MVoxelDebugRenderNode)

    static const MStringId                          BackBufferOutput;
    static const MStringId                          DepthBufferOutput;

    void                                            Initialize(MEngine* pEngine) override;

    void                                            Release() override;

    std::shared_ptr<IShaderPropertyUpdateDecorator> GetFramePropertyDecorator() override;

    void                                            Render(const MRenderInfo& info) override;

    void
                               Render(const MRenderInfo&               info,
                                      const MVoxelMapSetting&          voxelSetting,
                                      const MBuffer*                   pVoxelizerBuffer,
                                      const std::vector<IRenderable*>& vRenderable);

    std::shared_ptr<MMaterial> GetVoxelDebugMaterial() const { return m_voxelDebugMaterial; }

    const MBuffer*             GetVoxelDebugBuffer() const;

    MTexturePtr                GetVoxelGITexture() const;

protected:
    void                                            InitializeBuffer();

    void                                            ReleaseBuffer();

    void                                            InitializeDispatcher();

    void                                            ReleaseDispatcher();

    void                                            BindTarget() override;

    std::vector<MRenderTaskInputDesc>               InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc>              InitOutputDesc() override;

    std::shared_ptr<IShaderPropertyUpdateDecorator> m_framePropertyUpdateDecorator = nullptr;

    MComputeDispatcher*                             m_voxelDebugIndirectGenerator = nullptr;
    std::shared_ptr<MMaterial>                      m_voxelDebugMaterial          = nullptr;
    MBuffer                                         m_drawIndirectBuffer;


    std::shared_ptr<MShaderConstantParam>           m_debugVoxelMapSetting = nullptr;
    std::shared_ptr<MShaderStorageParam>            m_voxelStorageBuffer   = nullptr;
};

}// namespace morty