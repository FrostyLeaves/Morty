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


class IShaderPropertyUpdateDecorator;

class MORTY_API MVoxelizerRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MVoxelizerRenderWork)

public:

    void Initialize(MEngine* pEngine) override;
	void Release(MEngine* pEngine) override;
    std::shared_ptr<IShaderPropertyUpdateDecorator> GetFramePropertyDecorator() override;

    void Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);
    void RenderDebugVoxel(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

    const std::unordered_map<MStringId, std::shared_ptr<MMaterial>>& GetVoxelizerMaterial() const { return m_tVoxelizerMaterial; }
    std::shared_ptr<MMaterial> GetVoxelDebugMaterial() const { return m_pVoxelDebugMaterial; }
    const MBuffer* GetVoxelTableBuffer() const;
    const MBuffer* GetVoxelDebugBuffer() const;
    std::shared_ptr<MTexture> GetVoxelGITexture() const;
    MBoundsAABB GetVoxelizerBoundsAABB(uint32_t nClipmapIdx) const;

    void SetupVoxelSetting(const Vector3& f3CameraPosition, const uint32_t nClipmapIdx);

    MVoxelMapSetting GetVoxelSetting() const { return m_voxelSetting; }

protected:

    void InitializeBuffer();
    void ReleaseBuffer();

    void InitializeDispatcher();
    void ReleaseDispatcher();

    void InitializeVoxelTextureDispatcher();
    void ReleaseVoxelTextureDispatcher();

    void InitializeRenderPass();
    void ReleaseRenderPass();

    MVoxelMapSetting m_voxelSetting;
    std::shared_ptr<IShaderPropertyUpdateDecorator> m_pFramePropertyUpdateDecorator = nullptr;

    MComputeDispatcher* m_pVoxelDebugIndirectGenerator = nullptr;
    MComputeDispatcher* m_pVoxelTextureGenerator = nullptr;
    std::shared_ptr<MShaderConstantParam> m_pDebugVoxelMapSetting = nullptr;
    std::shared_ptr<MShaderConstantParam> m_pVoxelizerVoxelMapSetting = nullptr;
    std::unordered_map<MStringId, std::shared_ptr<MMaterial>> m_tVoxelizerMaterial = {};
    std::shared_ptr<MMaterial> m_pVoxelDebugMaterial = nullptr;
    MBuffer m_voxelizerBuffer;
    MBuffer m_drawIndirectBuffer;

    std::shared_ptr<MTexture> m_voxelGITexture = nullptr;
    std::shared_ptr<MTexture> m_pVoxelizerRenderTarget = nullptr;
    MRenderPass m_voxelizerRenderPass;
};
