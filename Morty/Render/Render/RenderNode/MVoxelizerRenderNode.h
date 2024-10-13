/**
 * @File         MVoxelizerRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Material/MMaterial.h"
#include "RenderProgram/RenderGraph/MSinglePassRenderNode.h"

#include "Basic/MBuffer.h"
#include "Basic/MCameraFrustum.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MRenderCommon.h"

namespace morty
{

class IShaderPropertyUpdateDecorator;
class MORTY_API MVoxelizerRenderNode : public ISinglePassRenderNode
{
    MORTY_CLASS(MVoxelizerRenderNode)

    static const MStringId VoxelizerBufferOutput;

public:
    void                                            Initialize(MEngine* pEngine) override;

    void                                            Release() override;

    std::shared_ptr<IShaderPropertyUpdateDecorator> GetFramePropertyDecorator() override;

    void                                            Resize(Vector2i size) override;

    void                                            Render(const MRenderInfo& info) override;

    void Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

    const std::unordered_map<MStringId, std::shared_ptr<MMaterial>>& GetVoxelizerMaterial() const
    {
        return m_voxelizerMaterial;
    }

    const MBuffer*            GetVoxelTableBuffer() const;

    const MBuffer*            GetVoxelDebugBuffer() const;

    std::shared_ptr<MTexture> GetVoxelGITexture() const;

    MBoundsAABB               GetVoxelizerBoundsAABB(uint32_t nClipmapIdx) const;

    void                      SetupVoxelSetting(const Vector3& f3CameraPosition, const uint32_t nClipmapIdx);

    MVoxelMapSetting          GetVoxelSetting() const { return m_voxelSetting; }

protected:
    void                                                      InitializeBuffer();

    void                                                      ReleaseBuffer();

    void                                                      InitializeDispatcher();

    void                                                      ReleaseDispatcher();

    void                                                      InitializeVoxelTextureDispatcher();

    void                                                      ReleaseVoxelTextureDispatcher();

    void                                                      InitializeRenderPass();


    void                                                      BindTarget() override;

    std::vector<MRenderTaskInputDesc>                         InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc>                        InitOutputDesc() override;


    MVoxelMapSetting                                          m_voxelSetting;
    std::shared_ptr<IShaderPropertyUpdateDecorator>           m_framePropertyUpdateDecorator = nullptr;

    MComputeDispatcher*                                       m_voxelTextureGenerator    = nullptr;
    std::shared_ptr<MShaderConstantParam>                     m_voxelizerVoxelMapSetting = nullptr;
    std::unordered_map<MStringId, std::shared_ptr<MMaterial>> m_voxelizerMaterial        = {};
    MBuffer                                                   m_voxelizerBuffer;

    std::shared_ptr<MTexture>                                 m_voxelGITexture = nullptr;
};

}// namespace morty